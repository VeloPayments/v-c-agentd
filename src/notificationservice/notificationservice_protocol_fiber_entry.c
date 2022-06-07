/**
 * \file notificationservice/notificationservice_protocol_fiber_entry.c
 *
 * \brief Entry point for a notificationservice protocol fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <signal.h>
#include <unistd.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for a notificationservice protocol fiber.
 *
 * This fiber manages a notificationservice protocol instance.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_entry(void* vctx)
{
    status retval = STATUS_SUCCESS, release_retval;
    notificationservice_protocol_fiber_context* ctx =
        (notificationservice_protocol_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_notificationservice_protocol_fiber_context_valid(ctx));

    /* decode-and-dispatch loop. */
    while (!ctx->inst->ctx->quiesce && !ctx->inst->ctx->terminate)
    {
        retval =
            notificationservice_protocol_read_decode_and_dispatch_packet(ctx);
        if (STATUS_SUCCESS != retval)
        {
            goto signal_shutdown;
        }
    }

signal_shutdown:
    /* notify the signal thread that we are terminating. */
    kill(getpid(), SIGTERM);

    /* clean up our context. */
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
