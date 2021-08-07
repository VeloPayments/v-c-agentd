/**
 * \file attestationservice/attestationservice_update_or_insert_artifact.c
 *
 * \brief Update or insert an artifact record.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Update the artifact record or insert a new one.
 *
 * \param inst              The attestation service instance.
 * \param artifact          The artifact record to use for update or insert.
 *
 * \note On success, this function takes ownership of this record value and will
 * release it if it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_update_or_insert_artifact(
    attestationservice_instance* inst,
    artifact_record_value* artifact)
{
    artifact_record_value* curr;
    status retval;

    /* Try to find the artifact in the rbtree. */
    retval =
        rbtree_find((resource**)&curr, inst->artifact_tree, artifact->data.key);
    if (ERROR_RBTREE_NOT_FOUND == retval)
    {
        /* insert the record. */
        TRY_OR_FAIL(rbtree_insert(inst->artifact_tree, &artifact->hdr), done);

        /* we're done. */
        retval = STATUS_SUCCESS;
        goto done;
    }
    else if (STATUS_SUCCESS != retval)
    {
        /* we've encountered an error.  Exit. */
        goto done;
    }

    /* otherwise, update the record with our artifact values. */
    memcpy(&curr->data.txn_latest, &artifact->data.txn_latest, 16);
    curr->data.net_state_latest = artifact->data.net_state_latest;

    /* release the artifact, since we've updated the tree artifact. */
    TRY_OR_FAIL(resource_release(&artifact->hdr), done);

    /* success. */
    retval = STATUS_SUCCESS;

done:
    return retval;
}
