/**
 * \file protocolservice/protocolservice_protocol_write_endpoint_add.c
 *
 * \brief Create and add a protocol write endpoint to the fiber scheduler.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_fiber;
RCPR_IMPORT_resource;

/**
 * \brief Create and add a protocol write endpoint instance to the fiber
 * manager.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_add(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    fiber* endpoint_fiber = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* increment the number of references on the context. */
    /* after this point, we must "release" the context on failure. */
    ++ctx->reference_count;

    /* create the endpoint fiber. */
    retval =
        fiber_create(
            &endpoint_fiber, ctx->alloc, ctx->ctx->sched,
            PROTOCOL_FIBER_STACK_SIZE, ctx,
            &protocolservice_protocol_write_endpoint_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* set the unexpected handler for this fiber. */
    retval =
        fiber_unexpected_event_callback_add(
            endpoint_fiber, &protocolservice_fiber_unexpected_handler, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* add the endpoint fiber to the scheduler. */
    retval = fiber_scheduler_add(ctx->ctx->sched, endpoint_fiber);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_endpoint_fiber;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_endpoint_fiber:
    release_retval = resource_release(fiber_resource_handle(endpoint_fiber));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
