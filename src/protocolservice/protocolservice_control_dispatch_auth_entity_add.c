/**
 * \file protocolservice/protocolservice_control_dispatch_auth_entity_add.c
 *
 * \brief Dispatch an auth entity add control command.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_uuid;

/**
 * \brief Dispatch an auth entity add control request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_auth_entity_add(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size)
{
    status retval, release_retval;
    vccrypt_buffer_t enc_pubkey;
    vccrypt_buffer_t sign_pubkey;
    rcpr_uuid entity_id;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(size > 0);

    /* get the expected sizes for the public encryption and signing keys. */
    const size_t expected_enc_pubkey_size =
        ctx->ctx->suite.key_cipher_opts.public_key_size;
    const size_t expected_sign_pubkey_size =
        ctx->ctx->suite.sign_opts.public_key_size;

    /* compute the message header size. */
    const size_t payload_header_size = 3 * sizeof(uint32_t);

    /* ensure that the payload size is at least large enough to hold this
     * header. */
    if (size < payload_header_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* treat the request as a byte buffer for convenience. */
    const uint8_t* breq = (const uint8_t*)payload;

    /* get the request offset. */
    uint32_t noffset;
    memcpy(&noffset, breq, sizeof(uint32_t)); breq += sizeof(uint32_t);

    /* get the encryption pubkey size. */
    uint32_t nenc_pubkey_size;
    memcpy(&nenc_pubkey_size, breq, sizeof(uint32_t)); breq += sizeof(uint32_t);
    uint32_t enc_pubkey_size = ntohl(nenc_pubkey_size);

    /* get the signing pubkey size. */
    uint32_t nsig_pubkey_size;
    memcpy(&nsig_pubkey_size, breq, sizeof(uint32_t)); breq += sizeof(uint32_t);
    uint32_t sig_pubkey_size = ntohl(nsig_pubkey_size);

    /* verify pubkey sizes. */
    if (    expected_enc_pubkey_size != enc_pubkey_size
        ||  expected_sign_pubkey_size != sig_pubkey_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* compute the remaining payload size. */
    const size_t payload_size =
        sizeof(rcpr_uuid)
      + enc_pubkey_size
      + sig_pubkey_size;

    /* verify that the remaining payload size is large enough. */
    size -= payload_header_size;
    if (size < payload_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* copy the entity uuid. */
    memcpy(&entity_id, breq, 16);

    /* initialize the encryption public key. */
    retval =
        vccrypt_buffer_init(
            &enc_pubkey, &ctx->ctx->vpr_alloc, enc_pubkey_size);
    if (STATUS_SUCCESS != retval)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                AGENTD_ERROR_GENERAL_OUT_OF_MEMORY);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        }
        goto done;
    }

    /* copy the encryption public key. */
    memcpy(enc_pubkey.data, breq + 16, enc_pubkey_size);

    /* initialize the signing public key. */
    retval =
        vccrypt_buffer_init(
            &sign_pubkey, &ctx->ctx->vpr_alloc, sig_pubkey_size);
    if (STATUS_SUCCESS != retval)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                AGENTD_ERROR_GENERAL_OUT_OF_MEMORY);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        }
        goto cleanup_enc_pubkey;
    }

    /* copy the signing public key. */
    memcpy(sign_pubkey.data, breq + 16 + enc_pubkey_size, sig_pubkey_size);

    /* add the entity to the context. */
    retval =
        protocolservice_authorized_entity_add(
            ctx->ctx, &entity_id, &enc_pubkey, &sign_pubkey);
    if (STATUS_SUCCESS != retval)
    {
        release_retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
                retval);
        if (STATUS_SUCCESS == release_retval)
        {
            retval = release_retval;
        }
        goto cleanup_sign_pubkey;
    }

    /* success. */
    retval =
        protocolservice_control_write_response(
            ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
            STATUS_SUCCESS);
    goto cleanup_sign_pubkey;

cleanup_sign_pubkey:
    dispose((disposable_t*)&sign_pubkey);

cleanup_enc_pubkey:
    dispose((disposable_t*)&enc_pubkey);

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
