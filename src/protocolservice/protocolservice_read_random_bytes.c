/**
 * \file protocolservice/protocolservice_read_random_bytes.c
 *
 * \brief Read random bytes from the random service endpoint.
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
 * \brief Read random bytes from the random service endpoint.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_read_random_bytes(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    message* req_message = NULL;
    message* resp_message = NULL;
    protocolservice_random_request_message* req_payload = NULL;
    protocolservice_random_response_message* resp_payload = NULL;

    /* create the random request message. */
    retval =
        protocolservice_random_request_message_create(
            &req_payload, ctx->alloc,
            ctx->server_challenge_nonce.size + ctx->server_key_nonce.size);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* create the message to send to the random endpoint. */
    retval =
        message_create(
            &req_message, ctx->alloc, ctx->return_addr, &req_payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_payload;
    }

    /* the request payload is now owned by the request message. */
    req_payload = NULL;

    /* send the message to the random endpoint. */
    retval =
        message_send(
            ctx->ctx->random_endpoint_addr, req_message, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req_message;
    }

    /* the request message is now owned by the message discipline. */
    req_message = NULL;

    /* read the response from the random endpoint. */
    retval =
        message_receive(ctx->return_addr, &resp_message, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* get the message payload. */
    resp_payload =
        (protocolservice_random_response_message*)message_payload(
            resp_message, false);
    MODEL_ASSERT(prop_valid_random_response_message(resp_payload));
    MODEL_ASSERT(
        resp_payload->size
            == ctx->server_challenge_nonce.size
                + ctx->server_key_nonce.size);

    /* get a byte pointer to the data. */
    const uint8_t* bdata = (const uint8_t*)resp_payload->data;

    /* copy the challenge nonce bytes. */
    memcpy(
        ctx->server_challenge_nonce.data, bdata,
        ctx->server_challenge_nonce.size);

    /* increment the byte pointer. */
    bdata += ctx->server_challenge_nonce.size;

    /* copy the key nonce bytes. */
    memcpy(ctx->server_key_nonce.data, bdata, ctx->server_key_nonce.size);

    /* clean up the response message. */
    retval = resource_release(message_resource_handle(resp_message));
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_req_message:
    if (NULL != req_message)
    {
        release_retval = resource_release(message_resource_handle(req_message));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        req_message = NULL;
    }

cleanup_req_payload:
    if (NULL != req_payload)
    {
        release_retval = resource_release(&req_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        req_payload = NULL;
    }

write_error_response:
    retval =
        protocolservice_write_error_response(
            ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE, 0U, false);
    if (STATUS_SUCCESS == retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE;
    }

done:
    return retval;
}
