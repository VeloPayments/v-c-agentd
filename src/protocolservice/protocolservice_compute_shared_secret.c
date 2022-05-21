/**
 * \file protocolservice/protocolservice_compute_shared_secret.c
 *
 * \brief Compute the shared secret between the server and the client.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

/* TODO - the private key should be moved to the auth service. Instead, this
 * method should call the auth service to get the shared secret, so that the
 * private key does not leak beyond the supervisor and the auth service.
 */

/* TODO - the protocol should be updated to use forward secrecy, in which both
 * the client and the server derive session keys, signed by their long-term
 * signing keys.  These should be used to derive the shared secret, and then
 * these should be discarded.
 */

/**
 * \brief Compute a shared secret based on the nonce data gathered during the
 * handshake, the server private key, and the client public key.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_compute_shared_secret(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval;
    bool agreement_init = false;
    vccrypt_key_agreement_context_t agreement;

    /* create the key agreement instance. */
    retval =
        vccrypt_suite_cipher_key_agreement_init(&ctx->ctx->suite, &agreement);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* the agreement instance is now initialized. */
    agreement_init = true;

    /* derive the shared secret using the key nonces. */
    retval =
        vccrypt_key_agreement_short_term_secret_create(
            &agreement, &ctx->ctx->agentd_enc_privkey,
            &ctx->entity->encryption_pubkey, &ctx->server_key_nonce,
            &ctx->client_key_nonce, &ctx->shared_secret);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* set the IVs. */
    ctx->client_iv = 0x0000000000000001UL;
    ctx->server_iv = 0x8000000000000001UL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_agreement;

write_error_response:
    retval =
        protocolservice_write_error_response(
            ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, 0U, false);
    if (STATUS_SUCCESS == retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
    }

cleanup_agreement:
    if (agreement_init)
    {
        dispose((disposable_t*)&agreement);
    }

    return retval;
}
