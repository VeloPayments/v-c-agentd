/**
 * \file
 * notificationservice/notificationservice_protocol_outbound_endpoint_fiber_entry.c
 *
 * \brief Entry point for a notificationservice protocol outbound endpoint
 * fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for a notificationservice protocol outbound endpoint
 * fiber.
 *
 * This fiber manages a notificationservice protocol outbound endpoint instance.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_fiber_entry(void* vctx)
{
    status retval = STATUS_SUCCESS, release_retval;
    notificationservice_protocol_outbound_endpoint_fiber_context* ctx =
        (notificationservice_protocol_outbound_endpoint_fiber_context*)vctx;
    message* msg;
    notificationservice_protocol_outbound_endpoint_message_payload* payload;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_notificationservice_protocol_outbound_endpoint_fiber_context_valid(
            ctx));

    /* loop while we are not terminating. */
    while (!ctx->inst->ctx->terminate)
    {
        /* read a message from our mailbox. */
        retval =
            message_receive(
                ctx->inst->outbound_addr, &msg, ctx->inst->ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* get the payload for this message. */
        payload =
            (notificationservice_protocol_outbound_endpoint_message_payload*)
            message_payload(msg, false);

        /* write the payload data to the socket. */
        retval =
            psock_write_boxed_data(
                ctx->inst->protosock, payload->payload_data,
                payload->payload_data_size);
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
