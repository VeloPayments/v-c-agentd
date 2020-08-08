/**
 * \file
 * canonization/canonizationservice_decode_and_dispatch_control_command_private_key_set.c
 *
 * \brief Decode and dispatch the private key set command.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/* forward decls. */
static int canonizationservice_private_key_set(
    canonizationservice_instance_t* instance,
    const uint8_t* entity_id, const uint8_t* encpub, const uint8_t* encpriv,
    const uint8_t* signpub, const uint8_t* signpriv);
static void canonizationservice_private_key_dispose(void* disp);

/**
 * \brief Decode and dispatch a private key set request.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int canonizationservice_decode_and_dispatch_control_command_private_key_set(
    canonizationservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* req, size_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* get the expected size for all keys from the crypto suite. */
    const size_t expected_enc_pubkey_size =
        instance->crypto_suite.key_cipher_opts.public_key_size;
    const size_t expected_enc_privkey_size =
        instance->crypto_suite.key_cipher_opts.private_key_size;
    const size_t expected_sign_pubkey_size =
        instance->crypto_suite.sign_opts.public_key_size;
    const size_t expected_sign_privkey_size =
        instance->crypto_suite.sign_opts.private_key_size;

    /* calculate the payload header size. */
    const size_t payload_header_size = 5 * sizeof(uint32_t);

    /* ensure that the payload size is at least large enough to hold this
     * header. */
    if (size < payload_header_size)
    {
        canonizationservice_decode_and_dispatch_write_status(
            sock, CANONIZATIONSERVICE_API_METHOD_PRIVATE_KEY_SET, 0U,
            AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL,
            0);

        return AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* make working with the payload more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* skip the request offset. */
    breq += sizeof(uint32_t);

    /* get the encryption pubkey size. */
    uint32_t nenc_pubkey_size;
    memcpy(&nenc_pubkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t enc_pubkey_size = ntohl(nenc_pubkey_size);

    /* get the encryption privkey size. */
    uint32_t nenc_privkey_size;
    memcpy(&nenc_privkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t enc_privkey_size = ntohl(nenc_privkey_size);

    /* get the signing pubkey size. */
    uint32_t nsign_pubkey_size;
    memcpy(&nsign_pubkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sign_pubkey_size = ntohl(nsign_pubkey_size);

    /* get the signing privkey size. */
    uint32_t nsign_privkey_size;
    memcpy(&nsign_privkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sign_privkey_size = ntohl(nsign_privkey_size);

    /* verify key sizes. */
    if (    expected_enc_pubkey_size != enc_pubkey_size
        ||  expected_enc_privkey_size != enc_privkey_size
        ||  expected_sign_pubkey_size != sign_pubkey_size
        ||  expected_sign_privkey_size != sign_privkey_size)
    {
        canonizationservice_decode_and_dispatch_write_status(
            sock, CANONIZATIONSERVICE_API_METHOD_PRIVATE_KEY_SET, 0U,
            AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL,
            0);

        return AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* compute remaining payload size. */
    const size_t payload_size =
          16 /* entity id */
        + enc_pubkey_size
        + enc_privkey_size
        + sign_pubkey_size
        + sign_privkey_size;

    /* verify that the remaining payload size is large enough. */
    size -= payload_header_size;
    if (size < payload_size)
    {
        canonizationservice_decode_and_dispatch_write_status(
            sock, CANONIZATIONSERVICE_API_METHOD_PRIVATE_KEY_SET, 0U,
            AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL,
            0);

        return AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* get the id and key buffers. */
    const uint8_t* id = breq; breq += 16;
    const uint8_t* enc_pubkey = breq; breq += enc_pubkey_size;
    const uint8_t* enc_privkey = breq; breq += enc_privkey_size;
    const uint8_t* sign_pubkey = breq; breq += sign_pubkey_size;
    const uint8_t* sign_privkey = breq; breq += sign_privkey_size;

    /* set the private key. */
    int retval =
        canonizationservice_private_key_set(
            instance, id, enc_pubkey, enc_privkey, sign_pubkey, sign_privkey);

    /* write a success status. */
    canonizationservice_decode_and_dispatch_write_status(
        sock, CANONIZATIONSERVICE_API_METHOD_PRIVATE_KEY_SET, 0U,
        retval, NULL, 0);

    return retval;
}

/**
 * \brief Set the private key.
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
static int canonizationservice_private_key_set(
    canonizationservice_instance_t* instance,
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
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_PRIVATE_KEY_ALREADY_SET;
        goto done;
    }

    /* allocate memory for a new private key instance. */
    canonizationservice_private_key_t* priv =
        (canonizationservice_private_key_t*)malloc(
            sizeof(canonizationservice_private_key_t));
    if (NULL == priv)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* clear out this instance. */
    memset(priv, 0, sizeof(canonizationservice_private_key_t));

    /* set the dispose method. */
    priv->hdr.dispose = &canonizationservice_private_key_dispose;

    /* copy the uuid. */
    memcpy(priv->id, entity_id, 16);

    /* initialize the encryption public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            &instance->crypto_suite, &priv->enc_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_priv;
    }

    /* copy the encryption public key to this buffer. */
    memcpy(priv->enc_pubkey.data, encpub, priv->enc_pubkey.size);

    /* initialize the encryption private key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_private_key(
            &instance->crypto_suite, &priv->enc_privkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubkey;
    }

    /* copy the encryption private key to this buffer. */
    memcpy(priv->enc_privkey.data, encpriv, priv->enc_privkey.size);

    /* initialize the signing public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_public_key(
            &instance->crypto_suite, &priv->sign_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_privkey;
    }

    /* copy the signing public key to this buffer. */
    memcpy(priv->sign_pubkey.data, signpub, priv->sign_pubkey.size);

    /* initialize the signing private key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_private_key(
            &instance->crypto_suite, &priv->sign_privkey);
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
    memset(priv, 0, sizeof(canonizationservice_private_key_t));
    free(priv);

done:
    return retval;
}

/**
 * \brief Dispose of a private key instance.
 *
 * \param disp      The private key entry to be disposed.
 */
static void canonizationservice_private_key_dispose(void* disp)
{
    canonizationservice_private_key_t* priv =
        (canonizationservice_private_key_t*)disp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != priv);

    /* dispose of buffers. */
    dispose((disposable_t*)&priv->enc_pubkey);
    dispose((disposable_t*)&priv->enc_privkey);
    dispose((disposable_t*)&priv->sign_pubkey);
    dispose((disposable_t*)&priv->sign_privkey);

    /* clear out the structure. */
    memset(priv, 0, sizeof(canonizationservice_private_key_t));
}
