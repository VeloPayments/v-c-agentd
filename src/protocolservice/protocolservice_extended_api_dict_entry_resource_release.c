/**
 * \file
 * protocolservice/protocolservice_extended_api_dict_entry_resource_release.c
 *
 * \brief Release an extended api dict entry.
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
 * \brief Release an extended API dictionary entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_dict_entry_resource_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_extended_api_dict_entry* entry =
        (protocolservice_extended_api_dict_entry*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* clear the memory. */
    memset(entry, 0, sizeof(*entry));

    /* reclaim the memory. */
    return rcpr_allocator_reclaim(alloc, entry);
}
