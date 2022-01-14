/**
 * \file protocolservice/protocolservice_protocol_handle_handshake.c
 *
 * \brief Handle the handshake for the client protocol.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Perform the handshake for the protocol.
 *
 * \param ctx       The protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status
protocolservice_protocol_handle_handshake(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval;

    /* read the handshake request from the client. */
    retval = protocolservice_protocol_read_handshake_req(ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* look up the client key. */
    retval =
        protocolservice_authorized_entity_lookup(&ctx->entity, ctx, 
        &ctx->entity_uuid);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* Read random bytes from the random service endpoint. */
    retval = protocolservice_read_random_bytes(ctx);
    if (STATUS_SUCCESS != retval)
    {
        retval =
            protocolservice_write_error_response(
                ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
                AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE, 0U, false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE;
        }
        goto done;
    }

    /* TODO - fill out the rest of the handshake. */
    /* compute the shared secret and the C/R response. */
    /* write the handshake request response. */
    /* read the handshake ack request from the client. */
    /* write the handshake ack response to the client. */

    retval = -1;
    goto done;

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
