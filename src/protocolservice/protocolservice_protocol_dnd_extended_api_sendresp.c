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

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

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
    protocolservice_protocol_fiber_context* ctx, uint32_t /*request_offset*/,
    const uint8_t* payload, size_t payload_size)
{
    status retval, release_retval;
    protocolservice_extended_api_response_xlat_entry* entry;
    protocol_req_extended_api_response req;
    protocolservice_protocol_write_endpoint_message* msg_payload = NULL;
    message* msg = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    /* perform a capability check for this operation. */
    if (!
        protocolservice_authorized_entity_capability_check(
            ctx->entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_RESP,
            &ctx->ctx->agentd_uuid))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
        goto done;
    }

    /* decode the request. */
    retval =
        vcblockchain_protocol_decode_req_extended_api_response(
            &req, &ctx->ctx->vpr_alloc, payload, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* attempt to look up the response entity. */
    retval =
        rbtree_find(
            (resource**)&entry, ctx->extended_api_offset_dict, &req.offset);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* allocate memory for the message payload. */
    retval =
        rcpr_allocator_allocate(
            ctx->alloc, (void**)&msg_payload, sizeof(*msg_payload));
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* clear payload memory. */
    memset(msg_payload, 0, sizeof(*msg_payload));

    /* initialize payload resource. */
    resource_init(
        &msg_payload->hdr,
        &protocolservice_protocol_write_endpoint_message_release);

    /* set init values. */
    msg_payload->alloc = ctx->alloc;
    msg_payload->message_type = PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_PACKET;

    /* create the client response. */
    retval =
        vcblockchain_protocol_encode_resp_extended_api(
            &msg_payload->payload, &ctx->ctx->vpr_alloc, entry->client_offset,
            req.status, &req.response_body);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* wrap this payload in a message envelope. */
    retval =
        message_create(&msg, ctx->alloc, ctx->return_addr, &msg_payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* the payload is now owned by the message. */
    msg_payload = NULL;

    /* send the message to the protocol write endpoint. */
    retval = message_send(entry->client_return_address, msg, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* the message is now owned by the message discipline. */
    msg = NULL;

    /* remove the entry from the translation table. */
    retval = rbtree_delete(NULL, ctx->extended_api_offset_dict, &req.offset);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* send a response letting the sentinel know that this request was
     * successful. */
    /* TODO - the offset here is truncated. That's an odd quirk. */
    retval =
        protocolservice_send_error_response_message(
            ctx, UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRESP, STATUS_SUCCESS,
            req.offset);
    goto cleanup_req;

cleanup_message:
    if (NULL != msg)
    {
        release_retval = resource_release(message_resource_handle(msg));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_payload:
    if (NULL != msg_payload)
    {
        release_retval = resource_release(&msg_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
