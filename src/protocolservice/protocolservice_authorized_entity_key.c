/**
 * \file protocolservice/protocolservice_authorized_entity_key.c
 *
 * \brief Get the key of a given authorized entity.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

/**
 * \brief Given an authorized entity resource handle, return its \ref rcpr_uuid
 * value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_authorized_entity_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    const protocolservice_authorized_entity* entity =
        (const protocolservice_authorized_entity*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_authorized_entity_valid(entity));

    /* return the entity id. */
    return &entity->entity_uuid;
}
