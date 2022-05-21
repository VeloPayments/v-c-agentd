/**
 * \file protocolservice/protocolservice_random_endpoint_fiber_entry.c
 *
 * \brief Entry point for the random service endpoint.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service random endpoint fiber.
 *
 * This fiber forwards requests to the random service and returns responses.
 *
 * \param vctx          The type erased random endopint context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_endpoint_fiber_entry(void* vctx)
{
    status retval, release_retval;
    protocolservice_random_request_message* req_payload;
    protocolservice_random_response_message* reply_payload;
    message* req_msg;
    message* reply_msg;
    uint32_t offset, status;

    protocolservice_random_endpoint_context* ctx =
        (protocolservice_random_endpoint_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_random_endpoint_context_valid(ctx));

    /* event loop for random service. */
    for (;;)
    {
        /* read a message from the message queue. */
        retval = message_receive(ctx->addr, &req_msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* get the req_payload. */
        req_payload =
            (protocolservice_random_request_message*)
            message_payload(req_msg, false);

        /* create the response payload. */
        retval =
            protocolservice_random_response_message_create(
                &reply_payload, ctx->alloc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_req_msg;
        }

        /* send the api request to the random service. */
        retval =
            random_service_api_sendreq_random_bytes_get(
                ctx->randomsock, 0, req_payload->size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_reply_payload;
        }

        /* receive the response from the random service. */
        retval =
            random_service_api_recvresp_random_bytes_get(
                ctx->randomsock, ctx->alloc, &offset, &status,
                &reply_payload->data, &reply_payload->size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_reply_payload;
        }

        /* create the response message. */
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
                message_return_address(req_msg), reply_msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_reply_msg;
        }

        /* the reply message is now owned by the message discipline. */
        reply_msg = NULL;

        /* clean up request message. */
        retval = resource_release(message_resource_handle(req_msg));
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }
    }

    /* successful termination. */
    retval = STATUS_SUCCESS;
    goto cleanup_context;

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
