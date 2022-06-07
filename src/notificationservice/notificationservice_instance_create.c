/**
 * \file notificationservice/notificationservice_instance_create.c
 *
 * \brief Create the notificationservice instance.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_resource;

/**
 * \brief Create a notificationservice instance.
 *
 * \param inst      Pointer to receive the instance on success.
 * \param ctx       The root context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_instance_create(
    notificationservice_instance** inst, notificationservice_context* ctx)
{
    status retval;
    notificationservice_instance* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(prop_notificationservice_context_valid(ctx));

    /* allocate memory for the context. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the struct. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize the resource. */
    resource_init(&tmp->hdr, &notificationservice_instance_resource_release);

    /* set values. */
    tmp->alloc = ctx->alloc;
    tmp->ctx = ctx;

    /* set the bitcaps to max permissible. */
    BITCAP_INIT_TRUE(tmp->caps);

    /* success. */
    retval = STATUS_SUCCESS;
    *inst = tmp;
    goto done;

done:
    return retval;
}
