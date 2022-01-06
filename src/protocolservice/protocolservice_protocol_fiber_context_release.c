/**
 * \file protocolservice/protocolservice_protocol_fiber_context_release.c
 *
 * \brief Release a protocol fiber context resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release a protocol service protocol fiber context.
 *
 * \param r             The protocol service  protocol fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_context_release(RCPR_SYM(resource)* r)
{
    status protosock_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    protocolservice_protocol_fiber_context* ctx =
        (protocolservice_protocol_fiber_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the protocol socket. */
    if (NULL != ctx->protosock)
    {
        protosock_release_retval =
            resource_release(psock_resource_handle(ctx->protosock));
    }

    /* dispose the client key nonce. */
    if (NULL != ctx->client_key_nonce.data)
    {
        dispose((disposable_t*)&ctx->client_key_nonce);
    }

    /* dispose the client challenge nonce. */
    if (NULL != ctx->client_challenge_nonce.data)
    {
        dispose((disposable_t*)&ctx->client_challenge_nonce);
    }

    /* reclaim memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the appropriate response code. */
    if (STATUS_SUCCESS != protosock_release_retval)
    {
        return protosock_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */