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

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
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
    status dataservice_context_release_retval = STATUS_SUCCESS;
    status protosock_release_retval = STATUS_SUCCESS;
    status mailbox_close_retval = STATUS_SUCCESS;
    status fiber_mailbox_close_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    status extended_api_disable_retval = STATUS_SUCCESS;
    protocolservice_protocol_fiber_context* ctx =
        (protocolservice_protocol_fiber_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* decrement the reference count. */
    --ctx->reference_count;

    /* if there are still references to this context, don't release it yet. */
    if (ctx->reference_count > 0)
    {
        return STATUS_SUCCESS;
    }

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* disable the extended API if enabled. */
    if (ctx->extended_api_enabled)
    {
        extended_api_disable_retval =
            protocolservice_protocol_unroute_extended_api_for_entity(ctx);
    }

    /* close the dataservice context. */
    if (ctx->dataservice_context_opened)
    {
        dataservice_context_release_retval =
            protocolservice_protocol_close_data_service_context(ctx);
    }

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

    /* dispose the server key nonce. */
    if (NULL != ctx->server_key_nonce.data)
    {
        dispose((disposable_t*)&ctx->server_key_nonce);
    }

    /* dispose the server challenge nonce. */
    if (NULL != ctx->server_challenge_nonce.data)
    {
        dispose((disposable_t*)&ctx->server_challenge_nonce);
    }

    /* dispose of the shared secret. */
    if (NULL != ctx->shared_secret.data)
    {
        dispose((disposable_t*)&ctx->shared_secret);
    }

    /* close the mailbox associated with this fiber. */
    if (ctx->return_addr > 0)
    {
        mailbox_close_retval =
            mailbox_close(ctx->return_addr, ctx->ctx->msgdisc);
    }

    /* close the fiber mailbox associated with this fiber. */
    if (ctx->fiber_addr > 0)
    {
        fiber_mailbox_close_retval =
            mailbox_close(ctx->fiber_addr, ctx->ctx->msgdisc);
    }

    /* reclaim memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the appropriate response code. */
    if (STATUS_SUCCESS != extended_api_disable_retval)
    {
        return extended_api_disable_retval;
    }
    else if (STATUS_SUCCESS != dataservice_context_release_retval)
    {
        return dataservice_context_release_retval;
    }
    else if (STATUS_SUCCESS != protosock_release_retval)
    {
        return protosock_release_retval;
    }
    else if (STATUS_SUCCESS != mailbox_close_retval)
    {
        return mailbox_close_retval;
    }
    else if (STATUS_SUCCESS != fiber_mailbox_close_retval)
    {
        return fiber_mailbox_close_retval;
    }
    else
    {
        return context_release_retval;
    }
}
