/**
 * \file
 * protocolservice/protocolservice_notificationservice_lookup_return_address_from_offset.c
 *
 * \brief Look up the return address from the notificationservice offset.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Look up a return address from a notificationservice offset, and remove
 * the entries from the lookup tables.
 *
 * \param return_addr       Pointer to receive the return address on success.
 * \param req_offset        Pointer to receive the client-side request offset.
 * \param ctx               The endpoint fiber context.
 * \param offset            The notificationservice offset.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_lookup_return_address_from_offset(
    RCPR_SYM(mailbox_address)* return_address, uint32_t* req_offset,
    protocolservice_notificationservice_fiber_context* ctx, uint64_t offset)
{
    status retval;
    protocolservice_notificationservice_xlat_entry* entry;

    /* look up the entry. */
    retval = rbtree_find((resource**)&entry, ctx->server_xlat_map, &offset);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the return address. */
    *return_address = entry->client_addr;
    *req_offset = entry->client_offset;

    /* delete the entry from the server transation tree. */
    retval = rbtree_delete(NULL, ctx->server_xlat_map, &entry->server_offset);
    (void)retval;

    /* delete the entry from the client translation tree. */
    /* NOTE that after this, entry will be dangling. */
    retval = rbtree_delete(NULL, ctx->client_xlat_map, &entry->client_addr);
    (void)retval;

    /* entry is invalid, so set it to NULL. */
    entry = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
