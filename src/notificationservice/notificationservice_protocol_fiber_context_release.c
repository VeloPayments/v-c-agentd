/**
 * \file
 * notificationservice/notificationservice_protocol_fiber_context_release.c
 *
 * \brief Release a notificationservice protocol fiber context resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release a notificationservice protocol fiber context resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_context_release(
    RCPR_SYM(resource)* r)
{
    status reclaim_retval = STATUS_SUCCESS;
    status mailbox_close_retval = STATUS_SUCCESS;
    notificationservice_protocol_fiber_context* ctx =
        (notificationservice_protocol_fiber_context*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* if a mailbox has been created, close it. */
    if (ctx->return_addr > 0)
    {
        mailbox_close_retval =
            mailbox_close(ctx->return_addr, ctx->inst->ctx->msgdisc);
    }

    /* clear memory. */
    memset(ctx, 0, sizeof(*ctx));

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode return value. */
    if (STATUS_SUCCESS != mailbox_close_retval)
    {
        return mailbox_close_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
