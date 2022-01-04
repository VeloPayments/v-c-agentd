/**
 * \file protocolservice/protocolservice_protocol_fiber_add.c
 *
 * \brief Add a protocol fiber.
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
 * \brief Create and add a protocol service protocol fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param sock          The client socket for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int sock)
{
    status retval, release_retval;
    protocolservice_protocol_fiber_context* tmp = NULL;
    fiber* protocol_fiber = NULL;
    psock* inner = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(sock >= 0);

    /* allocate memory for the protocol fiber context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the protocol fiber context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &protocolservice_protocol_fiber_context_release);

    /* set the allocator and protocol service context. */
    tmp->alloc = alloc;
    tmp->ctx = ctx;

    /* create the client key nonce buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &ctx->suite, &tmp->client_key_nonce);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create the client challenge nonce buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &ctx->suite, &tmp->client_challenge_nonce);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* create the protocol fiber. */
    retval =
        fiber_create(
            &protocol_fiber, alloc, ctx->sched, PROTOCOL_FIBER_STACK_SIZE,
            tmp, &protocolservice_protocol_fiber_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* save the protocol fiber. */
    tmp->fib = protocol_fiber;

    /* set the unexpected handler for the protocol fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            protocol_fiber, &protocolservice_fiber_unexpected_handler, ctx);
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
        psock_create_wrap_async(&tmp->protosock, alloc, protocol_fiber, inner);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_inner_psock;
    }

    /* the inner psock is now owned by the protocol fiber context. */
    inner = NULL;

    /* add the protocol fiber to the scheduler. */
    retval = fiber_scheduler_add(ctx->sched, protocol_fiber);
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
