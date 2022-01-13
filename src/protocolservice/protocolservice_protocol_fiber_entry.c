/**
 * \file protocolservice/protocolservice_protocol_fiber_entry.c
 *
 * \brief Entry point for a protocol service protocol fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for a protocol service protocol fiber.
 *
 * This fiber manages the protocol for a single client connection.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_entry(void* vctx)
{
    status retval = STATUS_SUCCESS, release_retval;
    protocolservice_protocol_fiber_context* ctx =
        (protocolservice_protocol_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* handle the handshake. */
    retval = protocolservice_protocol_handle_handshake(ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* TODO - spawn protocol write endpoint for this connection. */
    /* TODO - request data service context for this connection. */
    /* TODO - add decode-and-dispatch loop. */
    /* TODO - extend decode-and-dispatch with sentinel service registry. */
    retval = STATUS_SUCCESS;
    goto cleanup_context;

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
