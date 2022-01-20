/**
 * \file test_protocolservice_isolation.cpp
 *
 * Isolation tests for the protocol service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/api.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_protocolservice_isolation.h"

using namespace std;

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * Test that we can spawn the unauthorized protocol service.
 */
TEST_F(protocolservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, proto_proc_status);
}

/**
 * Test that writing a bad packet type results in an error.
 */
TEST_F(protocolservice_isolation_test, handshake_request_bad)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    ASSERT_EQ(0, ipc_write_string_block(protosock, "this is a test"));

    /* An invalid packet ends the connection before we can read a valid
     * response. */
    ASSERT_NE(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a malformed data packet results in an error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_size)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    ASSERT_EQ(0, ipc_write_data_block(protosock, "123", 3));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a request id other than one that initiates the
 * handshake results in an error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_reqid)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t bad_request_id = htonl(0x01U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &bad_request_id, sizeof(bad_request_id));
    breq += sizeof(bad_request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a non-zero offset for the handshake request results in an
 * error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_offset)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t bad_request_offset = htonl(0x01U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &bad_request_offset, sizeof(bad_request_offset));
    breq += sizeof(bad_request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that an invalid protocol version results in an error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_protocol_version)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t bad_protocol_version_requested = htonl(0x02U);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &bad_protocol_version_requested,
        sizeof(bad_protocol_version_requested));
    breq += sizeof(bad_protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that an invalid crypto suite results in an error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_crypto_suite)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t bad_crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1+5);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &bad_crypto_suite_version_requested,
        sizeof(bad_crypto_suite_version_requested));
    breq += sizeof(bad_crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a handshake request with a bad entity id results in an
 * error.
 */
TEST_F(protocolservice_isolation_test, handshake_req_bad_entity)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return an unauthorized error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a valid handshake request results in a valid handshake
 * response.
 */
TEST_F(protocolservice_isolation_test, handshake_request_happy)
{
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    (void)offset;
    (void)status;
    (void)client_key_nonce;
    (void)client_challenge_nonce;
    (void)server_public_key;
    (void)server_id;
    (void)shared_secret;
    (void)server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    ASSERT_TRUE(suite_initialized);

    /* add the hardcoded keys. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, add_hardcoded_keys());

    /* write the handshake request to the socket. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_handshake_request_block(
            protosock, &suite, authorized_entity_id, &client_key_nonce,
            &client_challenge_nonce));

    /* This should return successfully. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);

    /* the server id is correct. */
    EXPECT_EQ(16U, server_id.size);
    EXPECT_EQ(0, memcmp(server_id.data, agent_id, server_id.size));

    /* the server public key is correct. */
    EXPECT_EQ(32U, server_public_key.size);
    EXPECT_EQ(0,
        memcmp(
            server_public_key.data, agent_enc_pubkey_buffer,
            server_public_key.size));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
