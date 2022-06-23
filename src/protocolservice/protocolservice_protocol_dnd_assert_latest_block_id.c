/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_assert_latest_block_id.c
 *
 * \brief Decode and dispatch a latest block id assertion request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a block assertion request.
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
status protocolservice_protocol_dnd_assert_latest_block_id(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    protocol_req_assert_latest_block_id req;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_assert_latest_block_id(
            &req, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* TODO - capabilities check. */

    /* if the assertion is already set, it can't be set again. */
    if (ctx->latest_block_id_assertion_set)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_BLOCK_ASSERTION_ALREADY_SET;
        goto cleanup_req;
    }

    /* send request to notification service endpoint. */
    retval =
        protocolservice_notificationservice_handle_assert_block_request(
            ctx, req.offset, &req.latest_block_id,
            &ctx->latest_block_id_assertion_server_offset);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* save offset for notification service. */
    ctx->latest_block_id_assertion_client_offset = request_offset;
    ctx->latest_block_id_assertion_set = true;

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_req;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
