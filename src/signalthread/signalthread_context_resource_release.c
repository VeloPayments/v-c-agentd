/**
 * \file signalthread/signalthread_context_resource_release.c
 *
 * \brief Release a \ref signalthread_context.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/socket_utilities.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "signalthread_internal.h"

RCPR_IMPORT_allocator;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release a \ref signalthread_context resource.
 *
 * \param r             The signalthread_context resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_context_resource_release(resource* r)
{
    status release_psock_retval = STATUS_SUCCESS;
    status release_retval = STATUS_SUCCESS;

    signalthread_context* ctx = (signalthread_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_signalthread_context_valid(ctx));

    /* cache the allocator. */
    allocator* alloc = ctx->alloc;

    /* release the psock resource if valid. */
    if (NULL != ctx->signal_sock)
    {
        release_psock_retval =
            resource_release(psock_resource_handle(ctx->signal_sock));
    }

    /* release signal thread context memory. */
    release_retval = allocator_reclaim(alloc, ctx);

    /* return the appropriate status code. */
    if (STATUS_SUCCESS != release_psock_retval)
    {
        return release_psock_retval;
    }
    else
    {
        return release_retval;
    }
}
