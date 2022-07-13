/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_extended_api_sendresp.c
 *
 * \brief Decode and dispatch an extended API client response request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/protocolservice_capabilities.h>
#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch an extended API client response request.
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
status protocolservice_protocol_dnd_extended_api_sendresp(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size)
{
    (void)ctx;
    (void)request_offset;
    (void)payload;
    (void)payload_size;

    return -1;
}