/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_assert_latest_block_id_cancel.c
 *
 * \brief Decode and dispatch a latest block id assertion cancellation request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/protocolservice_capabilities.h>
#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a block assertion cancellation request.
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
status protocolservice_protocol_dnd_assert_latest_block_id_cancel(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    protocol_req_assert_latest_block_id_cancel req;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    /* TODO - perform a capability check for this operation. */

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_assert_latest_block_id_cancel(
            &req, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* if the assertion is NOT set, then it can't be unset. */
    if (!ctx->latest_block_id_assertion_set)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_BLOCK_ASSERTION_NOT_SET;
        goto cleanup_req;
    }

    /* send request to notification service endpoint. */
    retval =
        protocolservice_notificationservice_handle_assert_block_cancel_request(
            ctx, request_offset);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
