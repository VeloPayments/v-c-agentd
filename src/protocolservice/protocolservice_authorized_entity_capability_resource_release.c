/**
 * \file protocolservice/protocolservice_dataservice_request_message_release.c
 *
 * \brief Release a dataservice request message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a protocolserice authorized entity capability resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_capability_resource_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_authorized_entity_capability* cap =
        (protocolservice_authorized_entity_capability*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = cap->alloc;

    /* clear the memory. */
    memset(cap, 0, sizeof(*cap));

    /* reclaim the memory. */
    return
        rcpr_allocator_reclaim(alloc, cap);
}
