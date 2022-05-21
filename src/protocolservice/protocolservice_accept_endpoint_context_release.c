/**
 * \file protocolservice/protocolservice_accept_endpoint_context_release.c
 *
 * \brief Release the accept endpoint fiber context resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <string.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release the protocol service accept endpoint fiber context.
 *
 * \param r             The protocol service accept endpoint fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_endpoint_context_release(RCPR_SYM(resource)* r)
{
    status acceptsock_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    protocolservice_accept_endpoint_context* ctx =
        (protocolservice_accept_endpoint_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_accept_endpoint_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the accept sock. */
    if (NULL != ctx->acceptsock)
    {
        acceptsock_release_retval =
            resource_release(psock_resource_handle(ctx->acceptsock));
    }

    /* reclaim memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the appropriate response code. */
    if (STATUS_SUCCESS != acceptsock_release_retval)
    {
        return acceptsock_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}
