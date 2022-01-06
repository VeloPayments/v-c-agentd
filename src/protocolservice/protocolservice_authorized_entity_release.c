/**
 * \file protocolservice/protocolservice_authorized_entity_release.c
 *
 * \brief Release an authorized entity resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);

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

    /* clear the entity struct. */
    memset(entity, 0, sizeof(*entity));

    /* reclaim the struct. */
    return
        rcpr_allocator_reclaim(alloc, entity);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
