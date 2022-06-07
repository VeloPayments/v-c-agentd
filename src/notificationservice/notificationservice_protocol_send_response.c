/**
 * \file notificationservice/notificationservice_protocol_send_response.c
 *
 * \brief Send a response message to the outbound endpoint.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Send a response payload to the outbound endpoint.
 *
 * \param ctx           The context for this operation.
 * \param method_id     The method id for the request.
 * \param offset        The offset for the response.
 * \param status_code   The status for the response.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_send_response(
    notificationservice_protocol_fiber_context* ctx, uint32_t method_id,
    uint64_t offset, uint32_t status_code)
{
    status retval, release_retval;
    uint8_t* buf;
    size_t buf_size;
    notificationservice_protocol_outbound_endpoint_message_payload* payload;
    message* msg;

    /* encode a response message. */
    retval =
        notificationservice_api_encode_response(
            &buf, &buf_size, ctx->alloc, method_id, status_code, offset, NULL,
            0U);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* wrap this message in a payload. */
    retval =
        notificationservice_protocol_outbound_endpoint_message_payload_create(
            &payload, ctx->alloc, buf, buf_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_buf;
    }

    /* the buffer is now owned by the payload. */
    buf = NULL;

    /* wrap this payload in a message envelope. */
    retval =
        message_create(&msg, ctx->alloc, MESSAGE_ADDRESS_NONE, &payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* the payload is now owned by the message. */
    payload = NULL;

    /* send the message. */
    retval =
        message_send(ctx->inst->outbound_addr, msg, ctx->inst->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_message:
    release_retval = resource_release(message_resource_handle(msg));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_payload:
    if (NULL != payload)
    {
        release_retval = resource_release(&payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_buf:
    if (NULL != buf)
    {
        release_retval = rcpr_allocator_reclaim(ctx->alloc, buf);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

done:
    return retval;
}
