/**
 * \file protocolservice/protocolservice_close_data_service_context.c
 *
 * \brief Send a request to the data service endpoint to close the context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Close the data service context for this connection.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_close_data_service_context(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    message* request = NULL;
    protocolservice_dataservice_request_message* request_payload = NULL;
    message* response = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* create the request payload. */
    retval =
        protocolservice_dataservice_request_message_create(
            &request_payload, ctx, 0U,
            PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_CONTEXT_CLOSE,
            0U, ctx->return_addr, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create the request message. */
    retval =
        message_create(&request, ctx->alloc, ctx->fiber_addr,
        &request_payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_request_payload;
    }

    /* the request payload is now owned by the request message. */
    request_payload = NULL;

    /* send the request message. */
    retval =
        message_send(ctx->ctx->data_endpoint_addr, request, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_request;
    }

    /* the request message is now owned by the messaging discipline. */
    request = NULL;

    /* receive the response message. */
    retval = message_receive(ctx->fiber_addr, &response, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* the context is now closed. */
    ctx->dataservice_context_opened = false;

    /* release the response message. */
    release_retval = resource_release(message_resource_handle(response));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    /* return the status code of the response. */
    goto done;

cleanup_request:
    if (NULL != request)
    {
        release_retval = resource_release(message_resource_handle(request));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        request = NULL;
    }

cleanup_request_payload:
    if (NULL != request_payload)
    {
        release_retval = resource_release(&request_payload->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        request_payload = NULL;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
