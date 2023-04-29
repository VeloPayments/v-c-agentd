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

    /* request a data service context for this connection. */
    retval = protocolservice_protocol_request_data_service_context(ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* spawn a protocol write endpoint for this connection. */
    retval = protocolservice_protocol_write_endpoint_add(ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* decode-and-dispatch loop. */
    while (!ctx->ctx->quiesce && !ctx->shutdown && !ctx->req_shutdown)
    {
        retval = protocolservice_protocol_read_decode_and_dispatch_packet(ctx);
        if (STATUS_SUCCESS != retval)
        {
            goto shutdown_write_endpoint;
        }
    }

    retval = STATUS_SUCCESS;
    goto shutdown_write_endpoint;

shutdown_write_endpoint:
    release_retval = protocolservice_protocol_shutdown_write_endpoint(ctx);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
