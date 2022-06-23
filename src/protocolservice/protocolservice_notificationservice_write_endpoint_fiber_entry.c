/**
 * \file
 * protocolservice/protocolservice_notificationservice_write_endpoint_fiber_entry.c
 *
 * \brief Entry point for the notificationservice write endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service notificationservice write
 * endpoint fiber.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_write_endpoint_fiber_entry(
    void* vctx)
{
    status retval, release_retval;
    uint8_t* buf;
    size_t size;
    uint32_t method_id, status_code;
    uint32_t return_offset;
    uint64_t offset;
    const uint8_t* payload;
    size_t payload_size;
    bool entry_found;
    mailbox_address return_address;
    message* reply_msg;
    protocolservice_notificationservice_fiber_context* ctx =
        (protocolservice_notificationservice_fiber_context*)vctx;
    protocolservice_protocol_write_endpoint_message* reply_payload;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));

    /* loop while we are not quiescing. */
    while (!ctx->ctx->quiesce)
    {
        /* read a response from the notificationservice API. */
        retval =
            notificationservice_api_recvresp(
                ctx->notifysock, ctx->alloc, &buf, &size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_ctx;
        }

        /* decode the response. */
        retval =
            notificationservice_api_decode_response(
                buf, size, &method_id, &status_code, &offset, &payload,
                &payload_size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_buf;
        }

        /* look up the return address and offset. */
        retval =
            protocolservice_notificationservice_lookup_return_address_from_offset(
                &return_address, &return_offset, ctx, offset);
        if (STATUS_SUCCESS != retval)
        {
            entry_found = false;
        }
        else
        {
            entry_found = true;
        }

        /* TODO - handle different method IDs. */

        if (entry_found)
        {
            /* create the response payload. */
            /* TODO - cache client offset so it can be returned here. */
            retval =
                protocolservice_protocol_write_endpoint_message_create(
                    &reply_payload, ctx->ctx,
                    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_NOTIFICATION_MSG,
                    UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID,
                    return_offset, NULL, 0U);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_buf;
            }

            /* create the response message. */
            retval =
                message_create(
                    &reply_msg, ctx->alloc, ctx->notify_addr,
                    &reply_payload->hdr);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_reply_payload;
            }

            /* the reply payload is now owned by the message. */
            reply_payload = NULL;

            /* send the response message. */
            retval = message_send(return_address, reply_msg, ctx->msgdisc);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_reply_msg;
            }

            /* the reply message is now owned by the message discipline. */
            reply_msg = NULL;
        }

        /* clean up the response buffer. */
        retval = rcpr_allocator_reclaim(ctx->alloc, buf);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_ctx;
        }
    }

cleanup_reply_msg:
    if (NULL != reply_msg)
    {
        release_retval = resource_release(message_resource_handle(reply_msg));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_reply_payload:
    if (NULL != reply_payload)
    {
        release_retval = resource_release(&reply_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_buf:
    release_retval = rcpr_allocator_reclaim(ctx->alloc, buf);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
