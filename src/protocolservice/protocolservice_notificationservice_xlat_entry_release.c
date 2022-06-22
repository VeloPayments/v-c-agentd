/**
 * \file
 * protocolservice/protocolservice_notificationservice_xlat_entry_release.c
 *
 * \brief Reduce the reference count and possibly release a translation table
 * entry resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Reduce the reference count and possibly release a translation
 * table entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_xlat_entry_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_notificationservice_xlat_entry* entry =
        (protocolservice_notificationservice_xlat_entry*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_xlat_entry_valid(entry));

    /* reduce the reference count. */
    entry->reference_count -= 1;

    /* if this entry is still being referenced, don't release it yet. */
    if (entry->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache the allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* clear the entry. */
    memset(entry, 0, sizeof(*entry));

    /* reclaim the memory. */
    return rcpr_allocator_reclaim(alloc, entry);
}
