/**
 * \file
 * protocolservice/protocolservice_protocol_read_decode_and_dispatch_packet.c
 *
 * \brief Read a packet from the client, and decode / dispatch it.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

/**
 * \brief Read a packet from the client socket, and decode / dispatch it.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_decode_and_dispatch_packet(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    void* req = NULL;
    uint32_t size;
    uint32_t request_id;
    uint32_t request_offset;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* attempt to read a packet from the client. */
    retval =
        psock_read_authed_data(
            ctx->protosock, ctx->alloc, ctx->client_iv, &req, &size,
            &ctx->ctx->suite, &ctx->shared_secret);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* if we've read a message, increment the client IV. */
    ++ctx->client_iv;

    /* set up the read buffer pointer. */
    const uint8_t* breq = (const uint8_t*)req;

    /* verify that the size matches what we expect. */
    const size_t request_id_size = sizeof(request_id);
    const size_t request_offset_size = sizeof(request_offset);
    const size_t expected_size =
        request_id_size + request_offset_size;
    if (size < expected_size)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        goto write_error_response;
    }

    /* read the request ID. */
    memcpy(&request_id, breq, request_id_size);
    breq += request_id_size;
    request_id = ntohl(request_id);

    /* read the request offset. */
    memcpy(&request_offset, breq, request_offset_size);
    breq += request_offset_size;
    request_offset = ntohl(request_offset);

    /* decode and dispatch this request. */
    retval =
        protocolservice_protocol_decode_and_dispatch(
            ctx, request_id, request_offset, req, size);

    /* regardless of outcome, clean up the request. */
    goto cleanup_req;

write_error_response:
    release_retval =
        protocolservice_send_error_response_message(ctx, 0, retval, 0U);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_req:
    if (NULL != req)
    {
        memset(req, 0, size);
        free(req);
    }
    req = NULL;

    return retval;
}
