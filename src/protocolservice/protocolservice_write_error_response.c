/**
 * \file protocolservice/protocolservice_write_error_response.c
 *
 * \brief Write an error response to a protocol socket
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_psock;

/**
 * \brief Write an error response to the socket.
 *
 * \param ctx           The protocol fiber context for this socket.
 * \param request_id    The id of the request that caused the error.
 * \param status_       The status code of the error.
 * \param offset        The request offset that caused the error.
 * \param encrypted     Set to true if this packet should be encrypted.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_write_error_response(
    protocolservice_protocol_fiber_context* ctx, int request_id, int status_,
    uint32_t offset, bool encrypted)
{
    status retval;
    uint32_t payload[3] = { htonl(request_id), htonl(status_), htonl(offset) };

    /* attempt to write the response payload to the socket. */
    if (encrypted)
    {
        /* encrypted write. */
        retval =
            psock_write_authed_data(
                ctx->protosock, ctx->server_iv, payload, sizeof(payload),
                &ctx->ctx->suite, &ctx->shared_secret);

        /* Update the server iv. */
        ++ctx->server_iv;

        return retval;
    }
    else
    {
        retval =
            psock_write_boxed_data(ctx->protosock, payload, sizeof(payload));
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
