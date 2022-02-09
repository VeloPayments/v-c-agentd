/**
 * \file protocolservice/protocolservice_dataservice_endpoint_fiber_entry.c
 *
 * \brief Entry point for the data service fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service dataservice endpoint fiber.
 *
 * This fiber manages communication with the dataservice instance assigned to
 * the protocol service.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_fiber_entry(void* vctx)
{
    status retval, release_retval;
    protocolservice_dataservice_request_message* req_payload;
    protocolservice_protocol_write_endpoint_message* reply_payload;
    message* req_msg;
    message* reply_msg;
    mailbox_address return_address;
    protocolservice_dataservice_endpoint_context* ctx =
        (protocolservice_dataservice_endpoint_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));

    /* event loop for the data service endpoint. */
    for (;;)
    {
        /* read a message from the message queue. */
        retval = message_receive(ctx->addr, &req_msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* get the request payload. */
        req_payload =
            (protocolservice_dataservice_request_message*)
            message_payload(req_msg, false);

        /* get the return address. */
        return_address = message_return_address(req_msg);

        /* decode and dispatch this request, returning a reply. */
        /* TODO - decouple request and reply by adding reference IDs to
         * dataservice API. */
        retval =
            protocolservice_dataservice_endpoint_decode_and_dispatch(
                ctx, req_payload, return_address, &reply_payload);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_req_msg;
        }

        /* create a response message. */
        retval =
            message_create(
                &reply_msg, ctx->alloc, ctx->addr, &reply_payload->hdr);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_reply_payload;
        }

        /* the payload is now owned by the message. */
        reply_payload = NULL;

        /* send the response message. */
        retval =
            message_send(
                return_address, reply_msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_reply_msg;
        }

        /* the reply message is nown owned by the message discipline. */
        reply_msg = NULL;

        /* clean up the request message. */
        retval = resource_release(message_resource_handle(req_msg));
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
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
        reply_msg = NULL;
    }

cleanup_reply_payload:
    if (NULL != reply_payload)
    {
        release_retval = resource_release(&reply_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        reply_payload = NULL;
    }

cleanup_req_msg:
    release_retval = resource_release(message_resource_handle(req_msg));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
