/**
 * \file
 * protocolservice/protocolservice_extended_api_response_xlat_entry_release.c
 *
 * \brief Release an extended API response translation table entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release an extended API xlat table entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_response_xlat_entry_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_extended_api_response_xlat_entry* entry =
        (protocolservice_extended_api_response_xlat_entry*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* clear memory. */
    memset(entry, 0, sizeof(*entry));

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, entry);
}
