/**
 * \file protocolservice/protocolservice_send_error_response_message.c
 *
 * \brief Send an error response message to the protocol write endpoint.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Send an error response to the protocol write endpoint.
 *
 * \param ctx           The protocol fiber context for this socket.
 * \param request_id    The id of the request that caused the error.
 * \param status_       The status code of the error.
 * \param offset        The request offset that caused the error.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_send_error_response_message(
    protocolservice_protocol_fiber_context* ctx, int request_id, int status_,
    uint32_t offset)
{
    status retval, release_retval;
    uint32_t response[3] = { htonl(request_id), htonl(status_), htonl(offset) };
    protocolservice_protocol_write_endpoint_message* payload = NULL;
    message* msg = NULL;

    /* allocate memory for the message payload. */
    retval =
        rcpr_allocator_allocate(ctx->alloc, (void**)&payload, sizeof(*payload));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear payload memory. */
    memset(payload, 0, sizeof(*payload));

    /* initialize payload resource. */
    resource_init(
        &payload->hdr,
        &protocolservice_protocol_write_endpoint_message_release);

    /* set init values. */
    payload->alloc = ctx->alloc;
    payload->message_type = PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_PACKET;

    /* create a buffer for holding the response in the message payload. */
    retval =
        vccrypt_buffer_init(
            &payload->payload, &ctx->ctx->vpr_alloc, sizeof(response));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* copy the response to this buffer. */
    memcpy(payload->payload.data, response, sizeof(response));

    /* wrap this payload in a message envelope. */
    retval = message_create(&msg, ctx->alloc, ctx->return_addr, &payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* the payload is now owned by the message. */
    payload = NULL;

    /* send the message to the protocol write endpoint. */
    retval = message_send(ctx->return_addr, msg, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* the message is now owned by the message discipline. */
    msg = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_message:
    if (NULL != msg)
    {
        release_retval = resource_release(message_resource_handle(msg));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        msg = NULL;
    }

cleanup_payload:
    if (NULL != payload)
    {
        release_retval = resource_release(&payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        payload = NULL;
    }

done:
    return retval;
}
