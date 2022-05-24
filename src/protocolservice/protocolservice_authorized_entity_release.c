/**
 * \file protocolservice/protocolservice_authorized_entity_release.c
 *
 * \brief Release an authorized entity resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Release an authorized entity resource.
 *
 * \param r             The authorized entity resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_release(RCPR_SYM(resource)* r)
{
    status reclaim_retval = STATUS_SUCCESS;
    status capabilities_release_retval = STATUS_SUCCESS;
    protocolservice_authorized_entity* entity =
        (protocolservice_authorized_entity*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_authorized_entity_valid(entity));

    /* cache the allocator. */
    rcpr_allocator* alloc = entity->alloc;

    /* dispose the encryption pubkey. */
    dispose((disposable_t*)&entity->encryption_pubkey);

    /* dispose the signing pubkey. */
    dispose((disposable_t*)&entity->signing_pubkey);

    /* if the capabilities tree is initialized, release it. */
    if (NULL != entity->capabilities)
    {
        capabilities_release_retval =
            resource_release(rbtree_resource_handle(entity->capabilities));
    }

    /* clear the entity struct. */
    memset(entity, 0, sizeof(*entity));

    /* reclaim the struct. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, entity);

    /* decode response code. */
    if (STATUS_SUCCESS != capabilities_release_retval)
    {
        return capabilities_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
