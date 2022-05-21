/**
 * \file protocolservice/protocolservice_dataservice_endpoint_add.c
 *
 * \brief Add the data service endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>
#include <vpr/allocator/malloc_allocator.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Create and add the protocol service data service endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The fiber scheduler to which this endpoint fiber should
 *                      be assigned.
 * \param datasock      The socket connection to the data service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int datasock)
{
    status retval, release_retval;
    protocolservice_dataservice_endpoint_context* tmp = NULL;
    fiber* endpoint_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != addr);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));
    MODEL_ASSERT(datasock >= 0);

    /* allocate memory for the dataservice endpoint context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the dataservice endpoint context. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(
        &tmp->hdr, &protocolservice_dataservice_endpoint_context_release);

    /* set the allocator and dummy mailbox address. */
    tmp->alloc = alloc;
    tmp->addr = 0;

    /* Initialize a VPR allocator for this instance. */
    malloc_allocator_options_init(&tmp->vpr_alloc);

    /* create the dataservice endpoint fiber. */
    retval =
        fiber_create(
            &endpoint_fiber, alloc, sched, DATASERVICE_ENDPOINT_STACK_SIZE,
            tmp, &protocolservice_dataservice_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint fiber. */
    tmp->fib = endpoint_fiber;

    /* set the unexpected handler for the dataservice fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            endpoint_fiber, &protocolservice_fiber_unexpected_handler, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* create the inner psock for the dataservice fiber. */
    retval = psock_create_from_descriptor(&inner, alloc, datasock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&tmp->datasock, alloc, endpoint_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the dataservice endpoint context. */
    inner = NULL;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&tmp->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* create the mailbox address for this endpoint. */
    retval = mailbox_create(&tmp->addr, tmp->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* return this address to the caller. */
    *addr = tmp->addr;

    /* create the mailbox to context tree. */
    retval =
        rbtree_create(
            &tmp->mailbox_context_tree, alloc,
            &protocolservice_dataservice_endpoint_mailbox_context_tree_compare,
            &protocolservice_dataservice_endpoint_mailbox_context_tree_key,
            NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* create the context to mailbox tree. */
    retval =
        rbtree_create(
            &tmp->context_mailbox_tree, alloc,
            &protocolservice_dataservice_endpoint_context_mailbox_tree_compare,
            &protocolservice_dataservice_endpoint_context_mailbox_tree_key,
            NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* and the endpoint fiber to the scheduler. */
    retval = fiber_scheduler_add(sched, endpoint_fiber);
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

cleanup_endpoint_fiber:
    if (NULL != endpoint_fiber)
    {
        release_retval =
            resource_release(fiber_resource_handle(endpoint_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        endpoint_fiber = NULL;
    }

cleanup_context:
    if (NULL != tmp)
    {
        release_retval = resource_release(&tmp->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        tmp = NULL;
    }

done:
    return retval;
}
