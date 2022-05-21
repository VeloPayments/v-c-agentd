/**
 * \file
 * protocolservice/protocolservice_protocol_write_endpoint_write_packet.c
 *
 * \brief Write a packet to the peer.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

/**
 * \brief Write a packet to the peer.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The packet message to be written
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_write_packet(
    protocolservice_protocol_fiber_context* ctx,
    const protocolservice_protocol_write_endpoint_message* msg)
{
    /* encrypt and write the message to the endpoint. */
    status retval =
        psock_write_authed_data(
            ctx->protosock, ctx->server_iv, msg->payload.data,
            msg->payload.size, &ctx->ctx->suite, &ctx->shared_secret);

    /* update the server iv. */
    ++ctx->server_iv;

    /* return the result of the write to the caller. */
    return retval;
}
