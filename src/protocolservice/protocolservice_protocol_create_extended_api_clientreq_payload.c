/**
 * \file
 * protocolservice/protocolservice_protocol_create_extended_api_clientreq_payload.c
 *
 * \brief Create a payload for the client request to send to the sentinel.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Create an extended API client request payload buffer.
 *
 * \param buffer        Uninitialized buffer to receive the payload on success.
 * \param offset        Pointer to receive the offset on success.
 * \param ctx           The sending client's context.
 * \param entry         The receiving sentinel's routing entry.
 * \param req           The extended API request to send to the sentinel.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_create_extended_api_clientreq_payload(
    vccrypt_buffer_t* buffer, uint64_t* offset,
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_extended_api_dict_entry* entry,
    const protocol_req_extended_api* req)
{
    /* assign the new offset. */
    entry->ctx->extended_api_offset += 1;
    *offset = entry->ctx->extended_api_offset;

    /* create the client request. */
    return
        vcblockchain_protocol_encode_resp_extended_api_client_request(
            buffer, &ctx->ctx->vpr_alloc, *offset,
            (const vpr_uuid*)&ctx->entity_uuid, &req->verb_id,
            &ctx->entity->encryption_pubkey, &ctx->entity->signing_pubkey,
            &req->request_body);
}
