/**
 * \file protocolservice/protocolservice_dataservice_endpoint_context_release.c
 *
 * \brief Release the dataservice endpoint context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <string.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Release a protocolservice dataservice endpoint context resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_context_release(
    RCPR_SYM(resource)* r)
{
    status mailbox_close_retval = STATUS_SUCCESS;
    status datasock_release_retval = STATUS_SUCCESS;
    status mailbox_context_tree_release_retval = STATUS_SUCCESS;
    status context_mailbox_tree_release_retval = STATUS_SUCCESS;
    status reclaim_retval = STATUS_SUCCESS;
    protocolservice_dataservice_endpoint_context* ctx =
        (protocolservice_dataservice_endpoint_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* release the VPR allocator. */
    dispose((disposable_t*)&ctx->vpr_alloc);

    /* close the mailbox. */
    if (ctx->addr > 0)
    {
        mailbox_close_retval = mailbox_close(ctx->addr, ctx->msgdisc);
    }

    /* release the data socket. */
    if (NULL != ctx->datasock)
    {
        datasock_release_retval =
            resource_release(psock_resource_handle(ctx->datasock));
    }

    /* release the mailbox context tree. */
    if (NULL != ctx->mailbox_context_tree)
    {
        mailbox_context_tree_release_retval =
            resource_release(rbtree_resource_handle(ctx->mailbox_context_tree));
    }

    /* release the context mailbox tree. */
    if (NULL != ctx->context_mailbox_tree)
    {
        context_mailbox_tree_release_retval =
            resource_release(rbtree_resource_handle(ctx->context_mailbox_tree));
    }

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the return status code. */
    if (STATUS_SUCCESS != mailbox_close_retval)
    {
        return mailbox_close_retval;
    }
    else if (STATUS_SUCCESS != datasock_release_retval)
    {
        return datasock_release_retval;
    }
    else if (STATUS_SUCCESS != mailbox_context_tree_release_retval)
    {
        return mailbox_context_tree_release_retval;
    }
    else if (STATUS_SUCCESS != context_mailbox_tree_release_retval)
    {
        return context_mailbox_tree_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
