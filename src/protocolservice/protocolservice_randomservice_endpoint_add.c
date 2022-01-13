/**
 * \file protocolservice/protocolservice_randomservice_endpoint_add.c
 *
 * \brief Add the random service endpoint fiber.
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
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Create and add the protocol service random endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The fiber scheduler to which this endpoint fiber should
 *                      be assigned.
 * \param randomsock    The socket connection to the random service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_randomservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int randomsock)
{
    status retval, release_retval;
    protocolservice_random_endpoint_context* tmp = NULL;
    fiber* random_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != addr);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));
    MODEL_ASSERT(randomsock >= 0);

    /* allocate memory for the random endpoint context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the random endpoint context. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &protocolservice_random_endpoint_context_release);

    /* set the allocator and dummy mailbox address. */
    tmp->alloc = alloc;
    tmp->addr = 0;

    /* create the random endpoint fiber. */
    retval =
        fiber_create(
            &random_fiber, alloc, sched, RANDOM_ENDPOINT_STACK_SIZE,
            tmp, &protocolservice_random_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint fiber. */
    tmp->fib = random_fiber;

    /* set the unexpected handler for the endpoint fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            random_fiber, &protocolservice_fiber_unexpected_handler, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_random_fiber;
    }

    /* create the inner psock for the random socket. */
    retval = psock_create_from_descriptor(&inner, alloc, randomsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_random_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&tmp->randomsock, alloc, random_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the random endpoint context. */
    inner = NULL;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&tmp->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_random_fiber;
    }

    /* create the mailbox address for this endpoint. */
    retval = mailbox_create(&tmp->addr, tmp->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_random_fiber;
    }

    /* return this address to the caller. */
    *addr = tmp->addr;

    /* add the random fiber to the scheduler. */
    retval = fiber_scheduler_add(sched, random_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_random_fiber;
    }

    /* the random fiber is now owned by the scheduler. */
    random_fiber = NULL;
    /* the context is now owned by the random fiber. */
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

cleanup_random_fiber:
    if (NULL != random_fiber)
    {
        release_retval = resource_release(fiber_resource_handle(random_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        random_fiber = NULL;
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
