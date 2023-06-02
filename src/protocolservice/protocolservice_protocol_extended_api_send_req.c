/**
 * \file
 * protocolservice/protocolservice_protocol_extended_api_send_req.c
 *
 * \brief Look up a sentinel and forward a request to it.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Forward an extended API request to the appropriate sentinel.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      operation.
 * \param req           The request to forward.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_extended_api_send_req(
    protocolservice_protocol_fiber_context* ctx,
    const protocol_req_extended_api* req)
{
    status retval, release_retval;
    protocolservice_extended_api_dict_entry* entry;
    protocolservice_protocol_write_endpoint_message* payload = NULL;
    message* msg = NULL;
    uint64_t clientreq_offset;

    /* perform a capability check to ensure that this entity is allowed to
     * perform the requested verb on the requested entity. */
    if (!
        protocolservice_authorized_entity_capability_check(
            ctx->entity, &ctx->entity_uuid, (const rcpr_uuid*)&req->verb_id,
            (const rcpr_uuid*)&req->entity_id))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
        goto done;
    }

    /* attempt to look up the entity route mapping. */
    retval =
        rbtree_find(
            (resource**)&entry, ctx->ctx->extended_api_dict, &req->entity_id);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_EXTENDED_API_UNKNOWN_ENTITY;
        goto done;
    }

    /* allocate memory for the message payload. */
    retval =
        rcpr_allocator_allocate(ctx->alloc, (void**)&payload, sizeof(*payload));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear payload memory. */
    memset(payload, 0, sizeof(*payload));

    /* initialize payload resource. */
    resource_init(
        &payload->hdr,
        &protocolservice_protocol_write_endpoint_message_release);

    /* set init values. */
    payload->alloc = ctx->alloc;
    payload->message_type = PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_PACKET;

    /* create the client request response payload. */
    retval =
        protocolservice_protocol_create_extended_api_clientreq_payload(
            &payload->payload, &clientreq_offset, ctx, entry, req);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* wrap this payload in a message envelope. */
    retval = message_create(&msg, ctx->alloc, ctx->return_addr, &payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* the payload is now owned by the message. */
    payload = NULL;

    /* if response flag is set, make sure this entry is in the xlat table. */
    if (entry->ctx->extended_api_can_respond)
    {
        /* TODO - if message send fails below, we need to remove this entry. */
        retval =
            protocolservice_extended_api_response_xlat_entry_add(
                entry->ctx, clientreq_offset, req->offset, ctx->return_addr);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_message;
        }
    }

    /* send the message to the protocol write endpoint. */
    retval = message_send(entry->ctx->return_addr, msg, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* the message is now owned by the message discipline. */
    msg = NULL;

    /* if the response flag is NOT set, send a response message. */
    if (!entry->ctx->extended_api_can_respond)
    {
        retval =
            protocolservice_send_error_response_message(
                ctx, UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV,
                STATUS_SUCCESS, req->offset);
    }
    else
    {
        retval = STATUS_SUCCESS;
    }

    /* exit. */
    goto done;

cleanup_message:
    if (NULL != msg)
    {
        release_retval = resource_release(message_resource_handle(msg));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        msg = NULL;
    }

cleanup_payload:
    if (NULL != payload)
    {
        release_retval = resource_release(&payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        payload = NULL;
    }

done:
    return retval;
}
