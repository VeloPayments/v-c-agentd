/**
 * \file protocolservice/ups_private_key_set.c
 *
 * \brief Set the private key for this protocol service instance.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/status_codes.h>

#if !defined(AGENTD_NEW_PROTOCOL)

#include "unauthorized_protocol_service_private.h"

/* forward decls. */
static void ups_private_key_entry_dispose(void* disp);

/**
 * \brief Set the private key for the protocol service.
 *
 * \param instance      The instance for which the private key is set.
 * \param entity_id     The UUID of the blockchain agent entity.
 * \param encpub        The raw bytes of the encryption public key.
 * \param encpriv       The raw bytes of the encryption private key.
 * \param signpub       The raw bytes of the signing public key.
 * \param signpriv      The raw bytes of the signing private key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 */
int ups_private_key_set(
    unauthorized_protocol_service_instance_t* instance,
    const uint8_t* entity_id, const uint8_t* encpub, const uint8_t* encpriv,
    const uint8_t* signpub, const uint8_t* signpriv)
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != encpub);
    MODEL_ASSERT(NULL != encpriv);
    MODEL_ASSERT(NULL != signpub);
    MODEL_ASSERT(NULL != signpriv);

    /* if the private key is already set, then don't go any further. */
    if (NULL != instance->private_key)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_PRIVATE_KEY_ALREADY_SET;
        goto done;
    }

    /* allocate memory for a new private key instance. */
    ups_private_key_t* priv =
        (ups_private_key_t*)malloc(sizeof(ups_private_key_t));
    if (NULL == priv)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set the dispose method for this key. */
    priv->hdr.dispose = &ups_private_key_entry_dispose;

    /* copy the uuid to this instance. */
    memcpy(priv->id, entity_id, 16);

    /* initialize the encryption public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            &instance->suite, &priv->enc_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_priv;
    }

    /* copy the encryption public key to this buffer. */
    memcpy(priv->enc_pubkey.data, encpub, priv->enc_pubkey.size);

    /* initialize the encryption private key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_private_key(
            &instance->suite, &priv->enc_privkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubkey;
    }

    /* copy the encryption private key to this buffer. */
    memcpy(priv->enc_privkey.data, encpriv, priv->enc_privkey.size);

    /* initialize the signing public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_public_key(
            &instance->suite, &priv->sign_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_privkey;
    }

    /* copy the signing public key to this buffer. */
    memcpy(priv->sign_pubkey.data, signpub, priv->sign_pubkey.size);

    /* initialize the signing private key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_private_key(
            &instance->suite, &priv->sign_privkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_sign_pubkey;
    }

    /* copy the signing private key to this buffer. */
    memcpy(priv->sign_privkey.data, signpriv, priv->sign_privkey.size);

    /* save this entry to the instance. */
    instance->private_key = priv;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    /* the instance now owns this key, so we can skip cleanup. */
    goto done;

cleanup_sign_pubkey:
    dispose((disposable_t*)&priv->sign_pubkey);

cleanup_enc_privkey:
    dispose((disposable_t*)&priv->enc_privkey);

cleanup_enc_pubkey:
    dispose((disposable_t*)&priv->enc_pubkey);

cleanup_priv:
    memset(priv, 0, sizeof(ups_private_key_t));
    free(priv);

done:
    return retval;
}

/**
 * \brief Dispose of a private key instance.
 *
 * \param disp      The private key entry to be disposed.
 */
static void ups_private_key_entry_dispose(void* disp)
{
    ups_private_key_t* priv = (ups_private_key_t*)disp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != priv);

    /* dispose of buffers. */
    dispose((disposable_t*)&priv->enc_pubkey);
    dispose((disposable_t*)&priv->enc_privkey);
    dispose((disposable_t*)&priv->sign_pubkey);
    dispose((disposable_t*)&priv->sign_privkey);

    /* clear out the structure. */
    memset(priv, 0, sizeof(ups_private_key_t));
}

#endif /* !defined(AGENTD_NEW_PROTOCOL) */
