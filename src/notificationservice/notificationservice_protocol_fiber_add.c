/**
 * \file notificationservice/notificationservice_context_resource_release.c
 *
 * \brief Add a notificationservice protocol fiber to the fiber scheduler.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Create and add a protocol fiber to the scheduler.
 *
 * \param alloc     The allocator to use for this operation.
 * \param inst      The instance for this fiber.
 * \param sock      The socket for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_add(
    RCPR_SYM(allocator)* alloc, notificationservice_instance* inst, int sock)
{
    status retval, release_retval;
    notificationservice_protocol_fiber_context* tmp = NULL;
    fiber* protocol_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_notificationservice_instance_valid(inst));
    MODEL_ASSERT(sock >= 0);

    /* allocate memory for the fiber context. */
    retval = rcpr_allocator_allocate(inst->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the context. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(
        &tmp->hdr, &notificationservice_protocol_fiber_context_release);

    /* set the allocator and instance context. */
    tmp->alloc = inst->alloc;
    tmp->inst = inst;

    /* create the return mailbox for this fiber. */
    retval =
        mailbox_create(&tmp->return_addr, inst->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* create the protocol fiber. */
    retval =
        fiber_create(
            &protocol_fiber, alloc, inst->ctx->sched,
            NOTIFICATIONSERVICE_PROTOCOL_FIBER_STACK_SIZE, tmp,
            &notificationservice_protocol_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* save the protocol fiber. */
    tmp->fib = protocol_fiber;

    /* set the unexpected handler for the protocol fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            protocol_fiber, &notificationservice_fiber_unexpected_handler,
            inst->ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_protocol_fiber;
    }

    /* create the inner psock for the protocol socket. */
    retval = psock_create_from_descriptor(&inner, alloc, sock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_protocol_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&inst->protosock, alloc, protocol_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the protocol instance. */
    inner = NULL;

    /* add the protocol fiber to the scheduler. */
    retval = fiber_scheduler_add(inst->ctx->sched, protocol_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_protocol_fiber;
    }

    /* the protocol fiber is now owned by the scheduler. */
    protocol_fiber = NULL;
    /* the context is now owned by the protocol fiber. */
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

cleanup_protocol_fiber:
    if (protocol_fiber != NULL)
    {
        release_retval =
            resource_release(fiber_resource_handle(protocol_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        protocol_fiber = NULL;
    }

cleanup_ctx:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
