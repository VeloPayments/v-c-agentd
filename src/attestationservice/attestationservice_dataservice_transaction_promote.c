/**
 * \file attestationservice/attestationservice_dataservice_transaction_promote.c
 *
 * \brief Promote a transaction to promoted in the transaction process queue.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Promote a transaction to attested.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The child context to use for this operation.
 * \param txn_node          The transaction node to promote.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_dataservice_transaction_promote(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node)
{
    status retval;
    uint32_t status, offset;

    /* send the promotion request to the dataservice. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_transaction_promote(
            inst->data_sock, child_context, txn_node->key),
        done);

    /* receive the response from the promotion request. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_transaction_promote(
            inst->data_sock, inst->alloc, &offset,
            &status),
        done);

    /* if the operation failed, exit. */
    TRY_OR_FAIL(status, done);

    /* Add this transaction to the transaction tree. */
    TRY_OR_FAIL(
        attestationservice_transaction_tree_insert(
            inst, child_context, txn_node),
        done);

    /* success. */
    retval = STATUS_SUCCESS;

done:
    return retval;
}
