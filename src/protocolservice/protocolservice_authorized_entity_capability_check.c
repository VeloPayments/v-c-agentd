/**
 * \file protocolservice/protocolservice_authorized_entity_capability_check.c
 *
 * \brief Perform a capability check using the entities capabilities set.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Check to see if the given capabilities are set.
 *
 * \param entity        The entity to check.
 * \param subject_id    The subject id.
 * \param verb_id       The verb id.
 * \param object_id     The object id.
 *
 * \returns true if the capability is set and false otherwise.
 */
bool protocolservice_authorized_entity_capability_check(
    const protocolservice_authorized_entity* entity,
    const RCPR_SYM(rcpr_uuid)* subject_id, const RCPR_SYM(rcpr_uuid)* verb_id,
    const RCPR_SYM(rcpr_uuid)* object_id)
{
    status retval;
    protocolservice_authorized_entity_capability_key key;
    protocolservice_authorized_entity_capability* cap;

    /* set up key. */
    memcpy(&key.subject_id, subject_id, sizeof(key.subject_id));
    memcpy(&key.verb_id, verb_id, sizeof(key.verb_id));
    memcpy(&key.object_id, object_id, sizeof(key.object_id));

    /* look up entity. */
    retval = rbtree_find((resource**)&cap, entity->capabilities, &key);
    if (STATUS_SUCCESS != retval)
    {
        return false;
    }

    /* the capability was found. */
    return true;
}
