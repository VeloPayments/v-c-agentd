/**
 * \file protocolservice/protocolservice_protocol_read_handshake_req.c
 *
 * \brief Read the handshake request from the client.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Read the handshake request from the client.
 *
 * \param ctx       The protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_handshake_req(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    void* req = NULL;
    size_t size = 0U;
    uint32_t request_id;
    uint32_t request_offset;
    uint32_t protocol_version;
    uint32_t crypto_suite;

    /* read the request packet. */
    retval = psock_read_boxed_data(ctx->protosock, ctx->alloc, &req, &size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_BAD;
        goto done;
    }

    /* set up the read buffer pointer. */
    const uint8_t* breq = (const uint8_t*)req;

    /* verify that the size matches what we expect. */
    const size_t request_id_size = sizeof(request_id);
    const size_t request_offset_size = sizeof(request_offset);
    const size_t protocol_version_size = sizeof(protocol_version);
    const size_t crypto_suite_size = sizeof(crypto_suite);
    const size_t entity_uuid_size = sizeof(ctx->entity_uuid);
    const size_t expected_size =
          request_id_size
        + request_offset_size
        + protocol_version_size
        + crypto_suite_size
        + entity_uuid_size
        + ctx->client_key_nonce.size
        + ctx->client_challenge_nonce.size;
    if (size != expected_size)
    {
        retval =
            protocolservice_write_error_response(
                ctx, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0,
                false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        }
        goto cleanup_data;
    }

    /* read the request ID and verify it. */
    memcpy(&request_id, breq, request_id_size);
    breq += request_id_size;
    request_id = ntohl(request_id);
    if (UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE != request_id)
    {
        retval =
            protocolservice_write_error_response(
                ctx, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0,
                false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        }
        goto cleanup_data;
    }

    /* read the request offset. It should be 0x00000000. */
    memcpy(&request_offset, breq, request_offset_size);
    breq += request_offset_size;
    request_offset = ntohl(request_offset);
    if (0x00000000 != request_offset)
    {
        retval =
            protocolservice_write_error_response(
                ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
                AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        }
        goto cleanup_data;
    }

    /* read the protocol version. It should be 0x00000001. */
    memcpy(&protocol_version, breq, protocol_version_size);
    breq += protocol_version_size;
    protocol_version = ntohl(protocol_version);
    if (0x00000001 != protocol_version)
    {
        retval =
            protocolservice_write_error_response(
                ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
                AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        }
        goto cleanup_data;
    }

    /* read the crypto suite version.  It should be VCCRYPT_SUITE_VELO_V1. */
    memcpy(&crypto_suite, breq, crypto_suite_size);
    breq += crypto_suite_size;
    crypto_suite = ntohl(crypto_suite);
    if (VCCRYPT_SUITE_VELO_V1 != crypto_suite)
    {
        retval =
            protocolservice_write_error_response(
                ctx, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
                AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST;
        }
        goto cleanup_data;
    }

    /* read the entity uuid. */
    memcpy(&ctx->entity_uuid, breq, entity_uuid_size);
    breq += entity_uuid_size;

    /* read the client key nonce. */
    memcpy(ctx->client_key_nonce.data, breq, ctx->client_key_nonce.size);
    breq += ctx->client_key_nonce.size;

    /* read the client challenge nonce. */
    memcpy(
        ctx->client_challenge_nonce.data, breq,
        ctx->client_challenge_nonce.size);
    breq += ctx->client_challenge_nonce.size;

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_data;

cleanup_data:
    memset(req, 0, size);
    release_retval = rcpr_allocator_reclaim(ctx->alloc, req);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
