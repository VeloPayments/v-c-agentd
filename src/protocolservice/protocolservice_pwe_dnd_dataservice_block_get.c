/**
 * \file
 * protocolservice/protocolservice_pwe_dnd_dataservice_block_get.c
 *
 * \brief Decode and dispatch a dataservice block get response.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/inet.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Decode and dispatch a block read response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_block_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload)
{
    status retval;
    dataservice_response_block_get_t dresp;
    vccrypt_buffer_t respbuf;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_protocol_write_endpoint_mesasge_valid(payload));

    /* decode the response. */
    retval =
        dataservice_decode_response_block_get(
            payload->payload.data, payload->payload.size, &dresp);
    if (STATUS_SUCCESS != retval)
    {
        /* TODO - log fatal error here. */
        goto done;
    }

    /* check to see if the call succeeded. */
    if (STATUS_SUCCESS != dresp.hdr.status)
    {
        /* TODO - turn this into an encode method. */
        uint8_t out[3 * sizeof(uint32_t)];
        uint32_t net_req_id = htonl(payload->original_request_id);
        memcpy(out, &net_req_id, 4);
        uint32_t net_status = htonl(dresp.hdr.status);
        memcpy(out + 4, &net_status, 4);
        uint32_t net_offset = htonl(payload->offset);
        memcpy(out + 8, &net_offset, 4);

        /* create the response buffer. */
        retval =
            vccrypt_buffer_init(&respbuf, &ctx->ctx->vpr_alloc, sizeof(out));
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_dresp;
        }

        /* copy the output to this buffer. */
        memcpy(respbuf.data, out, sizeof(out));
    }
    else
    {
        /* decode the protocol request id to determine the response payload. */
        switch (payload->original_request_id)
        {
            default:
                /* build a block get payload. */
                retval =
                    vcblockchain_protocol_encode_resp_block_get(
                        &respbuf, &ctx->ctx->vpr_alloc, payload->offset,
                        dresp.hdr.status, (const vpr_uuid*)dresp.node.key,
                        (const vpr_uuid*)dresp.node.prev,
                        (const vpr_uuid*)dresp.node.next,
                        (const vpr_uuid*)dresp.node.first_transaction_id,
                        ntohll(dresp.node.net_block_height),
                        ntohll(dresp.node.net_block_cert_size),
                        dresp.data, dresp.data_size);
        }
    }

    /* check the result of the payload build. */
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
