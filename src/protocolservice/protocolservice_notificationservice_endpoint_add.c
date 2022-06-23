/**
 * \file protocolservice/protocolservice_notificationservice_endpoint_add.c
 *
 * \brief Add notificationservice endpoint fibers.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Create and add the protocol service notification endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param ctx           The protocol service context for this operation.
 * \param notifysock    The socket connection to the notification service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, protocolservice_context* ctx,
    int notifysock)
{
    status retval, release_retval;
    protocolservice_notificationservice_fiber_context* tmp = NULL;
    fiber* notificationservice_endpoint_fiber;
    fiber* notificationservice_write_endpoint_fiber;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(notifysock >= 0);

    /* allocate memory for the notificationservice fiber context. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(
        &tmp->hdr, &protocolservice_notificationservice_fiber_context_release);

    /* save the allocator, message discipline, and context. */
    tmp->alloc = ctx->alloc;
    tmp->msgdisc = ctx->msgdisc;
    tmp->ctx = ctx;

    /* set the reference count to start at 1. */
    tmp->reference_count = 1;

    /* create the mailbox for this fiber. */
    retval = mailbox_create(&tmp->notify_addr, ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create the notificationservice fiber. */
    retval =
        fiber_create(
            &notificationservice_endpoint_fiber, ctx->alloc, ctx->sched,
            NOTIFICATION_ENDPOINT_FIBER_STACK_SIZE, tmp,
            &protocolservice_notificationservice_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the endpoint fiber. */
    tmp->fib = notificationservice_endpoint_fiber;

    /* set the unexpected handler for this fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            notificationservice_endpoint_fiber,
            &protocolservice_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_notificationservice_endpoint_fiber;
    }

    /* create the inner psock for the notificationservice socket. */
    retval = psock_create_from_descriptor(&inner, ctx->alloc, notifysock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_notificationservice_endpoint_fiber;
    }

    /* wrap this as an async psock. */
    retval =
        psock_create_wrap_async(&tmp->notifysock, ctx->alloc, tmp->fib, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner;
    }

    /* the inner psock is now owned by the fiber context. */
    inner = NULL;

    /* create the client-side request translation rbtree. */
    retval =
        rbtree_create(
            &tmp->client_xlat_map, ctx->alloc,
            &protocolservice_notificationservice_client_xlat_map_compare,
            &protocolservice_notificationservice_client_xlat_map_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner;
    }

    /* create the server-side request translation rbtree. */
    retval =
        rbtree_create(
            &tmp->server_xlat_map, ctx->alloc,
            &protocolservice_notificationservice_server_xlat_map_compare,
            &protocolservice_notificationservice_server_xlat_map_key, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner;
    }

    /* add the fiber to the scheduler. */
    retval =
        fiber_scheduler_add(ctx->sched, notificationservice_endpoint_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner;
    }

    /* the endpoint fiber is now owned by the scheduler. */
    notificationservice_endpoint_fiber = NULL;

    /* create the notificationservice write endpoint fiber. */
    retval =
        fiber_create(
            &notificationservice_write_endpoint_fiber, ctx->alloc, ctx->sched,
            NOTIFICATION_ENDPOINT_FIBER_STACK_SIZE, tmp,
            &protocolservice_notificationservice_write_endpoint_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner;
    }

    /* increment the reference count on the context to denote the write endpoint
     * fiber's ownership. */
    tmp->reference_count += 1;

    /* set the unexpected handler for this fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            notificationservice_write_endpoint_fiber,
            &protocolservice_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_notificationservice_write_endpoint_fiber;
    }

    /* add the fiber to the scheduler. */
    retval =
        fiber_scheduler_add(
            ctx->sched, notificationservice_write_endpoint_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_notificationservice_write_endpoint_fiber;
    }

    /* return the address. */
    *addr = tmp->notify_addr;

    /* the write endpoint fiber is now owned by the scheduler. */
    notificationservice_write_endpoint_fiber = NULL;

    /* the context is now owned by both fibers. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_notificationservice_write_endpoint_fiber;

cleanup_notificationservice_write_endpoint_fiber:
    if (NULL != notificationservice_write_endpoint_fiber)
    {
        release_retval =
            resource_release(
                fiber_resource_handle(
                    notificationservice_write_endpoint_fiber));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_inner:
    if (NULL != inner)
    {
        release_retval = resource_release(psock_resource_handle(inner));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_notificationservice_endpoint_fiber:
    if (NULL != notificationservice_endpoint_fiber)
    {
        release_retval =
            resource_release(
                fiber_resource_handle(notificationservice_endpoint_fiber));
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
