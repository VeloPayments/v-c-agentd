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
#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>
#include <vccrypt/compare.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

static uint8_t ff_uuid[16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

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
        /* Encode an error response. */
        retval =
            vcblockchain_protocol_encode_error_resp(
                &respbuf, &ctx->ctx->vpr_alloc, payload->original_request_id,
                payload->offset, dresp.hdr.status);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_dresp;
        }
    }
    else
    {
        /* decode the protocol request id to determine the response payload. */
        switch (payload->original_request_id)
        {
            case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_NEXT:
                /* TODO - refactor to extract function. */
                if (!crypto_memcmp(dresp.node.next, ff_uuid, 16))
                {
                    /* Encode an error response. */
                    retval =
                        vcblockchain_protocol_encode_error_resp(
                            &respbuf, &ctx->ctx->vpr_alloc,
                            payload->original_request_id, payload->offset,
                            AGENTD_ERROR_DATASERVICE_NOT_FOUND);
                    if (STATUS_SUCCESS != retval)
                    {
                        goto cleanup_dresp;
                    }
                }
                else
                {
                    /* build a block get next id payload. */
                    retval =
                        vcblockchain_protocol_encode_resp_block_next_id_get(
                            &respbuf, &ctx->ctx->vpr_alloc, payload->offset,
                            dresp.hdr.status, (const vpr_uuid*)dresp.node.next);
                }
                break;

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
                break;
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
