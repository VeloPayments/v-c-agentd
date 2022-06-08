/**
 * \file notificationservice/notificationservice_context_add_instance.c
 *
 * \brief Add an instance to the context and take ownership of it.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_slist;

/**
 * \brief Create a notificationservice instance to the context.
 *
 * \note On success, the context takes ownership of this instance.
 *
 * \param ctx       The root context.
 * \param inst      The instance to add.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_context_add_instance(
    notificationservice_context* ctx, notificationservice_instance* inst)
{
    return
        slist_append_tail(ctx->instances, &inst->hdr);
}
