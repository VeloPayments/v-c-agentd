/**
 * \file
 * attestationservice/attestationservice_artifact_record_value_resource_release.c
 *
 * \brief Release an artifact record value.
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
 * \brief Release an artifact_record_value resource.
 *
 * \param r         The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_artifact_record_value_resource_release(resource* r)
{
    artifact_record_value* artifact = (artifact_record_value*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = artifact->alloc;

    /* clear the structure. */
    memset(artifact, 0, sizeof(artifact_record_value));

    return rcpr_allocator_reclaim(alloc, artifact);
}
