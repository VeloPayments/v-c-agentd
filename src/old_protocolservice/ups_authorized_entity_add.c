/**
 * \file protocolservice/ups_authorized_entity_add.c
 *
 * \brief Add an authorized entity to this protocol service instance.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls. */
static void ups_authorized_entity_dispose(void* disp);

/**
 * \brief Add an authorized entity to the protocol service.
 *
 * \param instance      The instance to which the authorized entity is added.
 * \param entity_id     The UUID of this entity.
 * \param enckey        The raw bytes of the encryption public key.
 * \param signkey       The raw bytes of the signing public key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 */
int ups_authorized_entity_add(
    unauthorized_protocol_service_instance_t* instance,
    const uint8_t* entity_id, const uint8_t* enckey, const uint8_t* signkey)
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != enckey);
    MODEL_ASSERT(NULL != signkey);

    /* allocate memory for a new entity instance. */
    ups_authorized_entity_t* entity =
        (ups_authorized_entity_t*)malloc(sizeof(ups_authorized_entity_t));
    if (NULL == entity)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set the dispose method for this entity. */
    entity->hdr.dispose = &ups_authorized_entity_dispose;

    /* copy the uuid to this instance. */
    memcpy(entity->id, entity_id, 16);

    /* initialize the encryption public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            &instance->suite, &entity->enc_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity;
    }

    /* copy the encryption public key to this buffer. */
    memcpy(entity->enc_pubkey.data, enckey, entity->enc_pubkey.size);

    /* initialize the signing public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_public_key(
            &instance->suite, &entity->sign_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubkey;
    }

    /* copy the signing public key to this buffer. */
    memcpy(entity->sign_pubkey.data, signkey, entity->sign_pubkey.size);

    /* append this item to the instance head. */
    entity->next = instance->entity_head;
    instance->entity_head = entity;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    /* the instance now owns this entity, so we can skip cleanup. */
    goto done;

cleanup_enc_pubkey:
    dispose((disposable_t*)&entity->enc_pubkey);

cleanup_entity:
    memset(entity, 0, sizeof(ups_authorized_entity_t));
    free(entity);

done:
    return retval;
}

/**
 * \brief Dispose of an authorized entity entry.
 *
 * \param disp      The authorized entity entry to be disposed.
 */
static void ups_authorized_entity_dispose(void* disp)
{
    ups_authorized_entity_t* entity = (ups_authorized_entity_t*)disp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != entity);

    /* dispose of buffers. */
    dispose((disposable_t*)&entity->enc_pubkey);
    dispose((disposable_t*)&entity->sign_pubkey);

    /* clear out the structure. */
    memset(entity, 0, sizeof(ups_authorized_entity_t));
}
