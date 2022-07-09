/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_extended_api_sendrecv.c
 *
 * \brief Decode and dispatch an extended API send request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/protocolservice_capabilities.h>
#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch an extended API send request.
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
status protocolservice_protocol_dnd_extended_api_sendrecv(
    protocolservice_protocol_fiber_context* ctx, uint32_t /*request_offset*/,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    protocol_req_extended_api req;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

#if 0
    /* perform a capability check for this operation. */
    if (!
        protocolservice_authorized_entity_capability_check(
            ctx->entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_SENDRECV,
            &ctx->ctx->agentd_uuid))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
        goto done;
    }
#endif

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_extended_api(
            &req, &ctx->ctx->vpr_alloc, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* attempt to send this request to the appropriate sentinel. */
    retval = protocolservice_protocol_extended_api_send_req(ctx, &req);
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
