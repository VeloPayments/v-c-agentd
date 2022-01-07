/**
 * \file protocolservice/protocolservice_authorized_entity_lookup.c
 *
 * \brief Look up an authorized entity.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Look up an authorized entity by entity id.
 *
 * \param entity                Pointer to the authorized entity pointer to
 *                              receive this entity on success.
 * \param ctx                   The context from which to look up this entity.
 * \param entity_uuid           The uuid of the entity to look up.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_lookup(
    const protocolservice_authorized_entity** entity,
    protocolservice_context* ctx, const RCPR_SYM(rcpr_uuid)* entity_uuid)
{
    status retval;
    resource* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(NULL != entity);
    MODEL_ASSERT(NULL != entity_uuid);

    /* attempt to find the entity in the authorized entities dict. */
    retval =
        rbtree_find(
            &tmp, ctx->authorized_entity_dict, (const void*)entity_uuid);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* return the value on success. */
    *entity = (const protocolservice_authorized_entity*)tmp;
    MODEL_ASSERT(prop_protocolservice_authorized_entity_valid(*entity));
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
