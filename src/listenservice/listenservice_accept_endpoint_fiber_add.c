/**
 * \file listenservice/listenservice_accept_endpoint_fiber_add.c
 *
 * \brief Create and add the accept endpoint fiber.
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
 * \brief Create and add the listen service accept endpoint fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this endpoint fiber should be
 *                      assigned.
 * \param endpoint_addr Pointer to receive the endpoint's mailbox address.
 * \param acceptsock    The socket descriptor to send accepted sockets.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched,
    RCPR_SYM(mailbox_address)* endpoint_addr, int acceptsock)
{
    status retval, release_retval;
    listenservice_accept_endpoint_context* ctx = NULL;
    fiber* accept_endpoint = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));
    MODEL_ASSERT(acceptsock >= 0);

    /* allocate memory for the accept endpoint fiber context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&ctx, sizeof(*ctx));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the fiber context. */
    memset(ctx, 0, sizeof(*ctx));

    /* set the resource release method. */
    resource_init(&ctx->hdr, &listenservice_accept_endpoint_context_release);

    /* set the allocator and scheduler. */
    ctx->alloc = alloc;
    ctx->sched = sched;
    ctx->accept_socket = NULL;
    ctx->endpoint_addr = (uint64_t)-1;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&ctx->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create the mailbox address for this endpoint. */
    retval = mailbox_create(&ctx->endpoint_addr, ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint address. */
    *endpoint_addr = ctx->endpoint_addr;

    /* create the endpoint fiber. */
    retval =
        fiber_create(
            &accept_endpoint, alloc, sched, ACCEPT_ENDPOINT_STACK_SIZE, ctx,
            &listenservice_accept_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint fiber. */
    ctx->fib = accept_endpoint;

    /* set the unexpected handler for the endpoint fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            accept_endpoint,
            &listenservice_accept_endpoint_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_endpoint;
    }

    /* create the inner psock for the accept socket. */
    retval = psock_create_from_descriptor(&inner, alloc, acceptsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_endpoint;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(
            &ctx->accept_socket, alloc, accept_endpoint, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the accept endpoint context. */
    inner = NULL;

    /* add the accept endpoint to the scheduler. */
    retval = fiber_scheduler_add(sched, accept_endpoint);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_endpoint;
    }

    /* the accept endpoint is now owned by the scheduler. */
    accept_endpoint = NULL;
    /* the context is now owned by the accept endpoint. */
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

cleanup_accept_endpoint:
    if (NULL != accept_endpoint)
    {
        release_retval =
            resource_release(fiber_resource_handle(accept_endpoint));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        accept_endpoint = NULL;
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
