/**
 * \file canonization/canonization_api_sendreq_private_key_set.c
 *
 * \brief Send the private key set request to the canonization service control
 * socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include <agentd/canonizationservice/api.h>

/**
 * \brief Set the private key for the canonization service.
 *
 * \param sock                  The socket to which this request is written.
 * \param alloc_opts            The allocator options.
 * \param entity_id             The blockchain agent entity UUID.
 * \param entity_enc_pubkey     The entity's encryption public key.
 * \param entity_enc_privkey    The entity's encryption private key.
 * \param entity_sign_pubkey    The entity's signing public key.
 * \param entity_sign_privkey   The entity's signing private key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int canonization_api_sendreq_private_key_set(
    int sock, allocator_options_t* alloc_opts, const uint8_t* entity_id,
    const vccrypt_buffer_t* entity_enc_pubkey,
    const vccrypt_buffer_t* entity_enc_privkey,
    const vccrypt_buffer_t* entity_sign_pubkey,
    const vccrypt_buffer_t* entity_sign_privkey)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != entity_enc_pubkey);
    MODEL_ASSERT(NULL != entity_sign_pubkey);

    /* create a buffer for holding the request. */
    size_t req_size =
          6 * sizeof(uint32_t)
        + 16 /* entity id size */
        + entity_enc_pubkey->size
        + entity_enc_privkey->size
        + entity_sign_pubkey->size
        + entity_sign_privkey->size;
    vccrypt_buffer_t req;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &req, alloc_opts, req_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* get a buffer pointer for convenience. */
    uint8_t* breq = (uint8_t*)req.data;

    /* write the method id. */
    uint32_t net_method_id =
        htonl(CANONIZATIONSERVICE_API_METHOD_PRIVATE_KEY_SET);
    memcpy(breq, &net_method_id, sizeof(net_method_id));
    breq += sizeof(net_method_id);

    /* write the request id. */
    uint32_t net_request_id = htonl(0UL);
    memcpy(breq, &net_request_id, sizeof(net_request_id));
    breq += sizeof(net_request_id);

    /* write the encryption pubkey size. */
    uint32_t net_enc_pubkey_size = htonl(entity_enc_pubkey->size);
    memcpy(breq, &net_enc_pubkey_size, sizeof(net_enc_pubkey_size));
    breq += sizeof(net_enc_pubkey_size);

    /* write the encryption privkey size. */
    uint32_t net_enc_privkey_size = htonl(entity_enc_privkey->size);
    memcpy(breq, &net_enc_privkey_size, sizeof(net_enc_privkey_size));
    breq += sizeof(net_enc_privkey_size);

    /* write the signing pubkey size. */
    uint32_t net_sign_pubkey_size = htonl(entity_sign_pubkey->size);
    memcpy(breq, &net_sign_pubkey_size, sizeof(net_sign_pubkey_size));
    breq += sizeof(net_sign_pubkey_size);

    /* write the signing privkey size. */
    uint32_t net_sign_privkey_size = htonl(entity_sign_privkey->size);
    memcpy(breq, &net_sign_privkey_size, sizeof(net_sign_privkey_size));
    breq += sizeof(net_sign_privkey_size);

    /* write the entity id. */
    memcpy(breq, entity_id, 16);
    breq += 16;

    /* write the encryption pubkey. */
    memcpy(breq, entity_enc_pubkey->data, entity_enc_pubkey->size);
    breq += entity_enc_pubkey->size;

    /* write the encryption privkey. */
    memcpy(breq, entity_enc_privkey->data, entity_enc_privkey->size);
    breq += entity_enc_privkey->size;

    /* write the signing pubkey. */
    memcpy(breq, entity_sign_pubkey->data, entity_sign_pubkey->size);
    breq += entity_sign_pubkey->size;

    /* write the signing privkey. */
    memcpy(breq, entity_sign_privkey->data, entity_sign_privkey->size);
    breq += entity_sign_privkey->size;

    /* write the request packet to the server. */
    retval = ipc_write_data_block(sock, req.data, req.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_req;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
