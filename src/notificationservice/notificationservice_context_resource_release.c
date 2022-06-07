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
RCPR_IMPORT_slist;

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
    status reclaim_retval = STATUS_SUCCESS;
    status instances_release_retval = STATUS_SUCCESS;
    notificationservice_context* ctx = (notificationservice_context*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the instances list if set. */
    if (NULL != ctx->instances)
    {
        instances_release_retval =
            resource_release(slist_resource_handle(ctx->instances));
    }

    /* clear the structure. */
    memset(ctx, 0, sizeof(*ctx));

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode return value. */
    if (STATUS_SUCCESS != instances_release_retval)
    {
        return instances_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
