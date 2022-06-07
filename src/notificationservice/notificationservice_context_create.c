/**
 * \file notificationservice/notificationservice_context_create.c
 *
 * \brief Create the notificationservice context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

/**
 * \brief Create a notificationservice context.
 *
 * \param ctx       Pointer to receive the context on success.
 * \param alloc     The allocator to use for this operation.
 * \param sched     The scheduler to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_context_create(
    notificationservice_context** ctx, rcpr_allocator* alloc,
    fiber_scheduler* sched)
{
    status retval, release_retval;
    notificationservice_context* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(rpcr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));

    /* allocate memory for the context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the struct. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize the resource. */
    resource_init(&tmp->hdr, &notificationservice_context_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->terminate = false;
    tmp->sched = sched;

    /* initialize the instances list. */
    retval = slist_create(&tmp->instances, alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    *ctx = tmp;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
