/**
 * \file
 * protocolservice/protocolservice_pwe_dnd_dataservice_block_id_latest_get.c
 *
 * \brief Handle a block id latest get response.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/psock.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_psock;

/**
 * \brief Decode and dispatch a latest block id get response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_block_id_latest_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload)
{
    status retval;
    dataservice_response_latest_block_id_get_t dresp;
    vccrypt_buffer_t respbuf;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_protocol_write_endpoint_mesasge_valid(payload));

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
    retval =
        vcblockchain_protocol_encode_resp_latest_block_id_get(
            &respbuf, &ctx->ctx->vpr_alloc, payload->offset, dresp.hdr.status,
            (const vpr_uuid*)dresp.block_id);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_dresp;
    }

    /* write this payload to the socket. */
    retval = 
        protocolservice_protocol_write_endpoint_write_raw_packet(
            ctx, respbuf.data, respbuf.size);

    /* clean up. */
    goto cleanup_respbuf;

cleanup_respbuf:
    dispose((disposable_t*)&respbuf);

cleanup_dresp:
    dispose((disposable_t*)&dresp);

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
