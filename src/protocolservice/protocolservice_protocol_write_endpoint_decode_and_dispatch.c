/**
 * \file
 * protocolservice/protocolservice_protocol_write_endpoint_decode_and_dispatch.c
 *
 * \brief Decode and dispatch a message sent to the protocol write endpoint.
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

/**
 * \brief Decode and dispatch a message sent to the protocol write endpoint.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The message to be decoded and dispatched.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_decode_and_dispatch(
    protocolservice_protocol_fiber_context* ctx, RCPR_SYM(message)* msg)
{
    protocolservice_protocol_write_endpoint_message* payload = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(prop_message_valid(msg));

    /* get the payload for this message. */
    payload =
        (protocolservice_protocol_write_endpoint_message*)message_payload(
            msg, false);

    /* verify cast. */
    MODEL_ASSERT(
        prop_protocolservice_protocol_write_endpoint_message_valid(payload));

    /* decode the message type. */
    switch (payload->message_type)
    {
        case PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_MESSAGE_SHUTDOWN:
            ctx->shutdown = true;
            return STATUS_SUCCESS;

        /* decode and dispatch for datasservice response messages. */
        case PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_DATASERVICE_MSG:
            return
                protocolservice_pwe_dnd_dataservice_message(
                    ctx, payload);

        /* TODO - add decode and dispatch for notification service responses. */
        case PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_NOTIFICATION_MSG:
            return STATUS_SUCCESS;

        /* Handle the write endpoint packet request. */
        case PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_PACKET:
            return
                protocolservice_protocol_write_endpoint_write_packet(
                    ctx, payload);

        default:
            return AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
    }
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
