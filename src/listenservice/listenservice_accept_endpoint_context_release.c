/**
 * \file listenservice/listenservice_accept_endpoint_context_release.c
 *
 * \brief Release an accept endpoint fiber context resource.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include "listenservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release the accept endpoint fiber context.
 *
 * \param r             The accept endpoint fiber context resource to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_context_release(RCPR_SYM(resource)* r)
{
    status accept_socket_release_retval = STATUS_SUCCESS;
    status endpoint_addr_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    listenservice_accept_endpoint_context* ctx =
        (listenservice_accept_endpoint_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_listenservice_accept_endpoint_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* attempt to release the accept socket. */
    if (NULL != ctx->accept_socket)
    {
        accept_socket_release_retval =
            resource_release(psock_resource_handle(ctx->accept_socket));
    }

    /* attempt to close the endpoint mailbox. */
    if (ctx->endpoint_addr != (uint64_t)-1)
    {
        endpoint_addr_release_retval =
            mailbox_close(ctx->endpoint_addr, ctx->msgdisc);
    }

    /* release the context memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* send the first failing status or success. */
    if (STATUS_SUCCESS != accept_socket_release_retval)
    {
        return accept_socket_release_retval;
    }
    else if (STATUS_SUCCESS != endpoint_addr_release_retval)
    {
        return endpoint_addr_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}
