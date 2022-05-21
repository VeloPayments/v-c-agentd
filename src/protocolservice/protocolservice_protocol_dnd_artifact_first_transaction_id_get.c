/**
 * \file
 * protocolservice/protocolservice_protocol_dnd_artifact_first_transaction_id_get.c
 *
 * \brief Decode and dispatch a get artifact first transaction id get request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_uuid;

/**
 * \brief Decode and dispatch an artifact get first transaction id request.
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
status protocolservice_protocol_dnd_artifact_first_transaction_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    vccrypt_buffer_t reqbuf;
    protocol_req_artifact_first_txn_id_get req;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_artifact_first_txn_id_get(
            &req, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* encode the request to the dataservice endpoint. */
    retval = 0;
        dataservice_encode_request_artifact_get(
            &reqbuf, &ctx->ctx->vpr_alloc, 0U,
            (const rcpr_uuid*)&req.artifact_id);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* send this message to the dataservice endpoint. */
    retval =
        protocolservice_dataservice_send_request(
            ctx, req.request_id, request_offset, &reqbuf);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_reqbuf;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_reqbuf;

cleanup_reqbuf:
    dispose((disposable_t*)&reqbuf);

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
