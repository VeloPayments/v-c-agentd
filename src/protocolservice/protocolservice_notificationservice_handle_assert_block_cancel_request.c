/**
 * \file
 * protocolservice/protocolservice_handle_assert_block_cancel_request.c
 *
 * \brief Handle sending and receiving a block cancel request to the
 * notificationservice endpoint.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

static const rcpr_uuid zero_uuid = { .data = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

/**
 * \brief Handle an assert block cancellation request from the protocol.
 *
 * This method creates an assert block cancel request for the notification
 * service endpoint, sends it, and receives a response with the notification
 * service offset.
 *
 * \param ctx           The protocolservice protocol context for this request.
 * \param req_offset    The request offset from the client request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_handle_assert_block_cancel_request(
    protocolservice_protocol_fiber_context* ctx, uint32_t req_offset)
{
    status retval, release_retval;
    message* req_message = NULL;
    message* resp_message = NULL;
    protocolservice_notificationservice_block_assertion_request*
        req_payload = NULL;

    /* create the request message. */
    retval =
        protocolservice_notificationservice_block_assertion_request_create(
            &req_payload, ctx->alloc, req_offset, &zero_uuid, ctx->return_addr);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the cancel flag. */
    req_payload->cancel = true;

    /* create the message to send to the notificationservice endpoint. */
    retval =
        message_create(
            &req_message, ctx->alloc, ctx->fiber_addr, &req_payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_payload;
    }

    /* the request payload is now owned by the request message. */
    req_payload = NULL;

    /* send the message to the notificationservice endpoint. */
    retval =
        message_send(
            ctx->ctx->notificationservice_endpoint_addr, req_message,
            ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_message;
    }

    /* the request message is now owned by the message discipline. */
    req_message = NULL;

    /* read the response from the notificationservice endpoint. */
    retval = message_receive(ctx->fiber_addr, &resp_message, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_message;
    }

    /* clean up the response message. */
    retval = resource_release(message_resource_handle(resp_message));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_message;
    }

cleanup_req_message:
    if (NULL != req_message)
    {
        release_retval = resource_release(message_resource_handle(req_message));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_req_payload:
    if (NULL != req_payload)
    {
        release_retval = resource_release(&req_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

done:
    return retval;
}
