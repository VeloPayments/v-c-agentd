/**
 * \file protocolservice/protocolservice_protocol_shutdown_write_endpoint.c
 *
 * \brief Send a shutdown message to the write endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Instruct the write endpoint fiber to shut down.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_shutdown_write_endpoint(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    protocolservice_protocol_write_endpoint_message* payload = NULL;
    message* msg = NULL;

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
    payload->message_type =
        PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_MESSAGE_SHUTDOWN;

    /* wrap this payload in a message envelope. */
    retval = message_create(&msg, ctx->alloc, ctx->return_addr, &payload->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* the payload is now owned by the message. */
    payload = NULL;

    /* send the message to the protocol write endpoint. */
    retval = message_send(ctx->return_addr, msg, ctx->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_message;
    }

    /* the message is now owned by the message discipline. */
    msg = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
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

#endif /* defined(AGENTD_NEW_PROTOCOL) */
