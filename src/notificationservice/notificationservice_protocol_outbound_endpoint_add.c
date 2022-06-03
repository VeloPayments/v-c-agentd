/**
 * \file
 * notificationservice/notificationservice_protocol_outbound_endpoint_add.c
 *
 * \brief Add the outbound endpoint fiber for a given protocol socket to the
 * fiber scheduler.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Create an outbound endpoint fiber for an instance.
 *
 * \param alloc     The allocator to use for this operation.
 * \param inst      The instance for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_add(
    RCPR_SYM(allocator)* alloc, notificationservice_instance* inst)
{
    status retval, release_retval;
    notificationservice_protocol_outbound_endpoint_fiber_context* tmp = NULL;
    fiber* endpoint_fiber = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_notificationservice_instance_valid(inst));

    /* allocate memory for the notificationservice protocol endpoint context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(
        &tmp->hdr,
        &notificationservice_protocol_outbound_endpoint_fiber_context_release);

    /* set the allocator and instance. */
    tmp->alloc = alloc;
    tmp->inst = inst;

    /* create the endpoint fiber. */
    retval =
        fiber_create(
            &endpoint_fiber, alloc, inst->ctx->sched,
            NOTIFICATIONSERVICE_PROTOCOL_ENDPOINT_FIBER_STACK_SIZE, tmp,
            &notificationservice_protocol_outbound_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint fiber. */
    tmp->fib = endpoint_fiber;

    /* set the unexpected handler. */
    retval =
        fiber_unexpected_event_callback_add(
            endpoint_fiber, &notificationservice_fiber_unexpected_handler,
            NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* create the mailbox address for this endpoint. */
    retval = mailbox_create(&inst->outbound_addr, inst->ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* add the endpoint fiber to the scheduler. */
    retval = fiber_scheduler_add(inst->ctx->sched, endpoint_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* the endpoint fiber is now owned by the scheduler. */
    endpoint_fiber = NULL;
    /* the context is now owned by the endpoint fiber. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_endpoint_fiber:
    if (NULL != endpoint_fiber)
    {
        release_retval =
            resource_release(fiber_resource_handle(endpoint_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_context:
    if (NULL != tmp)
    {
        release_retval = resource_release(&tmp->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

done:
    return retval;
}
