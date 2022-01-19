/**
 * \file protocolservice/protocolservice_protocol_write_handshake_ack_resp.c
 *
 * \brief Write the handshake ack response.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_psock;

/**
 * \brief Write the handshake ack response to the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_handshake_ack_resp(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;

    /* Build the ack payload. */
    uint32_t payload[3] = {
        htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_ACKNOWLEDGE),
        htonl(AGENTD_STATUS_SUCCESS),
        htonl(0)
    };

    /* write this payload to the socket. */
    retval =
        psock_write_authed_data(
            ctx->protosock, ctx->server_iv, payload, sizeof(payload),
            &ctx->ctx->suite, &ctx->shared_secret);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* update the server iv on success. */
    ++ctx->server_iv;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

write_error_response:
    release_retval =
        protocolservice_write_error_response(
            ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_ACKNOWLEDGE,
            retval, 0U, true);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
