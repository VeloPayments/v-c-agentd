/**
 * \file
 * notificationservice/notificationservice_protocol_outbound_endpoint_fiber_context_release.c
 *
 * \brief Release a notificationservice protocol outbound fiber context
 * resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release a notificationservice protocol outbound endpoint fiber context
 * resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_fiber_context_release(
    RCPR_SYM(resource)* r)
{
    notificationservice_protocol_outbound_endpoint_fiber_context* ctx =
        (notificationservice_protocol_outbound_endpoint_fiber_context*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* clear memory. */
    memset(ctx, 0, sizeof(*ctx));

    /* reclaim memory. */
    return rcpr_allocator_reclaim(alloc, ctx);
}
