/**
 * \file attestationservice/attestationservice_artifact_record_value_create.c
 *
 * \brief Create an artifact record value.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create an artifact record to insert into the artifact tree.
 *
 * \param artifact          Pointer to receive the pointer to the created
 *                          artifact record instance.
 * \param inst              The attestation service instance.
 * \param txn_node          The transaction node to create this record from.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_artifact_record_value_create(
    artifact_record_value** artifact, attestationservice_instance* inst,
    const data_transaction_node_t* txn_node)
{
    status retval;
    artifact_record_value* tmp;

    /* allocate memory for this record. */
    TRY_OR_FAIL(
        rcpr_allocator_allocate(
            inst->alloc, (void**)&tmp, sizeof(artifact_record_value)),
        done);

    /* clear the structure. */
    memset(tmp, 0, sizeof(artifact_record_value));

    /* initialize the resource. */
    resource_init(
        &tmp->hdr, &attestationservice_artifact_record_value_resource_release);

    /* copy init values. */
    tmp->alloc = inst->alloc;
    memcpy(tmp->data.key, txn_node->artifact_id, 16);
    memcpy(tmp->data.txn_first, txn_node->key, 16);
    memcpy(tmp->data.txn_latest, txn_node->key, 16);
    tmp->data.net_state_latest = txn_node->net_txn_state;

    /* success. */
    retval = STATUS_SUCCESS;
    *artifact = tmp;

done:
    return retval;
}
