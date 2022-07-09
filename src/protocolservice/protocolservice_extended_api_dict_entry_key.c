/**
 * \file protocolservice/protocolservice_extended_api_dict_entry_key.c
 *
 * \brief Get the key of a given extended api dictionary entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

/**
 * \brief Given a \ref protocolservice_extended_api_dict_entry resource handle,
 * return its \ref rcpr_uuid key.
 *
 * \param context       Unused.
 * \param r             The resource handle of an extended api dict entry.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_extended_api_dict_entry_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    const protocolservice_extended_api_dict_entry* entry =
        (const protocolservice_extended_api_dict_entry*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_extended_api_dict_entry_valid(entry));

    /* return the entity id. */
    return &entry->entity_id;
}
