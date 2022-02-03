/**
 * \file
 * protocolservice/protocolservice_protocol_decode_and_dispatch.c
 *
 * \brief Decode and dispatch a client protocol packet.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Decode and dispatch a packet from the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_id        The request id of the packet.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_decode_and_dispatch(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_id,
    uint32_t request_offset, const uint8_t* payload, size_t payload_size)
{
    status retval, release_retval;
    (void)payload;
    (void)payload_size;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    switch (request_id)
    {
        default:
            retval = AGENTD_ERROR_PROTOCOLSERVICE_INVALID_REQUEST_ID;
            goto write_error_response;
    }

write_error_response:
    release_retval =
        protocolservice_send_error_response_message(
            ctx, request_id, retval, request_offset);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }
    else
    {
        /* Once the protocol is running, normal error responses aren't fatal. */
        retval = STATUS_SUCCESS;
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
