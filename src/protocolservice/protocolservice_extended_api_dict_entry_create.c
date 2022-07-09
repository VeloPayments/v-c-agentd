/**
 * \file protocolservice/protocolservice_extended_api_dict_entry_create.c
 *
 * \brief Create an extended api dictionary entry.
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
 * \brief Create an extended API dictionary entry.
 *
 * \param entry         Pointer to receive the entry on success.
 * \param alloc         The allocator to use for this operation.
 * \param entity_id     The entity id for this entry.
 * \param ctx           A weak reference to the protocolservice protocol fiber
 *                      context for this entry.
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_dict_entry_create(
    protocolservice_extended_api_dict_entry** entry, RCPR_SYM(allocator)* alloc,
    const RCPR_SYM(rcpr_uuid)* entity_id,
    protocolservice_protocol_fiber_context* ctx)
{
    status retval;
    protocolservice_extended_api_dict_entry* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != entry);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* allocate memory for this entry. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr, &protocolservice_extended_api_dict_entry_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    memcpy(&tmp->entity_id, entity_id, sizeof(*entity_id));
    tmp->ctx = ctx;

    /* return this instance. */
    *entry = tmp;

    /* success. */
    return STATUS_SUCCESS;
}
