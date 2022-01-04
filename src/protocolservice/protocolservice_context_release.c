/**
 * \file protocolservice/protocolservice_context_release.c
 *
 * \brief Release the protocol service context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release the protocol service context.
 *
 * \param r             The protocol service context to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_context_release(RCPR_SYM(resource)* r)
{
    status context_release_retval = STATUS_SUCCESS;
    protocolservice_context* ctx = (protocolservice_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the context memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    return context_release_retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
