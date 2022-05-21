/**
 * \file protocolservice/protocolservice_pwe_dnd_dataservice_message.c
 *
 * \brief Decode and dispatch a dataservice response message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "protocolservice_internal.h"

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
    /* make working with the payload more convenient. */
    const uint32_t* uresp = (const uint32_t*)payload->payload.data;

    /* get the API method. */
    uint32_t method = ntohl(uresp[0]);

    /* decode method. */
    switch (method)
    {
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ:
            return
                protocolservice_pwe_dnd_dataservice_block_id_latest_get(
                    ctx, payload);

        case DATASERVICE_API_METHOD_APP_BLOCK_READ:
            return
                protocolservice_pwe_dnd_dataservice_block_get(
                    ctx, payload);

        case DATASERVICE_API_METHOD_APP_TRANSACTION_READ:
            return
                protocolservice_pwe_dnd_dataservice_transaction_get(
                    ctx, payload);

        case DATASERVICE_API_METHOD_APP_ARTIFACT_READ:
            return
                protocolservice_pwe_dnd_dataservice_artifact_get(ctx, payload);

        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT:
            return
                protocolservice_pwe_dnd_dataservice_transaction_submit(
                    ctx, payload);

        case DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ:
            return
                protocolservice_pwe_dnd_dataservice_block_id_by_height_get(
                    ctx, payload);

        default:
            return AGENTD_ERROR_PROTOCOLSERVICE_DATASERVICE_INVALID_RESPONSE_ID;
    }
}
