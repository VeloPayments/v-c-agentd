/**
 * \file protocolservice/protocolservice_authorized_entity_capability_create.c
 *
 * \brief Create an authorized entity capability.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a protocolservice_authorized_entity_capability instance.
 *
 * \param cap           Pointer to receive this instance.
 * \param alloc         The allocator to use for this operation.
 * \param subject_id    The subject UUID.
 * \param verb_id       The verb UUID.
 * \param object_id     The object UUID.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_capability_create(
    protocolservice_authorized_entity_capability** cap,
    RCPR_SYM(allocator)* alloc, const RCPR_SYM(rcpr_uuid)* subject_id,
    const RCPR_SYM(rcpr_uuid)* verb_id, const RCPR_SYM(rcpr_uuid)* object_id)
{
    status retval;
    protocolservice_authorized_entity_capability* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != cap);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != subject_id);
    MODEL_ASSERT(NULL != verb_id);
    MODEL_ASSERT(NULL != object_id);

    /* allocate memory for this capability. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr,
        &protocolservice_authorized_entity_capability_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    memcpy(&tmp->key.subject_id, subject_id, sizeof(tmp->key.subject_id));
    memcpy(&tmp->key.verb_id, verb_id, sizeof(tmp->key.verb_id));
    memcpy(&tmp->key.object_id, object_id, sizeof(tmp->key.object_id));

    /* return this instance. */
    *cap = tmp;

    /* success. */
    return STATUS_SUCCESS;
}
