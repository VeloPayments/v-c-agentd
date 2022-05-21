/**
 * \file protocolservice/protocolservice_dataservice_send_request.c
 *
 * \brief Send a request message to the dataservice endpoint.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Send a message to the dataservice endpoint.
 *
 * \note This function takes ownership of the contents of the request buffer on
 * success. These contents are moved to the internal message sent to the
 * endpoint and are no longer available to the caller when ownership is taken.
 *
 * \param ctx               The protocol fiber context.
 * \param protocol_req_id   The protocol request id.
 * \param request_offset    The protocol request offset of the message.
 * \param request_buffer    The buffer holding the encoded request message.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_send_request(
    protocolservice_protocol_fiber_context* ctx, uint32_t protocol_req_id,
    uint32_t request_offset, vccrypt_buffer_t* request_buffer)
{
    status retval, release_retval;
    message* request = NULL;
    protocolservice_dataservice_request_message* request_payload = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(prop_vccrypt_buffer_valid(request_buffer));

    /* create the request payload. */
    retval =
        protocolservice_dataservice_request_message_create(
            &request_payload, ctx, protocol_req_id,
            PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_DATASERVICE_REQ,
            request_offset, 0U, request_buffer);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create the request message. */
    retval =
        message_create(
            &request, ctx->alloc, ctx->return_addr, &request_payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_request_payload;
    }

    /* the request payload is now owned by the request message. */
    request_payload = NULL;

    /* send the request message. */
    retval =
        message_send(ctx->ctx->data_endpoint_addr, request, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_request;
    }

    /* the request message is now owned by the messaging discipline. */
    request = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_request:
    if (NULL != request)
    {
        release_retval = resource_release(message_resource_handle(request));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        request = NULL;
    }

cleanup_request_payload:
    if (NULL != request_payload)
    {
        release_retval = resource_release(&request_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        request_payload = NULL;
    }

done:
    return retval;
}
