/**
 * \file protocolservice/protocolservice_accept_fiber_add.c
 *
 * \brief Add the accept fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Create and add the protocol service accept fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param protosock     The socket connection to the listen service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int protosock)
{
    status retval, release_retval;
    protocolservice_accept_endpoint_context* tmp = NULL;
    fiber* accept_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(protosock >= 0);

    /* allocate memory for the accept fiber context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clean the accept fiber context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &protocolservice_accept_endpoint_context_release);

    /* set the allocator and protocol service context. */
    tmp->alloc = alloc;
    tmp->ctx = ctx;

    /* create the accept fiber. */
    retval =
        fiber_create(
            &accept_fiber, alloc, ctx->sched, ACCEPT_ENDPOINT_FIBER_STACK_SIZE,
            tmp, &protocolservice_accept_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the accept fiber. */
    tmp->fib = accept_fiber;

    /* set the unexpected handler for the accept fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            accept_fiber,
            &protocolservice_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_fiber;
    }

    /* create the inner psock for the accept socket. */
    retval = psock_create_from_descriptor(&inner, alloc, protosock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&tmp->acceptsock, alloc, accept_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the accept fiber context. */
    inner = NULL;

    /* add the accept fiber to the scheduler. */
    retval = fiber_scheduler_add(ctx->sched, accept_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_accept_fiber;
    }

    /* the accept fiber is now owned by the scheduler. */
    accept_fiber = NULL;
    /* the context is now owned by the accept fiber. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_inner_psock:
    if (inner != NULL)
    {
        release_retval = resource_release(psock_resource_handle(inner));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        inner = NULL;
    }

cleanup_accept_fiber:
    if (accept_fiber != NULL)
    {
        release_retval = resource_release(fiber_resource_handle(accept_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        accept_fiber = NULL;
    }

cleanup_context:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
