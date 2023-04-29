/**
 * \file protocolservice/protocolservice_protocol_dnd_close.c
 *
 * \brief Decode and dispatch a close request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vcblockchain/psock.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a close request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_close(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* /*payload*/, size_t /*payload_size*/)
{
    status retval, release_retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* close the dataservice context. */
    retval =
        protocolservice_protocol_close_data_service_context(ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* set the request shutdown flag. */
    ctx->req_shutdown = true;

    /* success. */
    retval = STATUS_SUCCESS;
    goto write_error_response;

write_error_response:
    /* send the response. */
    release_retval =
        protocolservice_send_error_response_message(
            ctx, UNAUTH_PROTOCOL_REQ_ID_CLOSE, retval, request_offset);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
