/**
 * \file attestationservice/attestationservice_transaction_tree_insert.c
 *
 * \brief Insert a transaction into the transaction / artifact trees.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

static uint8_t zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

/**
 * \brief Add a transaction to the red-black tree.
 *
 * If this is a create transaction, add the artifact to the rbtree. Otherwise,
 * update the artifact record in the artifact rbtree.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The dataservice child context.
 * \param txn_node          The transaction node to add.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_transaction_tree_insert(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node)
{
    status retval, release_retval;
    transaction_record_value* txn;
    artifact_record_value* artifact;

    /* create a record value. */
    TRY_OR_FAIL(
        attestationservice_transaction_record_value_create(
            &txn, inst, txn_node),
        done);

    /* insert this record into the transaction tree. */
    TRY_OR_FAIL(
        rbtree_insert(inst->transaction_tree, &txn->hdr),
        cleanup_txn);

    /* if there is no previous transaction ID, then insert the artifact. */
    if (!memcmp(txn_node->prev, zero_uuid, 16))
    {
        /* create an artifact record from the transaction record. */
        TRY_OR_FAIL(
            attestationservice_artifact_record_value_create(
                &artifact, inst, txn_node),
            done);

        /* update or insert this record into the artifact tree. */
        TRY_OR_FAIL(
            attestationservice_update_or_insert_artifact(
                inst, artifact),
            cleanup_artifact);
    }
    else
    {
        /* get the artifact or query it from the dataservice. */
        TRY_OR_FAIL(
            attestationservice_get_or_query_artifact(
                inst, child_context, (rcpr_uuid*)txn_node->artifact_id,
                &artifact),
            done);

        /* set the latest values. */
        memcpy(artifact->data.txn_latest, txn_node->key, 16);
        artifact->data.net_state_latest = txn_node->net_txn_state;

        /* update or insert this artifact into the artifact tree. */
        TRY_OR_FAIL(
            attestationservice_update_or_insert_artifact(
                inst, artifact),
            cleanup_artifact);
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_artifact:
    CLEANUP_OR_FALLTHROUGH(resource_release(&artifact->hdr));
    goto done;

cleanup_txn:
    CLEANUP_OR_FALLTHROUGH(resource_release(&txn->hdr));

done:
    return retval;
}
