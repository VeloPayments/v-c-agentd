/**
 * \file
 * protocolservice/protocolservice_pwe_dnd_dataservice_artifact_get.c
 *
 * \brief Decode and dispatch a dataservice artifact get response.
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

/* forward decls. */
static status protocolservice_pwe_dnd_encode_protocol_artifact_first_txn_get(
    vccrypt_buffer_t* respbuf, protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload,
    const dataservice_response_artifact_get_t* dresp);
static status protocolservice_pwe_dnd_encode_protocol_artifact_last_txn_get(
    vccrypt_buffer_t* respbuf, protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload,
    const dataservice_response_artifact_get_t* dresp);

/**
 * \brief Decode and dispatch an artifact read response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_artifact_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload)
{
    status retval;
    dataservice_response_artifact_get_t dresp;
    vccrypt_buffer_t respbuf;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_protocol_write_endpoint_mesasge_valid(payload));

    /* decode the response. */
    retval =
        dataservice_decode_response_artifact_get(
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
            case UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_FIRST_TXN_BY_ID_GET:
            default:
                retval =
                    protocolservice_pwe_dnd_encode_protocol_artifact_first_txn_get(
                        &respbuf, ctx, payload, &dresp);
                break;

            case UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_LAST_TXN_BY_ID_GET:
                retval =
                    protocolservice_pwe_dnd_encode_protocol_artifact_last_txn_get(
                        &respbuf, ctx, payload, &dresp);
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

/**
 * \brief Encode an artifact get first transaction response.
 *
 * \param respbuf       The buffer in which the response is stored.
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 * \param dresp         The decoded response from the data service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status protocolservice_pwe_dnd_encode_protocol_artifact_first_txn_get(
    vccrypt_buffer_t* respbuf, protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload,
    const dataservice_response_artifact_get_t* dresp)
{
    return
        vcblockchain_protocol_encode_resp_artifact_first_txn_id_get(
            respbuf, &ctx->ctx->vpr_alloc, payload->offset,
            dresp->hdr.status, (const vpr_uuid*)dresp->record.txn_first);
}

/**
 * \brief Encode an artifact get last transaction response.
 *
 * \param respbuf       The buffer in which the response is stored.
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 * \param dresp         The decoded response from the data service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status protocolservice_pwe_dnd_encode_protocol_artifact_last_txn_get(
    vccrypt_buffer_t* respbuf, protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload,
    const dataservice_response_artifact_get_t* dresp)
{
    return
        vcblockchain_protocol_encode_resp_artifact_last_txn_id_get(
            respbuf, &ctx->ctx->vpr_alloc, payload->offset,
            dresp->hdr.status, (const vpr_uuid*)dresp->record.txn_latest);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
