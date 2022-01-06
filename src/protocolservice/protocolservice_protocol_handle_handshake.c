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

    /* TODO - fill out the rest of the handshake. */
    goto done;

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
