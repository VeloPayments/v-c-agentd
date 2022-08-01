/**
 * \file protocolservice/protocolservice_protocol_dnd_latest_block_id_get.c
 *
 * \brief Decode and dispatch a latest block id get request.
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
 * \brief Decode and dispatch a latest block id get request.
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
status protocolservice_protocol_dnd_latest_block_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* /*payload*/, size_t /*payload_size*/)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* encode this request. */
    retval =
        dataservice_encode_request_latest_block_id_get(
            &reqbuf, &ctx->ctx->vpr_alloc, 0U);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* send this message to the dataservice endpoint. */
    retval =
        protocolservice_dataservice_send_request(
            ctx, UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET, request_offset,
            &reqbuf);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_reqbuf;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_reqbuf;

cleanup_reqbuf:
    dispose((disposable_t*)&reqbuf);

done:
    return retval;
}
