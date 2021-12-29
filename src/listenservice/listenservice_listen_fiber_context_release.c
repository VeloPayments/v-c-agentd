/**
 * \file listenservice/listenservice_listen_fiber_context_release.c
 *
 * \brief Release a listen fiber context resource.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include "listenservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release a listen fiber context.
 *
 * \param r             The listen fiber context resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_context_release(RCPR_SYM(resource)* r)
{
    status listen_socket_release_retval = STATUS_SUCCESS;
    status return_addr_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    listenservice_listen_fiber_context* ctx =
        (listenservice_listen_fiber_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_listenservice_listen_fiber_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* attempt to release the listen socket. */
    if (NULL != ctx->listen_socket)
    {
        listen_socket_release_retval =
            resource_release(psock_resource_handle(ctx->listen_socket));
    }

    /* attempt to close the fiber mailbox. */
    if (ctx->return_addr != (uint64_t)-1)
    {
        return_addr_release_retval =
            mailbox_close(ctx->return_addr, ctx->msgdisc);
    }

    /* release the context memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* send the first failing status or success. */
    if (STATUS_SUCCESS != listen_socket_release_retval)
    {
        return listen_socket_release_retval;
    }
    else if (STATUS_SUCCESS != return_addr_release_retval)
    {
        return return_addr_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}
