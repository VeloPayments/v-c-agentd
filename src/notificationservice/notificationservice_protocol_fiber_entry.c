/**
 * \file notificationservice/notificationservice_protocol_fiber_entry.c
 *
 * \brief Entry point for a notificationservice protocol fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_resource;

/**
 * \brief Entry point for a notificationservice protocol fiber.
 *
 * This fiber manages a notificationservice protocol instance.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_entry(void* vctx)
{
    status retval = STATUS_SUCCESS, release_retval;
    notificationservice_protocol_fiber_context* ctx =
        (notificationservice_protocol_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_notificationservice_protocol_fiber_context_valid(ctx));

    /* TODO - fill out. */
    /* TODO - shut down outbound fiber on shutdown. */
    retval = -1;
    goto cleanup_context;

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
