/**
 * \file protocolservice/protocolservice_pwe_dnd_dataservice_message.c
 *
 * \brief Decode and dispatch a dataservice response message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/psock.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_psock;

/**
 * \brief Decode and dispatch a response message from the data service.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_message(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload)
{
    status retval;
    dataservice_response_latest_block_id_get_t dresp;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_protocol_write_endpoint_message_valid(payload));

    /* decode the response. */
    retval =
        dataservice_decode_response_latest_block_id_get(
            payload->payload.data, payload->payload.size, &dresp);
    if (STATUS_SUCCESS != retval)
    {
        /* TODO - log fatal error here. */
        goto done;
    }

    /* build the payload. */
    /* TODO - turn this into an encode function. */
    uint32_t net_method = htonl(UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET);
    uint32_t net_status = htonl(dresp.hdr.status);
    uint32_t net_offset = htonl(payload->offset);
    uint8_t packet[3 * sizeof(uint32_t) + 16];
    memcpy(packet, &net_method, 4);
    memcpy(packet + 4, &net_status, 4);
    memcpy(packet + 8, &net_offset, 4);
    memcpy(packet + 12, dresp.block_id, 16);

    /* write this payload to the socket. */
    /* TODO - write generic method for writing raw encrypted packets. */
    retval = 
        psock_write_authed_data(
            ctx->protosock, ctx->server_iv, packet, sizeof(packet),
            &ctx->ctx->suite, &ctx->shared_secret);

    /* update the server iv. */
    ++ctx->server_iv;

    /* clean up. */
    goto cleanup_dresp;

cleanup_dresp:
    dispose((disposable_t*)&dresp);

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
