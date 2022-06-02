/**
 * \file notificationservice/notificationservice_context_resource_release.c
 *
 * \brief Release the notificationservice context resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a notificationservice resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_context_resource_release(RCPR_SYM(resource)* r)
{
    notificationservice_context* ctx = (notificationservice_context*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* clear the structure. */
    memset(ctx, 0, sizeof(*ctx));

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, ctx);
}
