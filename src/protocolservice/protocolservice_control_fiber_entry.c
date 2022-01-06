/**
 * \file protocolservice/protocolservice_control_fiber_entry.c
 *
 * \brief Entry point for the control protocol fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service control fiber.
 *
 * This fiber manages the control protocol for the protocol service.
 *
 * \param vctx          The type erased control fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_entry(void* vctx)
{
    status retval, release_retval;
    void* req;
    size_t size;
    protocolservice_control_fiber_context* ctx =
        (protocolservice_control_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));

    while (!ctx->should_exit)
    {
        /* read a control packet from the supervisor. */
        retval =
            psock_read_boxed_data(ctx->controlsock, ctx->alloc, &req, &size);
        if (STATUS_SUCCESS != retval)
        {
            /* if reading a control packet fails, force an exit. */
            goto force_exit;
        }

        /* decode and dispatch the control packet. */
        retval =
            protocolservice_control_decode_and_dispatch(ctx, req, size);

        /* clean up the request data. */
        memset(req, 0, size);
        release_retval = rcpr_allocator_reclaim(ctx->alloc, req);
        if (STATUS_SUCCESS != release_retval)
        {
            goto force_exit;
        }

        /* if the decode and dispatch failed, force an exit. */
        if (STATUS_SUCCESS != retval)
        {
            goto force_exit;
        }
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_context;

force_exit:
    release_retval = protocolservice_force_exit(ctx->ctx);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
