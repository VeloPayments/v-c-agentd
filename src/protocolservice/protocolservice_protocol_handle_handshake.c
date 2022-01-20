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

    /* if the private key hasn't been set, we can't do a handshake. */
    if (!ctx->ctx->private_key_set)
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_MISSING_PRIVATE_KEY;
    }

    /* read the handshake request from the client. */
    retval = protocolservice_protocol_read_handshake_req(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* look up the client key. */
    retval =
        protocolservice_authorized_entity_lookup(&ctx->entity, ctx, 
        &ctx->entity_uuid);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* Read random bytes from the random service endpoint. */
    retval = protocolservice_read_random_bytes(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* compute the shared secret and the C/R response. */
    retval = protocolservice_compute_shared_secret(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the handshake request response. */
    retval = protocolservice_protocol_write_handshake_req_resp(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* read the handshake ack request from the client. */
    retval = protocolservice_protocol_read_handshake_ack_req(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the handshake ack response to the client. */
    retval = protocolservice_protocol_write_handshake_ack_resp(ctx);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* success. */
    return STATUS_SUCCESS;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
