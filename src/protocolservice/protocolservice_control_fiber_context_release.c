/**
 * \file protocolservice/protocolservice_control_fiber_context_release.c
 *
 * \brief Release the control fiber context;
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release the protocol service control fiber context.
 *
 * \param r             The protocol service control fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_context_release(RCPR_SYM(resource)* r)
{
    status controlsock_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    protocolservice_control_fiber_context* ctx =
        (protocolservice_control_fiber_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the control socket. */
    if (NULL != ctx->controlsock)
    {
        controlsock_release_retval =
            resource_release(psock_resource_handle(ctx->controlsock));
    }

    /* reclaim memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the appropriate response code. */
    if (STATUS_SUCCESS != controlsock_release_retval)
    {
        return controlsock_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}
