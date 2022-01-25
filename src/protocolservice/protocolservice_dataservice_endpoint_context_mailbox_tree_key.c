/**
 * \file
 * protocolservice/protocolservice_dataservice_endpoint_context_mailbox_tree_key.c
 *
 * \brief Get the key for a context mailbox entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Given an mailbox_context resource handle, return its context value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_dataservice_endpoint_context_mailbox_tree_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    const protocolservice_dataservice_mailbox_context_entry* entry =
        (const protocolservice_dataservice_mailbox_context_entry*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_dataservice_mailbox_context_entry_valid(entry));

    /* return the mailbox address. */
    return &entry->context;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
