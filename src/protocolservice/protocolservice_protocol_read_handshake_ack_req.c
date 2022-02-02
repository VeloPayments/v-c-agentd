/**
 * \file protocolservice/protocolservice_protocol_read_handshake_ack_req.c
 *
 * \brief Read the handshake ack request.
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
 * \brief Read the handshake ack request from the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_handshake_ack_req(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    void* req = NULL;
    uint32_t size;

    /* attempt to read the ack packet. */
    retval =
        psock_read_authed_data(
            ctx->protosock, ctx->alloc, ctx->client_iv, &req, &size,
            &ctx->ctx->suite, &ctx->shared_secret);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        goto write_error_response;
    }

    /* if we've read a message, increment the client IV. */
    ++ctx->client_iv;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

write_error_response:
    release_retval =
        protocolservice_write_error_response(ctx, 0U, retval, 0U, true);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    if (NULL != req)
    {
        memset(req, 0, size);
        free(req);
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
