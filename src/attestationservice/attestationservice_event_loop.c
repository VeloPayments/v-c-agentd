/**
 * \file attestationservice/attestationservice_event_loop.c
 *
 * \brief Perform attestation of transactions in the pending transaction queue.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/attestationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/* forward decls. */
#if ATTESTATION == 1
static status attestationservice_do_attestation(
    attestationservice_instance* inst, uint32_t child_context);

/* key denoting the end of the transaction chain. */
const uint8_t END_OF_TRANSACTION_KEY[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#endif

/**
 * \brief The event loop for the attestation service. This event loop sleeps
 * until activation time, then queries the process queue for transactions that
 * have not yet been attested, and performs attestation on these.
 *
 * \param inst          The attestation service instance to use for this loop.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_event_loop(attestationservice_instance* inst)
{
    status retval, release_retval;
    uint32_t child_context;

    /* set up child node for data service. */
    TRY_OR_FAIL(
        attestationservice_dataservice_child_context_create(
            inst, &child_context),
        cleanup_inst);

    for (;;)
    {
        /* TODO - replace with config value. */
        uint64_t sleep_micros = 5000 * 1000;

        /* sleep. */
        TRY_OR_FAIL(
            attestationservice_sleep(inst->sleep_sock, sleep_micros),
            cleanup_inst);

        #if ATTESTATION == 1
        /* start a round of attestation if there are pending transactions. */
        TRY_OR_FAIL(
            attestationservice_do_attestation(inst, child_context),
            cleanup_inst);

        /* reset the rbtree instances for transactions and artifacts. */
        TRY_OR_FAIL(rbtree_clear(inst->transaction_tree), cleanup_inst);
        TRY_OR_FAIL(rbtree_clear(inst->artifact_tree), cleanup_inst);
        #endif
    }

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

    return retval;
}

#if ATTESTATION == 1
/**
 * \brief Perform the attestation process.
 *
 * \param inst              The attestation service instance to use.
 * \param child_context     The child context for the dataservice.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status attestationservice_do_attestation(
    attestationservice_instance* inst, uint32_t child_context)
{
    status retval, release_retval;
    data_transaction_node_t txn_node;
    void* txn_data;
    size_t txn_data_size;
    uint32_t status;
    uint32_t drop_status, drop_offset;

    /* Query the pending transaction table for new entries. */
    retval = status =
        attestationservice_dataservice_query_pending_transaction(
            inst->data_sock, &inst->vpr_alloc, inst->alloc, child_context, NULL,
            &txn_node, &txn_data, &txn_data_size);
    if (AGENTD_ERROR_DATASERVICE_NOT_FOUND == retval)
    {
        /* if no results were found, go back to sleep. */
        goto exit_nonfatal;
    }
    else if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* if we got any other error, exit the process. */
        goto exit_fatal;
    }

    /* if an entry is found, but it has been attested, go back to sleep. */
    uint32_t txn_state = ntohl(txn_node.net_txn_state);
    if (DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED == txn_state)
    {
        /* TODO - this assumes a malloc allocator. Fix in recvresp. */
        CLEANUP_OR_FALLTHROUGH(
            rcpr_allocator_reclaim(inst->alloc, txn_data));
        goto exit_nonfatal;
    }

    /* Otherwise, start the attestation loop. */
    while (AGENTD_STATUS_SUCCESS == status
        && DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED == txn_state)
    {
        /* TODO: Has this entry been signed by an authorized entity? */

        /* If this is a create transaction, is the artifact ID unique and
         * all fields valid for a create?
         */
        TRY_OR_FAIL(
            attestationservice_verify_txn_has_valid_fields(
                inst, &txn_node, txn_data, txn_data_size),
            drop_txn);

        /* If this is any other transaction, does the previous transaction
         * match the latest transaction from that artifact (either from the
         * pending transactions or queried from the database?) and does the
         * previous transaction state match the last transaction's state?
         */
        TRY_OR_FAIL(
            attestationservice_verify_txn_is_in_correct_sequence(
                inst, child_context, &txn_node, txn_data, txn_data_size),
            drop_txn);

        /* Is the transaction unique? */
        /* NOTE: unique means that its transaction id (and artifact id if a
         * create) does not exist as an artifact, entity, block, or transaction
         * id anywhere else.
         */
        TRY_OR_FAIL(
            attestationservice_verify_txn_is_unique(
                inst, child_context, &txn_node, txn_data, txn_data_size),
            drop_txn);

        /* if the transaction passes all attestation tests, promote it. */
        TRY_OR_FAIL(
            attestationservice_dataservice_transaction_promote(
                inst, child_context, &txn_node),
            exit_fatal);

        /* move on to the next transaction. */
        goto txn_cleanup;

    drop_txn:
        /* drop a failed transaction. */
        TRY_OR_FAIL(
            dataservice_api_sendreq_transaction_drop(
                inst->data_sock, child_context, txn_node.key),
            exit_fatal);
        TRY_OR_FAIL(
            dataservice_api_recvresp_transaction_drop(
                inst->data_sock, inst->alloc, &drop_offset, &drop_status),
            exit_fatal);
        /* ignore the drop status; it's possible that the canonization
         * service is clobbering us. */

    txn_cleanup:
        /* TODO - this assumes a malloc allocator. Fix in recvresp. */
        CLEANUP_OR_FALLTHROUGH(
            rcpr_allocator_reclaim(inst->alloc, txn_data));

        /* get the next transaction by sequence. */
        if (memcmp(txn_node.next, END_OF_TRANSACTION_KEY, 16))
        {
            /* query the next pending transaction. */
            TRY_OR_FAIL(
                attestationservice_dataservice_query_pending_transaction(
                    inst->data_sock, &inst->vpr_alloc, inst->alloc,
                    child_context, (rcpr_uuid*)txn_node.next,
                    &txn_node, &txn_data, &txn_data_size),
                exit_fatal);
        }
        else
        {
            status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        }
    }

exit_fatal:
    return retval;

exit_nonfatal:
    return STATUS_SUCCESS;
}
#endif
