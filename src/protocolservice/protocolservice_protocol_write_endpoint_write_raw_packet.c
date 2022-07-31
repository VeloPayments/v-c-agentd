/**
 * \file
 * protocolservice/protocolservice_protocol_write_endpoint_write_raw_packet.c
 *
 * \brief Write a raw packet to the peer.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <vcblockchain/psock.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_psock;

/**
 * \brief Write a packet to the peer.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The raw message buffer to write.
 * \param size          The size of the message buffer to write.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_write_raw_packet(
    protocolservice_protocol_fiber_context* ctx, const void* msg, size_t size)
{
    status retval;

    /* write the raw packet to the peer as an authed packet. */
    retval = 
        psock_write_authed_data(
            ctx->protosock, ctx->server_iv, msg, size, &ctx->ctx->suite,
            &ctx->shared_secret);

    /* update the server iv. */
    ++ctx->server_iv;

    /* return the status from the write. */
    return retval;
}
