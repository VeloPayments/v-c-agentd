/**
 * \file notificationservice/notificationservice_assertion_entry_release.c
 *
 * \brief Release an assertion entry resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a notificationservice assertion entry.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_assertion_entry_release(RCPR_SYM(resource)* r)
{
    notificationservice_assertion_entry* entry =
        (notificationservice_assertion_entry*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* clear memory. */
    memset(entry, 0, sizeof(*entry));

    /* reclaim memory. */
    return rcpr_allocator_reclaim(alloc, entry);
}
