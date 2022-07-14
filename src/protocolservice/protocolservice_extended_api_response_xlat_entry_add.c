/**
 * \file
 * protocolservice/protocolservice_extended_api_response_xlat_entry_add.c
 *
 * \brief Add an entry to the extended API response translation table.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add an extended API response xlat entry to the given sentinel context.
 *
 * \param ctx           The context to which this entry is added.
 * \param server_offset The server offset.
 * \param client_offset The client offset.
 * \param return_addr   The client return address.
 *
 * \returns an error code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_response_xlat_entry_add(
    protocolservice_protocol_fiber_context* ctx, uint64_t server_offset,
    uint32_t client_offset, RCPR_SYM(mailbox_address) return_addr)
{
    status retval, release_retval;
    protocolservice_extended_api_response_xlat_entry* tmp;

    /* allocate memory for this entry. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize the resource. */
    resource_init(
        &tmp->hdr, &protocolservice_extended_api_response_xlat_entry_release);

    /* save values. */
    tmp->alloc = ctx->alloc;
    tmp->server_offset = server_offset;
    tmp->client_offset = client_offset;
    tmp->client_return_address = return_addr;

    /* insert this record into the translation table. */
    retval = rbtree_insert(ctx->extended_api_offset_dict, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
