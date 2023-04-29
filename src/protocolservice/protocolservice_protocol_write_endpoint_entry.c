/**
 * \file protocolservice/protocolservice_protocol_write_endpoint_entry.c
 *
 * \brief Entry point for a protocol service write endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for a protocol service protocol write endpoint fiber.
 *
 * This fiber writes messages from the messaging discipline to the client.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_entry(void* vctx)
{
    status retval, release_retval;
    protocolservice_protocol_fiber_context* ctx =
        (protocolservice_protocol_fiber_context*)vctx;
    message* msg;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* loop while we are not quiescing and we shouldn't shut down. */
    while (!ctx->ctx->quiesce && !ctx->shutdown)
    {
        /* read a message from the return mailbox. */
        retval = message_receive(ctx->return_addr, &msg, ctx->ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* decode and dispatch this message. */
        retval =
            protocolservice_protocol_write_endpoint_decode_and_dispatch(
                ctx, msg);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_message;
        }

        /* release the message. */
        retval = resource_release(message_resource_handle(msg));
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }
    }

    /* we are shutting down. */
    retval = STATUS_SUCCESS;
    goto cleanup_context;

cleanup_message:
    release_retval = resource_release(message_resource_handle(msg));
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
