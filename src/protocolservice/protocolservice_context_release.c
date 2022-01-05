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
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

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
    status authorized_entity_dict_release_retval = STATUS_SUCCESS;
    status context_release_retval = STATUS_SUCCESS;
    protocolservice_context* ctx = (protocolservice_context*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* dispose the crypto suite if initialized. */
    if (NULL != ctx->suite.vccrypt_suite_hash_alg_init)
    {
        dispose((disposable_t*)&ctx->suite);
    }

    /* dispose the VPR allocator if initialized. */
    if (NULL != ctx->vpr_alloc.allocator_allocate)
    {
        dispose((disposable_t*)&ctx->vpr_alloc);
    }

    /* release the authorized entity dictionary if initialized. */
    if (NULL != ctx->authorized_entity_dict)
    {
        authorized_entity_dict_release_retval =
            resource_release(
                rbtree_resource_handle(ctx->authorized_entity_dict));
    }

    /* release the context memory. */
    context_release_retval = rcpr_allocator_reclaim(alloc, ctx);

    /* decode the appropriate return value. */
    if (STATUS_SUCCESS != authorized_entity_dict_release_retval)
    {
        return authorized_entity_dict_release_retval;
    }
    else
    {
        return context_release_retval;
    }
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
