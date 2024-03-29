/**
 * \file protocolservice/protocolservice_accept_endpoint_fiber_entry.c
 *
 * \brief Entry point for the protocol service accept endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service accept endpoint fiber.
 *
 * This fiber accepts connections from the listen service and assigns protocol
 * fiber instances to manage them.
 *
 * \param vctx          The type erased accept endopint context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_endpoint_fiber_entry(void* vctx)
{
    status retval = STATUS_SUCCESS, release_retval;
    protocolservice_accept_endpoint_context* ctx =
        (protocolservice_accept_endpoint_context*)vctx;
    int desc;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_accept_endpoint_context_valid(ctx));

    /* loop while we are not quiescing... */
    while (!ctx->ctx->quiesce)
    {
        /* accept a socket from the listen service. */
        retval = psock_read_raw_descriptor(ctx->acceptsock, &desc);
        if (STATUS_SUCCESS != retval)
        {
            /* TODO - if this fails, we should shut down this process so the
             * supervisor restarts everyone. */
            goto cleanup_context;
        }

        /* TODO - limit connections by configurable value. */

        /* add a protocol fiber to the scheduler to handle this descriptor. */
        retval = protocolservice_protocol_fiber_add(ctx->alloc, ctx->ctx, desc);
        if (STATUS_SUCCESS != retval)
        {
            /* if this fails, close the descriptor. Don't exit the loop. */
            close(desc);
        }

        /* if the above call succeeds, then the descriptor is owned by the
         * protocol fiber. */
        desc = -1;
        retval = STATUS_SUCCESS;
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
