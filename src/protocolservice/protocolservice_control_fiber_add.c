/**
 * \file protocolservice/protocolservice_control_fiber_add.c
 *
 * \brief Add the control fiber.
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
 * \brief Create and add the protocol service control fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param controlsock   The socket connection to the supervisor.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int controlsock)
{
    status retval, release_retval;
    protocolservice_control_fiber_context* tmp = NULL;
    fiber* control_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(sock >= 0);

    /* allocate memory for the control fiber context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the control fiber context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &protocolservice_control_fiber_context_release);

    /* set the allocator and protocol service context. */
    tmp->alloc = alloc;
    tmp->ctx = ctx;

    /* create the control fiber. */
    retval =
        fiber_create(
            &control_fiber, alloc, ctx->sched, CONTROL_FIBER_STACK_SIZE,
            tmp, &protocolservice_control_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the control fiber. */
    tmp->fib = control_fiber;

    /* set the unexpected handler for the control fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            control_fiber, &protocolservice_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_control_fiber;
    }

    /* create the inner psock for the control socket. */
    retval = psock_create_from_descriptor(&inner, alloc, controlsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_control_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&tmp->controlsock, alloc, control_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the control fiber context. */
    inner = NULL;

    /* add the control fiber to the scheduler. */
    retval = fiber_scheduler_add(ctx->sched, control_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_control_fiber;
    }

    /* the control fiber is now owned by the scheduler. */
    control_fiber = NULL;
    /* the context is now owned by the control fiber. */
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

cleanup_control_fiber:
    if (control_fiber != NULL)
    {
        release_retval = resource_release(fiber_resource_handle(control_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        control_fiber = NULL;
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
