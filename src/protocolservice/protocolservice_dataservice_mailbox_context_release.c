/**
 * \file protocolservice/protocolservice_dataservice_mailbox_context_release.c
 *
 * \brief Release a mailbox_context entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release a mailbox context resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_mailbox_context_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_dataservice_mailbox_context_entry* entry =
        (protocolservice_dataservice_mailbox_context_entry*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_dataservice_mailbox_context_entry_valid(entry));

    /* decrement the reference count. */
    entry->reference_count -= 1;

    /* if the reference count is still positive, exit. */
    if (entry->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache allocator. */
    rcpr_allocator* alloc = entry->alloc;

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, entry);
}
