/**
 * \file protocolservice/protocolservice_authorized_entity_add.c
 *
 * \brief Add an authorized entity to the protocol service context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add an authorized entity to the protocol service context.
 *
 * \param ctx                   The context to which the entity should be added.
 * \param entity_uuid           The uuid of this entity.
 * \param encryption_pubkey     The encryption public key of this entity.
 * \param signing_pubkey        The signing public key of this entity.
 *
 * \note This method transfers ownership of the public keys on success; they do
 * not have to be disposed afterward.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_add(
    protocolservice_context* ctx, const RCPR_SYM(rcpr_uuid)* entity_uuid,
    vccrypt_buffer_t* encryption_pubkey, vccrypt_buffer_t* signing_pubkey)
{
    status retval, release_retval;
    protocolservice_authorized_entity* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_context_valid(ctx));
    MODEL_ASSERT(prop_vccrypt_buffer_valid(encryption_pubkey));
    MODEL_ASSERT(prop_vccrypt_buffer_valid(signing_pubkey));

    /* allocate memory for the authorized entity. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear this structure. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &protocolservice_authorized_entity_release);

    /* set the basic values. */
    tmp->alloc = ctx->alloc;
    memcpy(&tmp->entity_uuid, entity_uuid, sizeof(*entity_uuid));

    /* move the key values. */
    vccrypt_buffer_move(&tmp->encryption_pubkey, encryption_pubkey);
    vccrypt_buffer_move(&tmp->signing_pubkey, signing_pubkey);

    /* insert this entity into the authorized entity dictionary. */
    retval = rbtree_insert(ctx->authorized_entity_dict, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_entity;
    }

    /* this entity is owned by the context now. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_entity:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }
    
done:
    return retval;
}
