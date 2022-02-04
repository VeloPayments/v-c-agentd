/**
 * \file attestationservice/attestationservice_get_or_query_artifact.c
 *
 * \brief Get a copy of the artifact record from a tree or query it from the
 * data service.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Query the data service for an artifact record by artifact id.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The child context to use for this operation.
 * \param artifact_id       The artifact id to query.
 * \param artifact          The artifact record to return.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_get_or_query_artifact(
    attestationservice_instance* inst,
    uint32_t child_context, RCPR_SYM(rcpr_uuid)* artifact_id,
    artifact_record_value** artifact)
{
    status retval;
    artifact_record_value* curr;
    data_artifact_record_t artifact_rec;
    uint32_t status, offset;

    /* Try to find the artifact in the rbtree. */
    retval = rbtree_find((resource**)&curr, inst->artifact_tree, artifact_id);
    if (ERROR_RBTREE_NOT_FOUND == retval)
    {
        /* send an artifact query request to the data service. */
        TRY_OR_FAIL(
            dataservice_api_sendreq_artifact_get(
                inst->data_sock, &inst->vpr_alloc, child_context,
                artifact_id->data),
            done);

        /* get the response for this request. */
        TRY_OR_FAIL(
            dataservice_api_recvresp_artifact_get(
                inst->data_sock, inst->alloc, &offset, &status,
                &artifact_rec),
            done);

        /* verify that this request succeeded. */
        TRY_OR_FAIL(status, done);

        /* create an artifact record value from this value. */
        TRY_OR_FAIL(
            attestationservice_artifact_record_value_create_from_artifact(
                artifact, inst, &artifact_rec),
            done);

        /* success. */
        retval = STATUS_SUCCESS;
        goto done;
    }

    /* otherwise, clone a value from curr. */
    TRY_OR_FAIL(
        attestationservice_artifact_record_value_create_from_artifact(
            artifact, inst, &curr->data),
        done);

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
