/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_status_get.c
 *
 * \brief Decode and dispatch a status get request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_uuid;

/**
 * \brief Decode and dispatch a status get request.
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
status protocolservice_protocol_dnd_status_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    protocol_req_status_get req;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_status_get(
            &req, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* send a response to this status call. */
    retval =
        protocolservice_send_error_response_message(
            ctx, req.request_id, STATUS_SUCCESS, request_offset);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_req;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
