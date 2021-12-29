/**
 * \file listenservice/listenservice_listen_fiber_add.c
 *
 * \brief Create and add a listen fiber to the fiber scheduler.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include "listenservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Create and add a listen fiber for the listen service.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this listen fiber should be
 *                      assigned.
 * \param endpoint_addr The endpoint's mailbox address.
 * \param desc          The descriptor on which this fiber listens.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched,
    RCPR_SYM(mailbox_address) endpoint_addr, int desc)
{
    status retval, release_retval;
    listenservice_listen_fiber_context* ctx = NULL;
    fiber* listen_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));
    MODEL_ASSERT(prop_mailbox_address_valid(endpoint_addr));
    MODEL_ASSERT(desc >= 0);

    /* allocate memory for the listen fiber context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&ctx, sizeof(*ctx));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the listen fiber context. */
    memset(ctx, 0, sizeof(*ctx));

    /* set the resource release method. */
    resource_init(&ctx->hdr, &listenservice_listen_fiber_context_release);

    /* set the allocator, scheduler, and endpoint mailbox address. */
    ctx->alloc = alloc;
    ctx->sched = sched;
    ctx->endpoint_addr = endpoint_addr;
    ctx->return_addr = (uint64_t)-1;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&ctx->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create a return address for this listen fiber. */
    retval = mailbox_create(&ctx->return_addr, ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create the listen fiber. */
    retval =
        fiber_create(
            &listen_fiber, alloc, sched, LISTEN_FIBER_STACK_SIZE, ctx,
            &listenservice_listen_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the listen fiber. */
    ctx->fib = listen_fiber;

    /* set the unexpected handler for the listen fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            listen_fiber, &listenservice_listen_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_listen_fiber;
    }

    /* create the inner psock for the listen address. */
    retval = psock_create_from_descriptor(&inner, alloc, desc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_listen_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(
            &ctx->listen_socket, alloc, listen_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the listen fiber context. */
    inner = NULL;

    /* add the listen fiber to the scheduler. */
    retval = fiber_scheduler_add(sched, listen_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_listen_fiber;
    }

    /* the listen fiber is now owned by the scheduler. */
    listen_fiber = NULL;
    /* the context is now owned by the listen fiber. */
    ctx = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_inner_psock:
    if (NULL != inner)
    {
        release_retval = resource_release(psock_resource_handle(inner));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        inner = NULL;
    }

cleanup_listen_fiber:
    if (NULL != listen_fiber)
    {
        release_retval = resource_release(fiber_resource_handle(listen_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        listen_fiber = NULL;
    }

cleanup_context:
    if (NULL != ctx)
    {
        release_retval = resource_release(&ctx->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        ctx = NULL;
    }

done:
    return retval;
}
