/**
 * \file
 * protocolservice/protocolservice_control_api_sendreq_authorized_entity_add.c
 *
 * \brief Send the authorized entity add request to the protocol service control
 * socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include <agentd/protocolservice/control_api.h>

/**
 * \brief Add an authorized entity to the protocol service.
 *
 * This entity is allowed to connect to the protocol service and send requests
 * to this service.
 *
 * \param sock                  The socket to which this request is written.
 * \param suite                 The crypto suite.
 * \param entity_id             The entity UUID.
 * \param entity_enc_pubkey     The entity's encryption public key.
 * \param entity_sign_pubkey    The entity's signing public key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_control_api_sendreq_authorized_entity_add(
    int sock, vccrypt_suite_options_t* suite, const uint8_t* entity_id,
    const vccrypt_buffer_t* entity_enc_pubkey,
    const vccrypt_buffer_t* entity_sign_pubkey)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != entity_enc_pubkey);
    MODEL_ASSERT(NULL != entity_sign_pubkey);

    /* create a buffer for holding the request. */
    size_t req_size =
          4 * sizeof(uint32_t)
        + 16 /* entity id size */
        + entity_enc_pubkey->size
        + entity_sign_pubkey->size;
    vccrypt_buffer_t req;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &req, suite->alloc_opts, req_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* get a buffer pointer for convenience. */
    uint8_t* breq = (uint8_t*)req.data;

    /* write the method id. */
    uint32_t net_method_id =
        htonl(UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD);
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

    /* write the signing pubkey size. */
    uint32_t net_sign_pubkey_size = htonl(entity_sign_pubkey->size);
    memcpy(breq, &net_sign_pubkey_size, sizeof(net_sign_pubkey_size));
    breq += sizeof(net_sign_pubkey_size);

    /* write the entity id. */
    memcpy(breq, entity_id, 16);
    breq += 16;

    /* write the encryption pubkey. */
    memcpy(breq, entity_enc_pubkey->data, entity_enc_pubkey->size);
    breq += entity_enc_pubkey->size;

    /* write the signing pubkey. */
    memcpy(breq, entity_sign_pubkey->data, entity_sign_pubkey->size);
    breq += entity_sign_pubkey->size;

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
