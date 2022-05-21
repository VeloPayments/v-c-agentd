/**
 * \file protocolservice/protocolservice_protocol_write_handshake_req_resp.c
 *
 * \brief Write the handshake request response.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_psock;

/**
 * \brief Write the response to the handshake request to the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_handshake_req_resp(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval;
    vccrypt_buffer_t payload;
    bool payload_init = false;
    vccrypt_mac_context_t mac;
    bool mac_init = false;
    vccrypt_buffer_t mac_buffer;
    bool mac_buffer_init = false;

    /* Compute the response packet payload size. */
    uint32_t request_id = htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE);
    uint32_t offset = htonl(0x00U);
    uint32_t protocol_version = htonl(0x00000001);
    uint32_t crypto_suite = htonl(VCCRYPT_SUITE_VELO_V1);
    int32_t status = htonl(STATUS_SUCCESS);
    size_t payload_size =
          sizeof(request_id)
        + sizeof(offset)
        + sizeof(status)
        + sizeof(protocol_version)
        + sizeof(crypto_suite)
        + 16 /* agentd entity id */
        + ctx->ctx->agentd_enc_pubkey.size
        + ctx->server_key_nonce.size
        + ctx->server_challenge_nonce.size
        + ctx->ctx->suite.mac_short_opts.mac_size;

    /* create the response payload buffer. */
    retval = vccrypt_buffer_init(&payload, &ctx->ctx->vpr_alloc, payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* the payload has been initialized. */
    payload_init = true;

    /* create the HMAC instance. */
    retval =
        vccrypt_suite_mac_short_init(
            &ctx->ctx->suite, &mac, &ctx->shared_secret);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* the mac has been initialized. */
    mac_init = true;

    /* create the buffer for holding the mac output. */
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            &ctx->ctx->suite, &mac_buffer, true);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* the mac buffer has been initialized. */
    mac_buffer_init = true;

    /* convenience pointer for working with this buffer. */
    uint8_t* buf_start = (uint8_t*)payload.data;
    uint8_t* pbuf = (uint8_t*)payload.data;

    /* write the payload values to this buffer. */
    memcpy(pbuf, &request_id, sizeof(request_id));
    pbuf += sizeof(request_id);
    memcpy(pbuf, &status, sizeof(status));
    pbuf += sizeof(status);
    memcpy(pbuf, &offset, sizeof(offset));
    pbuf += sizeof(offset);
    memcpy(pbuf, &protocol_version, sizeof(protocol_version));
    pbuf += sizeof(protocol_version);
    memcpy(pbuf, &crypto_suite, sizeof(crypto_suite));
    pbuf += sizeof(crypto_suite);
    memcpy(pbuf, &ctx->ctx->agentd_uuid, 16);
    pbuf += 16;
    memcpy(
        pbuf, ctx->ctx->agentd_enc_pubkey.data,
        ctx->ctx->agentd_enc_pubkey.size);
    pbuf += ctx->ctx->agentd_enc_pubkey.size;
    memcpy(pbuf, ctx->server_key_nonce.data, ctx->server_key_nonce.size);
    pbuf += ctx->server_key_nonce.size;
    memcpy(
        pbuf, ctx->server_challenge_nonce.data,
        ctx->server_challenge_nonce.size);
    pbuf += ctx->server_challenge_nonce.size;

    /* digest the response packet.*/
    retval = vccrypt_mac_digest(&mac, buf_start, pbuf - buf_start);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* add the client challenge nonce to the response packet. */
    retval =
        vccrypt_mac_digest(
            &mac, ctx->client_challenge_nonce.data,
            ctx->client_challenge_nonce.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* finalize the mac. */
    retval = vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* copy the hmac to the payload. */
    memcpy(pbuf, mac_buffer.data, mac_buffer.size);
    pbuf += mac_buffer.size;

    /* write the data to the socket. */
    retval =
        psock_write_boxed_data(
            ctx->protosock, payload.data, payload.size);
    if (STATUS_SUCCESS != retval)
    {
        goto write_error_response;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

write_error_response:
    retval =
        protocolservice_write_error_response(
            ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, 0U, false);
    if (STATUS_SUCCESS == retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
    }

done:
    if (payload_init)
    {
        dispose((disposable_t*)&payload);
    }

    if (mac_init)
    {
        dispose((disposable_t*)&mac);
    }

    if (mac_buffer_init)
    {
        dispose((disposable_t*)&mac_buffer);
    }

    return retval;
}
