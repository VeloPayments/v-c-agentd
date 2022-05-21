/**
 * \file protocolservice/protocolservice_random_endpoint_context_release.c
 *
 * \brief Resource release method for the random endpoint context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Release the protocol service random endpoint context.
 *
 * \param r             The protocol service random endpoint context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_endpoint_context_release(RCPR_SYM(resource)* r)
{
    status mailbox_close_retval = STATUS_SUCCESS;
    status randomsock_release_retval = STATUS_SUCCESS;
    status reclaim_retval = STATUS_SUCCESS;

    protocolservice_random_endpoint_context* ctx =
        (protocolservice_random_endpoint_context*)r;

    /* parameter sanity checking. */
    MODEL_ASSERT(prop_protocolservice_random_endpoint_context_valid(ctx));

    /* cache allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* close the random endpoint mailbox if it exists. */
    if (ctx->addr > 0)
    {
        mailbox_close_retval = mailbox_close(ctx->addr, ctx->msgdisc);
    }

    /* release the random socket resource, if created. */
    if (NULL != ctx->randomsock)
    {
        randomsock_release_retval =
            resource_release(psock_resource_handle(ctx->randomsock));
    }

    /* reclaim the memory for this context. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the right error response. */
    if (STATUS_SUCCESS != mailbox_close_retval)
    {
        return mailbox_close_retval;
    }
    else if (STATUS_SUCCESS != randomsock_release_retval)
    {
        return randomsock_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
