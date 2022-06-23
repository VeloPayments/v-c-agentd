/**
 * \file
 * protocolservice/protocolservice_pwe_dnd_notificationservice_message.c
 *
 * \brief Decode and dispatch a notificationservice message response.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <vcblockchain/protocol/serialization.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a response message from the notificationservice.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_notificationservice_message(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload)
{
    status retval;
    vccrypt_buffer_t respbuf;

    /* encode a generic response. */
    retval =
        vcblockchain_protocol_encode_resp_generic(
            &respbuf, &ctx->ctx->vpr_alloc, payload->original_request_id,
            payload->offset, STATUS_SUCCESS, NULL, 0U);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* write this payload to the socket. */
    retval =
        protocolservice_protocol_write_endpoint_write_raw_packet(
            ctx, respbuf.data, respbuf.size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_respbuf;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_respbuf;

cleanup_respbuf:
    dispose((disposable_t*)&respbuf);

done:
    return retval;
}
