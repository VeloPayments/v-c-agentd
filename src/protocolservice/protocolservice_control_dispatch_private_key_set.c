/**
 * \file protocolservice/protocolservice_control_dispatch_private_key_set.c
 *
 * \brief Dispatch a private key set control command.
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

/**
 * \brief Dispatch a private key set request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_private_key_set(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size)
{
    status retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(size > 0);

    /* get the expected size for all keys from the crypto suite. */
    const size_t expected_enc_pubkey_size =
        ctx->ctx->suite.key_cipher_opts.public_key_size;
    const size_t expected_enc_privkey_size =
        ctx->ctx->suite.key_cipher_opts.private_key_size;
    const size_t expected_sign_pubkey_size =
        ctx->ctx->suite.sign_opts.public_key_size;
    const size_t expected_sign_privkey_size =
        ctx->ctx->suite.sign_opts.private_key_size;

    /* compute the message header size. */
    const size_t payload_header_size = 5 * sizeof(uint32_t);

    /* ensure that the payload size is at least large enough to hold this
     * header. */
    if (size < payload_header_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET,
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
    memcpy(&noffset, breq, sizeof(uint32_t));
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
    uint32_t nsig_pubkey_size;
    memcpy(&nsig_pubkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sig_pubkey_size = ntohl(nsig_pubkey_size);

    /* get the signing privkey size. */
    uint32_t nsig_privkey_size;
    memcpy(&nsig_privkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sig_privkey_size = ntohl(nsig_privkey_size);

    /* verify key sizes. */
    if (    expected_enc_pubkey_size != enc_pubkey_size
        ||  expected_enc_privkey_size != enc_privkey_size
        ||  expected_sign_pubkey_size != sig_pubkey_size
        ||  expected_sign_privkey_size != sig_privkey_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* compute remaining payload size. */
    const size_t payload_size =
          16 /* entity id */
        + enc_pubkey_size
        + enc_privkey_size
        + sig_pubkey_size
        + sig_privkey_size;

    /* verify that the remaining payload size is large enough. */
    size -= payload_header_size;
    if (size < payload_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* get the id and key buffers. */
    const uint8_t* id = breq; breq += 16;
    const uint8_t* enc_pubkey = breq; breq += enc_pubkey_size;
    const uint8_t* enc_privkey = breq; breq += enc_privkey_size;
    const uint8_t* sig_pubkey = breq; breq += sig_pubkey_size;
    const uint8_t* sig_privkey = breq; breq += sig_privkey_size;

    /* copy the data to the context buffers. */
    memcpy(&ctx->ctx->agentd_uuid, id, 16);
    memcpy(ctx->ctx->agentd_enc_pubkey.data, enc_pubkey, enc_pubkey_size);
    memcpy(ctx->ctx->agentd_enc_privkey.data, enc_privkey, enc_privkey_size);
    memcpy(ctx->ctx->agentd_sign_pubkey.data, sig_pubkey, sig_pubkey_size);
    memcpy(ctx->ctx->agentd_sign_privkey.data, sig_privkey, sig_privkey_size);

    /* the private key has been set. */
    ctx->ctx->private_key_set = true;

    /* success. */
    retval =
        protocolservice_control_write_response(
            ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET,
            STATUS_SUCCESS);
    goto done;

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
