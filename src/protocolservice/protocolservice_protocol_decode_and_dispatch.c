/**
 * \file protocolservice/protocolservice_protocol_decode_and_dispatch.c
 *
 * \brief Decode and dispatch a client protocol packet.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a packet from the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_id        The request id of the packet.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_decode_and_dispatch(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_id,
    uint32_t request_offset, const uint8_t* payload, size_t payload_size)
{
    status retval, release_retval;
    (void)payload;
    (void)payload_size;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);

    switch (request_id)
    {
        case UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET:
            retval =
                protocolservice_protocol_dnd_latest_block_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT:
            retval =
                protocolservice_protocol_dnd_transaction_submit(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_BY_ID_GET:
            retval =
                protocolservice_protocol_dnd_block_by_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_NEXT:
            retval =
                protocolservice_protocol_dnd_block_id_next_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_PREV:
            retval =
                protocolservice_protocol_dnd_block_id_prev_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_BY_HEIGHT_GET:
            retval =
                protocolservice_protocol_dnd_block_id_by_height_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_BY_ID_GET:
            retval =
                protocolservice_protocol_dnd_transaction_by_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_NEXT:
            retval =
                protocolservice_protocol_dnd_transaction_id_next_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_PREV:
            retval =
                protocolservice_protocol_dnd_transaction_id_prev_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_BLOCK_ID:
            retval =
                protocolservice_protocol_dnd_transaction_block_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_FIRST_TXN_BY_ID_GET:
            retval =
                protocolservice_protocol_dnd_artifact_first_transaction_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_LAST_TXN_BY_ID_GET:
            retval =
                protocolservice_protocol_dnd_artifact_last_transaction_id_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID:
            retval =
                protocolservice_protocol_dnd_assert_latest_block_id(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID_CANCEL:
            retval =
                protocolservice_protocol_dnd_assert_latest_block_id_cancel(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_STATUS_GET:
            retval =
                protocolservice_protocol_dnd_status_get(
                    ctx, request_offset, payload, payload_size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_CLOSE:
            retval =
                protocolservice_protocol_dnd_close(
                    ctx, request_offset, payload, payload_size);
            break;

        default:
            retval = AGENTD_ERROR_PROTOCOLSERVICE_INVALID_REQUEST_ID;
            break;
    }

    /* if the request failed, send an error response. */
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* if we made it this far, we succeeded. */
    goto done;

write_error_response:
    release_retval =
        protocolservice_send_error_response_message(
            ctx, request_id, retval, request_offset);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }
    else
    {
        /* Once the protocol is running, normal error responses aren't fatal. */
        retval = STATUS_SUCCESS;
    }

done:
    return retval;
}
