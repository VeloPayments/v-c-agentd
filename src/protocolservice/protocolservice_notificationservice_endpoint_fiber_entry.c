/**
 * \file
 * protocolservice/protocolservice_notificationservice_endpoint_fiber_entry.c
 *
 * \brief Entry point for the notificationservice endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service notificationservice endpoint
 * fiber.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_fiber_entry(void* vctx)
{
    status retval, release_retval;
    message* req_msg;
    protocolservice_notificationservice_fiber_context* ctx =
        (protocolservice_notificationservice_fiber_context*)vctx;
    protocolservice_notificationservice_block_assertion_request* req_payload;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));

    /* event loop for notificationservice endpoint. */
    for (;;)
    {
        /* read a message from the message queue. */
        retval = message_receive(ctx->notify_addr, &req_msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_ctx;
        }

        /* get the message payload. */
        req_payload =
            (protocolservice_notificationservice_block_assertion_request*)
            message_payload(req_msg, false);

        /* TODO - discriminate between assertion and assertion cancel. */

        /* compute a new offset. */
        ctx->request_offset_counter += 1;
        uint64_t msg_offset = ctx->request_offset_counter;

        /* add the request entry to the translation map. */
        retval =
            protocolservice_notificationservice_xlat_map_add(
                ctx, msg_offset, req_payload->reply_addr);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_req_msg;
        }

        /* send the request to the notification service API. */
        retval =
            notificationservice_api_sendreq_block_assertion(
                ctx->notifysock, ctx->alloc, msg_offset,
                &req_payload->block_id);
        if (STATUS_SUCCESS != retval)
        {
            goto reply_error;
        }

        /* send the response to the reply-to mailbox. */
        retval =
            protocolservice_notificationservice_endpoint_send_request_response(
                ctx, message_return_address(req_msg), msg_offset, true);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_req_msg;
        }

        /* clean up the request message. */
        retval = resource_release(message_resource_handle(req_msg));
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_ctx;
        }
    }

reply_error:
    /* send the response to the reply-to mailbox. */
    release_retval =
        protocolservice_notificationservice_endpoint_send_request_response(
            ctx, message_return_address(req_msg), 0U, false);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_req_msg:
    release_retval = resource_release(message_resource_handle(req_msg));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
