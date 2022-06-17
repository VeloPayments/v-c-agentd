/**
 * \file
 * protocolservice/protocolservice_notificationservice_endpoint_fiber_entry.c
 *
 * \brief Entry point for the notificationservice endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_resource;

/**
 * \brief Entry point for the protocol service notificationservice endpoint
 * fiber.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_fiber_entry(void* vctx)
{
    status retval, release_retval;
    protocolservice_notificationservice_fiber_context* ctx =
        (protocolservice_notificationservice_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));

    /* TODO - fill out. */
    retval = -1;
    goto cleanup_ctx;

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
