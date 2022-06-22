/**
 * \file
 * protocolservice/protocolservice_notificationservice_fiber_context_release.c
 *
 * \brief Release a notificationservice fiber context resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Release the protocol service notification service fiber context.
 *
 * \param r             The context to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_fiber_context_release(
    RCPR_SYM(resource)* r)
{
    status notify_addr_close_retval = STATUS_SUCCESS;
    status notifysock_release_retval = STATUS_SUCCESS;
    status client_xlat_map_release_retval = STATUS_SUCCESS;
    status server_xlat_map_release_retval = STATUS_SUCCESS;
    status reclaim_retval = STATUS_SUCCESS;
    protocolservice_notificationservice_fiber_context* ctx =
        (protocolservice_notificationservice_fiber_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));

    /* decrement the reference count. */
    ctx->reference_count -= 1;

    /* if the reference count is greater than zero, then don't release. */
    if (ctx->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* close the mailbox. */
    if (ctx->notify_addr > 0)
    {
        notify_addr_close_retval =
            mailbox_close(ctx->notify_addr, ctx->msgdisc);
    }

    /* release the notify socket. */
    if (NULL != ctx->notifysock)
    {
        notifysock_release_retval =
            resource_release(psock_resource_handle(ctx->notifysock));
    }

    /* release the client-side translation map. */
    if (NULL != ctx->client_xlat_map)
    {
        client_xlat_map_release_retval =
            resource_release(rbtree_resource_handle(ctx->client_xlat_map));
    }

    /* release the server-side translation map. */
    if (NULL != ctx->server_xlat_map)
    {
        server_xlat_map_release_retval =
            resource_release(rbtree_resource_handle(ctx->server_xlat_map));
    }

    /* clear the struct. */
    memset(ctx, 0, sizeof(*ctx));

    /* release the struct. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the return value. */
    if (STATUS_SUCCESS != notify_addr_close_retval)
    {
        return notify_addr_close_retval;
    }
    else if (STATUS_SUCCESS != notifysock_release_retval)
    {
        return notifysock_release_retval;
    }
    else if (STATUS_SUCCESS != client_xlat_map_release_retval)
    {
        return client_xlat_map_release_retval;
    }
    else if (STATUS_SUCCESS != server_xlat_map_release_retval)
    {
        return server_xlat_map_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
