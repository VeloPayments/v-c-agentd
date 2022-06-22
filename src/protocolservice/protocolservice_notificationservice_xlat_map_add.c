/**
 * \file
 * protocolservice/protocolservice_notificationservice_xlat_map_add.c
 *
 * \brief Add an entry to the translation maps.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add a request to the notificationservice translation maps.
 *
 * \param ctx           The endpoint context.
 * \param msg_offset    The server-side offset.
 * \param client_addr   The client_side mailbox address.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_xlat_map_add(
    protocolservice_notificationservice_fiber_context* ctx,
    uint64_t msg_offset, RCPR_SYM(mailbox_address) client_addr)
{
    status retval, release_retval;
    protocolservice_notificationservice_xlat_entry* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));
    MODEL_ASSERT(msg_offset > 0);
    MODEL_ASSERT(client_addr > 0);

    /* allocate memory for an entry. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr, &protocolservice_notificationservice_xlat_entry_release);

    /* set entry values. */
    tmp->alloc = ctx->alloc;
    tmp->reference_count = 1;
    tmp->client_addr = client_addr;
    tmp->server_offset = msg_offset;

    /* insert entry into the client xlat map. */
    retval = rbtree_insert(ctx->client_xlat_map, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entry;
    }

    /* increase the reference count to show shared ownership with the map. */
    tmp->reference_count += 1;

    /* insert entry into the server xlat map. */
    retval = rbtree_insert(ctx->server_xlat_map, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entry;
    }

    /* the entry is now owned by both maps. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_entry:
    /* we either need to reduce the reference count or reclaim the entry. */
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
