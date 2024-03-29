/**
 * \file test_protocolservice_isolation.cpp
 *
 * Isolation tests for the protocol service.
 *
 * \copyright 2021-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/api.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <minunit/minunit.h>
#include <string>
#include <unistd.h>
#include <vcblockchain/protocol.h>
#include <vcblockchain/protocol/data.h>
#include <vcblockchain/protocol/serialization.h>
#include <vpr/disposable.h>

#include "test_protocolservice_isolation.h"

using namespace std;

RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

TEST_SUITE(protocolservice_isolation_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    protocolservice_isolation_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the unauthorized protocol service.
 */
BEGIN_TEST_F(simple_spawn)
    TEST_ASSERT(0 == fixture.proto_proc_status);
END_TEST_F()

/**
 * Test that writing a bad packet type results in an error.
 */
BEGIN_TEST_F(handshake_request_bad)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    TEST_ASSERT(
        0 == ipc_write_string_block(fixture.protosock, "this is a test"));

    /* An invalid packet ends the connection before we can read a valid
     * response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            != protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that writing a malformed data packet results in an error.
 */
BEGIN_TEST_F(handshake_req_bad_size)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    TEST_ASSERT(0 == ipc_write_data_block(fixture.protosock, "123", 3));

    /* we return a truncated error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that writing a request id other than one that initiates the
 * handshake results in an error.
 */
BEGIN_TEST_F(handshake_req_bad_reqid)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
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

    TEST_ASSERT(
        0 == ipc_write_data_block(fixture.protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that writing a non-zero offset for the handshake request results in an
 * error.
 */
BEGIN_TEST_F(handshake_req_bad_offset)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
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

    TEST_ASSERT(
        0 == ipc_write_data_block(fixture.protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that an invalid protocol version results in an error.
 */
BEGIN_TEST_F(handshake_req_bad_protocol_version)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
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

    TEST_ASSERT(
        0 == ipc_write_data_block(fixture.protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that an invalid crypto suite results in an error.
 */
BEGIN_TEST_F(handshake_req_bad_crypto_suite)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t bad_crypto_suite_version_requested =
        htonl(VCCRYPT_SUITE_VELO_V1+5);
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

    TEST_ASSERT(
        0 == ipc_write_data_block(fixture.protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that writing a handshake request with a bad entity id results in an
 * error.
 */
BEGIN_TEST_F(handshake_req_bad_entity)
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
                    &fixture.suite, &client_challenge_nonce));
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

    TEST_ASSERT(
        0 == ipc_write_data_block(fixture.protosock, payload, sizeof(payload)));

    /* we return an unauthorized error response. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
END_TEST_F()

/**
 * Test that writing a valid handshake request results in a valid handshake
 * response.
 */
BEGIN_TEST_F(handshake_request_happy)
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
    TEST_ASSERT(fixture.suite_initialized);

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* write the handshake request to the socket. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_handshake_request_block(
                    fixture.protosock, &fixture.suite,
                    fixture.authorized_entity_id, &client_key_nonce,
                    &client_challenge_nonce));

    /* This should return successfully. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);

    /* the server id is correct. */
    TEST_EXPECT(16U == server_id.size);
    TEST_EXPECT(0 == memcmp(server_id.data, fixture.agent_id, server_id.size));

    /* the server public key is correct. */
    TEST_EXPECT(32U == server_public_key.size);
    TEST_EXPECT(
        0
            == memcmp(
                    server_public_key.data, fixture.agent_enc_pubkey_buffer,
                    server_public_key.size));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
END_TEST_F()

/**
 * Writing an unencrypted packet after a valid handshake response causes an
 * error.
 */
BEGIN_TEST_F(handshake_response_plaintext_error)
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    TEST_ASSERT(fixture.suite_initialized);

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* write the handshake request to the socket. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_handshake_request_block(
                    fixture.protosock, &fixture.suite,
                    fixture.authorized_entity_id, &client_key_nonce,
                    &client_challenge_nonce));

    /* This should return successfully. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);

    /* write a garbage packet. */
    TEST_ASSERT(
        54
            == write(
                    fixture.protosock,
                    "test12345678901234567890123456789012345678901234567890",
                    54));

    /* we'll get back an encrypted error response. */
    uint8_t* val;
    uint32_t size;
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == ipc_read_authed_data_block(
                    fixture.protosock, 0x8000000000000001UL, (void**)&val,
                    &size, &fixture.suite, &shared_secret));

    /* the value should not be NULL. */
    TEST_ASSERT(nullptr != val);
    /* the size of the payload should be 12 bytes. */
    TEST_ASSERT(12U == size);

    /* create a response array for convenience. */
    uint32_t* resparr = (uint32_t*)val;

    /* the request ID should be 0, as the request was malformed. */
    TEST_EXPECT(0U == resparr[0]);
    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    TEST_EXPECT(
        AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST
            == (int)ntohl(resparr[1]));
    /* the offset is 0. */
    TEST_EXPECT(0U == resparr[2]);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
    free(val);
END_TEST_F()

/**
 * Test that writing a valid response to the server challenge results in a
 * successful response packet.
 */
BEGIN_TEST_F(handshake_response_happy_path)
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    TEST_ASSERT(fixture.suite_initialized);

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* write the handshake request to the socket. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_handshake_request_block(
                    fixture.protosock, &fixture.suite,
                    fixture.authorized_entity_id, &client_key_nonce,
                    &client_challenge_nonce));

    /* This should return successfully. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_handshake_request_block(
                    fixture.protosock, &fixture.suite, &server_id,
                    &fixture.client_private_key, &server_public_key,
                    &client_key_nonce, &client_challenge_nonce,
                    &server_challenge_nonce, &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    TEST_EXPECT(0U == offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);

    /* send the handshake ack request. */
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_handshake_ack_block(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, &server_challenge_nonce));

    /* receive the handshake ack response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_handshake_ack_block(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* at this point, we have successfully established a secure channel. */

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
END_TEST_F()

/**
 * Test that a request to get the latest block ID returns the latest block ID.
 */
BEGIN_TEST_F(get_latest_block_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xb2, 0xf3, 0xfa, 0x16, 0x75, 0x9f, 0x4d, 0x4a,
        0xaf, 0x6b, 0xf7, 0x68, 0x14, 0x35, 0x7d, 0x21
    };
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the latest block id api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_id_latest_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_latest_block_id_get_block(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the response. */
    vccrypt_buffer_t block_id;
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_latest_block_id_get_block(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, &block_id));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);
    /* the block_id size should be the correct size. */
    TEST_ASSERT(block_id.size == sizeof(EXPECTED_BLOCK_ID));
    /* the block id should match. */
    TEST_ASSERT(0 == memcmp(block_id.data, EXPECTED_BLOCK_ID, block_id.size));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a latest block_id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&block_id);
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that a request to get a block id by height returns that block id.
 */
BEGIN_TEST_F(get_block_id_by_height_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x3d, 0x30, 0x6b, 0x0b, 0x73, 0x1d, 0x4b, 0xe9,
        0x84, 0xda, 0x2a, 0xb8, 0xd7, 0x8f, 0x52, 0x30
    };
    const uint64_t EXPECTED_HEIGHT = 117;
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the latest block id api call. */
    fixture.dataservice->register_callback_block_id_by_height_read(
        [&](const dataservice_request_block_id_by_height_read_t& req,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            if (req.block_height != EXPECTED_HEIGHT)
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;

            int retval =
                dataservice_encode_response_block_id_by_height_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_id_by_height_get_block(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_HEIGHT));

    /* get the response. */
    vccrypt_buffer_t block_id;
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_id_by_height_get_block(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, &block_id));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);
    /* the block_id size should be the correct size. */
    TEST_ASSERT(block_id.size == sizeof(EXPECTED_BLOCK_ID));
    /* the block id should match. */
    TEST_ASSERT(0 == memcmp(block_id.data, EXPECTED_BLOCK_ID, block_id.size));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a latest block_id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_by_height_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_HEIGHT));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&block_id);
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that a request to submit a transaction that is too large fails with an
 * AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION.
 */
BEGIN_TEST_F(transaction_submit_big_certificate)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0x64, 0x91, 0xf1, 0xcf, 0x34, 0xbb, 0x42, 0x15,
        0x9b, 0xc5, 0x49, 0x1e, 0x7a, 0x46, 0xcd, 0x69
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc0, 0x9d, 0x7a, 0xed, 0x7a, 0xef, 0x4b, 0x15,
        0x9a, 0xdd, 0xd2, 0x03, 0x59, 0xbc, 0xc8, 0x3a
    };
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t cert;

    /* create the certificate buffer. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&cert, &fixture.alloc_opts, 32768));
    memset(cert.data, 0xFE, cert.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the submission request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_submit(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TRANSACTION_ID,
                    EXPECTED_ARTIFACT_ID, &cert));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_submit(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status));

    /* the status should indicate failure. */
    TEST_ASSERT(
        AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION == (int)status);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&cert);
END_TEST_F()

/**
 * Test that a request to submit a transaction goes through our mock.
 */
BEGIN_TEST_F(transaction_submit_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0x64, 0x91, 0xf1, 0xcf, 0x34, 0xbb, 0x42, 0x15,
        0x9b, 0xc5, 0x49, 0x1e, 0x7a, 0x46, 0xcd, 0x69
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc0, 0x9d, 0x7a, 0xed, 0x7a, 0xef, 0x4b, 0x15,
        0x9a, 0xdd, 0xd2, 0x03, 0x59, 0xbc, 0xc8, 0x3a
    };
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t cert;

    /* create the certificate buffer. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&cert, &fixture.alloc_opts, 5000));
    memset(cert.data, 0xFE, cert.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the transaction submit api call. */
    fixture.dataservice->register_callback_transaction_submit(
        [&](const dataservice_request_transaction_submit_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the submission request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_submit(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TRANSACTION_ID,
                    EXPECTED_ARTIFACT_ID, &cert));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_submit(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a transaction submit call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_submit(
            fixture.EXPECTED_CHILD_INDEX,
            EXPECTED_TRANSACTION_ID, EXPECTED_ARTIFACT_ID, cert.size,
            (const uint8_t*)cert.data));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&cert);
END_TEST_F()

/**
 * Test that a request to get a block by id passes a failure condition back when
 * the query fails in our data service mock.
 */
BEGIN_TEST_F(block_get_by_id_not_found)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    vccrypt_buffer_t shared_secret;
    data_block_node_t data_block_node;
    uint8_t* block_cert = nullptr;
    size_t block_cert_size = 0UL;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream&) {
            /* block not found. */
            return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, &data_block_node,
                    &block_cert, &block_cert_size));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_ERROR_DATASERVICE_NOT_FOUND == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of block_get_by_id
 */
BEGIN_TEST_F(block_get_by_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    vccrypt_buffer_t shared_secret;
    data_block_node_t data_block_node;
    uint8_t* block_cert = nullptr;
    size_t block_cert_size = 0UL;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID, 10,
                    true, EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, &data_block_node,
                    &block_cert, &block_cert_size));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* in the mock above, we hack in the block id as the certificate. */
    TEST_ASSERT(0 == memcmp(block_cert, EXPECTED_BLOCK_ID, 16));
    TEST_ASSERT(16U == block_cert_size);

    /* clean up memory. */
    free(block_cert);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of block_get_next_id.
 */
BEGIN_TEST_F(block_get_next_id)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_NEXT_BLOCK_ID[16] = {
        0xbd, 0xbc, 0xbd, 0x4a, 0x2d, 0x39, 0x4f, 0x23,
        0xbc, 0xc6, 0xf7, 0xb8, 0x03, 0xa5, 0x7f, 0x6a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, EXPECTED_NEXT_BLOCK_ID,
                    EXPECTED_BLOCK_ID, 10, false, EXPECTED_BLOCK_ID,
                    sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_next_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_next_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, next_id));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* in the mock above, we hack in the next block id. */
    TEST_ASSERT(0 == memcmp(next_id, EXPECTED_NEXT_BLOCK_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that block_get_next_id returns NOT_FOUND if the block id is the end
 * sentry.
 */
BEGIN_TEST_F(block_get_next_id_end)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_NEXT_BLOCK_ID[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, EXPECTED_NEXT_BLOCK_ID,
                    EXPECTED_BLOCK_ID, 10, false, EXPECTED_BLOCK_ID,
                    sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_next_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_next_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, next_id));

    /* the status should indicate failure. */
    TEST_ASSERT(AGENTD_ERROR_DATASERVICE_NOT_FOUND == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of block_get_prev_id.
 */
BEGIN_TEST_F(block_get_prev_id)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_PREV_BLOCK_ID[16] = {
        0x58, 0x73, 0x64, 0xa8, 0x4d, 0x75, 0x41, 0x40,
        0x84, 0x76, 0x9f, 0x4e, 0x12, 0xa4, 0xdb, 0xb0
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_PREV_BLOCK_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, 10, false, EXPECTED_BLOCK_ID,
                    sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_prev_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_prev_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, prev_id));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* in the mock above, we hack in the prev block id. */
    TEST_ASSERT(0 == memcmp(prev_id, EXPECTED_PREV_BLOCK_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that block_get_prev_id returns NOT_FOUND if the block id is the begin
 * sentry.
 */
BEGIN_TEST_F(block_get_prev_id_end)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_PREV_BLOCK_ID[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_PREV_BLOCK_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, 10, false, EXPECTED_BLOCK_ID,
                    sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_block_prev_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_BLOCK_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_block_prev_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, prev_id));

    /* the status should indicate failure. */
    TEST_ASSERT(AGENTD_ERROR_DATASERVICE_NOT_FOUND == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of transaction_get_by_id
 */
BEGIN_TEST_F(txn_get_by_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    vccrypt_buffer_t shared_secret;
    data_transaction_node_t data_txn_node;
    uint8_t* txn_cert = nullptr;
    size_t txn_cert_size = 0UL;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, &data_txn_node, &txn_cert,
                    &txn_cert_size));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* in the mock above, we hack in the txn id as the certificate. */
    TEST_ASSERT(0 == memcmp(txn_cert, EXPECTED_TXN_ID, 16));
    TEST_ASSERT(16U == txn_cert_size);

    /* clean up memory. */
    free(txn_cert);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of transaction_get_next_id.
 */
BEGIN_TEST_F(txn_get_next_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_NEXT_TXN_ID[16] = {
        0xa8, 0x33, 0x7c, 0x29, 0x26, 0xfa, 0x48, 0x4e,
        0x9f, 0x29, 0x6c, 0xe7, 0xb3, 0x3e, 0x4a, 0x65
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_NEXT_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_next_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_next_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, next_id));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* we should get the next txn id. */
    TEST_ASSERT(0 == memcmp(next_id, EXPECTED_NEXT_TXN_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that transaction_get_next_id returns NOT_FOUND if the block id is the
 * end sentry.
 */
BEGIN_TEST_F(txn_get_next_id_end)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_NEXT_TXN_ID[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_NEXT_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_next_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_next_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, next_id));

    /* the status should indicate failure. */
    TEST_ASSERT(AGENTD_ERROR_DATASERVICE_NOT_FOUND == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of transaction_get_prev_id.
 */
BEGIN_TEST_F(txn_get_prev_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_PREV_TXN_ID[16] = {
        0x3d, 0x36, 0x93, 0x5c, 0x9d, 0x8d, 0x49, 0xbe,
        0xab, 0x76, 0xbf, 0xf2, 0x62, 0xe8, 0x53, 0x60
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID,
                    EXPECTED_PREV_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, 10, true, EXPECTED_TXN_ID,
                    sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_prev_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_prev_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, prev_id));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* we should get the prev txn id. */
    TEST_ASSERT(0 == memcmp(prev_id, EXPECTED_PREV_TXN_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test that transaction_get_prev_id returns NOT_FOUND if the block id is the
 * end sentry.
 */
BEGIN_TEST_F(txn_get_prev_id_end)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_PREV_TXN_ID[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID,
                    EXPECTED_PREV_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, 10, true, EXPECTED_TXN_ID,
                    sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_prev_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_prev_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, prev_id));

    /* the status should indicate failure. */
    TEST_ASSERT(AGENTD_ERROR_DATASERVICE_NOT_FOUND == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of transaction_get_block_id.
 */
BEGIN_TEST_F(txn_get_block_id_happy_path)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_BLOCK_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t block_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_BLOCK_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_transaction_block_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_TXN_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_transaction_block_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, block_id));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* we should get the block txn id. */
    TEST_ASSERT(0 == memcmp(block_id, EXPECTED_BLOCK_TXN_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_canonized_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of artifact_get_first_txn_id.
 */
BEGIN_TEST_F(artifact_first_txn_happy)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_FIRST_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t first_txn_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_payload_artifact_read(
        [&](const dataservice_request_payload_artifact_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_payload_artifact_read(
                    &payload, &payload_size, EXPECTED_ARTIFACT_ID,
                    EXPECTED_FIRST_TXN_ID, fixture.zero_uuid, 10, 12, 77);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_artifact_first_txn_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_ARTIFACT_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_artifact_first_txn_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, first_txn_id));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* we should get the first txn id. */
    TEST_ASSERT(0 == memcmp(first_txn_id, EXPECTED_FIRST_TXN_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_payload_artifact_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_ARTIFACT_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the happy path of artifact_get_last_txn_id.
 */
BEGIN_TEST_F(artifact_last_txn_happy)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_LAST_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t last_txn_id[16];

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the block get call. */
    fixture.dataservice->register_callback_payload_artifact_read(
        [&](const dataservice_request_payload_artifact_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_payload_artifact_read(
                    &payload, &payload_size, EXPECTED_ARTIFACT_ID,
                    fixture.zero_uuid, EXPECTED_LAST_TXN_ID, 10, 12, 77);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_artifact_last_txn_id_get(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret, EXPECTED_ARTIFACT_ID));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_artifact_last_txn_id_get(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret, &offset, &status, last_txn_id));

    /* the status should indicate that the record wasn't found. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* we should get the first txn id. */
    TEST_ASSERT(0 == memcmp(last_txn_id, EXPECTED_LAST_TXN_ID, 16));

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_sendreq_close(
                    fixture.protosock, &fixture.suite, &client_iv,
                    &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == protocolservice_api_recvresp_close(
                    fixture.protosock, &fixture.suite, &server_iv,
                    &shared_secret));

    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_payload_artifact_read(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_ARTIFACT_ID));

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * Test the status api method.
 */
BEGIN_TEST_F(status_happy)
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the status get request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_api_sendreq_status_get(
                        fixture.protosock, &fixture.suite, &client_iv,
                        &shared_secret));

    /* get the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_api_recvresp_status_get(
                        fixture.protosock, &fixture.suite, &server_iv,
                        &shared_secret, &offset, &status));

    /* the status should indicate success. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_ASSERT(0U == offset);

    /* send the close request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_api_sendreq_close(
                        fixture.protosock, &fixture.suite, &client_iv,
                        &shared_secret));

    /* get the close response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_api_recvresp_close(
                        fixture.protosock, &fixture.suite, &server_iv,
                        &shared_secret));
 
    /* close the socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify proper connection setup. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_setup());

    /* verify proper connection teardown. */
    TEST_EXPECT(0 == fixture.dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * It is possible to add an authorized entity via the control socket.
 */
BEGIN_TEST_F(ctrl_auth_entity_add)
    uint32_t offset, status;
    const uint8_t entity_id[16] = {
        0xa6, 0xeb, 0x8e, 0x98, 0x5a, 0x84, 0x45, 0x4e,
        0xa2, 0x07, 0x9f, 0x11, 0xbd, 0x36, 0x80, 0x1e };
    vccrypt_buffer_t entity_encryption_key;
    vccrypt_buffer_t entity_signing_key;

    /* create dummy entity encryption key. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_encryption_key, &fixture.alloc_opts, 32));
    memset(entity_encryption_key.data, 0xFF, 32);

    /* create dummy entity signing key. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_signing_key, &fixture.alloc_opts, 32));
    memset(entity_signing_key.data, 0xFF, 32);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* send an authorized entity add request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_control_api_sendreq_authorized_entity_add(
                        fixture.controlsock, fixture.suite.alloc_opts,
                        entity_id, &entity_encryption_key,
                        &entity_signing_key));

    /* read the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_control_api_recvresp_authorized_entity_add(
                        fixture.controlsock, &offset, &status));

    /* the offset should be 0. */
    TEST_EXPECT(0U == offset);
    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);

    /* close the protocol socket */
    close(fixture.protosock);

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&entity_encryption_key);
    dispose((disposable_t*)&entity_signing_key);
END_TEST_F()

/**
 * It is possible to set the protocol service private key.
 */
BEGIN_TEST_F(ctrl_set_private_key)
    uint32_t offset, status;
    const uint8_t entity_id[16] = {
        0xa6, 0xeb, 0x8e, 0x98, 0x5a, 0x84, 0x45, 0x4e,
        0xa2, 0x07, 0x9f, 0x11, 0xbd, 0x36, 0x80, 0x1e };
    vccrypt_buffer_t entity_encryption_pubkey;
    vccrypt_buffer_t entity_encryption_privkey;
    vccrypt_buffer_t entity_signing_pubkey;
    vccrypt_buffer_t entity_signing_privkey;

    /* create dummy entity encryption pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_encryption_pubkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_pubkey.data, 0xFF, 32);

    /* create dummy entity encryption privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_encryption_privkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_privkey.data, 0xFF, 32);

    /* create dummy entity signing pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_signing_pubkey, &fixture.alloc_opts, 32));
    memset(entity_signing_pubkey.data, 0xFF, 32);

    /* create dummy entity signing privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
              == vccrypt_buffer_init(
                        &entity_signing_privkey, &fixture.alloc_opts, 64));
    memset(entity_signing_privkey.data, 0xFF, 64);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* send the private key set request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_control_api_sendreq_private_key_set(
                        fixture.controlsock, fixture.suite.alloc_opts,
                        entity_id, &entity_encryption_pubkey,
                        &entity_encryption_privkey, &entity_signing_pubkey,
                        &entity_signing_privkey));

    /* read the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == protocolservice_control_api_recvresp_private_key_set(
                        fixture.controlsock, &offset, &status));

    /* the offset should be 0. */
    TEST_EXPECT(0U == offset);
    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);

    /* close the protocol socket */
    close(fixture.protosock);

    /* stop the mock. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&entity_encryption_pubkey);
    dispose((disposable_t*)&entity_encryption_privkey);
    dispose((disposable_t*)&entity_signing_pubkey);
    dispose((disposable_t*)&entity_signing_privkey);
END_TEST_F()

/**
 * An assert block request reserves a block assertion in the notification
 * service.
 */
BEGIN_TEST_F(assert_block_happy_path)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    vccrypt_buffer_t shared_secret;
    psock* sock;
    vpr_uuid BLOCK_ID = { .data = {
        0xa8, 0xc1, 0x54, 0x15, 0x9e, 0x3d, 0x40, 0x0a,
        0xa4, 0x1f, 0x06, 0x4b, 0x92, 0xea, 0xea, 0x54 } };
    const uint32_t EXPECTED_OFFSET = 47;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* don't send the response from a block assert. */
    fixture.notifyservice->override_block_assertion_status(true);

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the latest block id assert request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET, &BLOCK_ID));

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify that a block assertion request was sent to the notification
     * service. */
    TEST_EXPECT(
        fixture.notifyservice->request_matches_block_assertion(
            1, (const rcpr_uuid*)&BLOCK_ID));

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * When an invalidation is sent, the client gets a reply from the assert block
 * call. We can simulate this with the mock just by allowing the block assertion
 * status to pass through, which it does by default.
 */
BEGIN_TEST_F(assert_block_invalidation)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    vpr_uuid BLOCK_ID = { .data = {
        0xa8, 0xc1, 0x54, 0x15, 0x9e, 0x3d, 0x40, 0x0a,
        0xa4, 0x1f, 0x06, 0x4b, 0x92, 0xea, 0xea, 0x54 } };
    const uint32_t EXPECTED_OFFSET = 47;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the latest block id assert request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET, &BLOCK_ID));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request_id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify that a block assertion request was sent to the notification
     * service. */
    TEST_EXPECT(
        fixture.notifyservice->request_matches_block_assertion(
            1, (const rcpr_uuid*)&BLOCK_ID));

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * An assert block request fails when the user lacks capabilities to perform a
 * block assertion.
 */
BEGIN_TEST_F(assert_block_capabilities_check)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    vpr_uuid BLOCK_ID = { .data = {
        0xa8, 0xc1, 0x54, 0x15, 0x9e, 0x3d, 0x40, 0x0a,
        0xa4, 0x1f, 0x06, 0x4b, 0x92, 0xea, 0xea, 0x54 } };
    const uint32_t EXPECTED_OFFSET = 47;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* remove the block assertion capability. */
    fixture.entity_caps.erase(fixture.verb_assert_latest_block_id);

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the latest block id assert request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET, &BLOCK_ID));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request_id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID == request_id);
    /* this call was unauthorized. */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * An assert block request can be canceled.
 */
BEGIN_TEST_F(assert_block_cancel_happy_path)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    vpr_uuid BLOCK_ID = { .data = {
        0xa8, 0xc1, 0x54, 0x15, 0x9e, 0x3d, 0x40, 0x0a,
        0xa4, 0x1f, 0x06, 0x4b, 0x92, 0xea, 0xea, 0x54 } };
    const uint32_t EXPECTED_OFFSET = 47;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* don't send the response from a block assert. */
    fixture.notifyservice->override_block_assertion_status(true);

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the latest block id assert request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET, &BLOCK_ID));

    /* cancel this request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id_cancel(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request_id should match what we expect. */
    TEST_EXPECT(
        UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID_CANCEL == request_id);
    /* this call succeeded. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* verify that a block assertion request was sent to the notification
     * service. */
    TEST_EXPECT(
        fixture.notifyservice->request_matches_block_assertion(
            1, (const rcpr_uuid*)&BLOCK_ID));

    /* clean up. */
    dispose((disposable_t*)&response);
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * An assert block request cancellation will fail if unauthorized.
 */
BEGIN_TEST_F(assert_block_cancel_unauthorized)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 47;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* don't send the response from a block assert. */
    fixture.notifyservice->override_block_assertion_status(true);

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* remove the block assertion cancellation capability. */
    fixture.entity_caps.erase(fixture.verb_assert_latest_block_id_cancel);

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* cancel a block assertion request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_assert_latest_block_id_cancel(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request_id should match what we expect. */
    TEST_EXPECT(
        UNAUTH_PROTOCOL_REQ_ID_ASSERT_LATEST_BLOCK_ID_CANCEL == request_id);
    /* this call succeeded. */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&response);
    dispose((disposable_t*)&shared_secret);
END_TEST_F()

/**
 * A Sentinel with the permission to do so can enable an extended API.
 */
BEGIN_TEST_F(extended_api_enable_happy_path)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * An entity that DOES NOT have the extended API enable capability will receive
 * an unauthorized failure when attempting the extended api enable call.
 */
BEGIN_TEST_F(extended_api_enable_unauthorized)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* remove the extended api enable capability. */
    fixture.entity_caps.erase(fixture.verb_sentinel_extend_api_enable);

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    /* however, the call should have failed with an unauthorized status. */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * If an extended api call is made to an unregistered entity, an error is
 * returned.
 */
BEGIN_TEST_F(extended_api_unregistered_entity)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;
    const string sentinel_string = "3361486f-e88d-4c72-a15b-bff22dcdebfd";
    const vpr_uuid sentinel_id = { .data = {
        0x33, 0x61, 0x48, 0x6f, 0xe8, 0x8d, 0x4c, 0x72,
        0xa1, 0x5b, 0xbf, 0xf2, 0x2d, 0xcd, 0xeb, 0xfd } };
    const string verb_string = "55757960-6f0c-41bd-b167-10784e2558af";
    const vpr_uuid verb_id = { .data = {
        0x55, 0x75, 0x79, 0x60, 0x6f, 0x0c, 0x41, 0xbd,
        0xb1, 0x67, 0x10, 0x78, 0x4e, 0x25, 0x58, 0xaf } };
    vccrypt_buffer_t request_body;
    capabilities_entry ext_api_auth = {
        fixture.authorized_entity_id_string, verb_string, sentinel_string };

    /* create dummy request body. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&request_body, &fixture.alloc_opts, 32));
    memset(request_body.data, 0x55, request_body.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* add the ability to perform the requested verb id on this sentinel. */
    fixture.entity_caps.insert(make_pair(verb_string, ext_api_auth));

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));


    /* send an extended api request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET, &sentinel_id, &verb_id, &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV == request_id);
    TEST_EXPECT(
        AGENTD_ERROR_PROTOCOLSERVICE_EXTENDED_API_UNKNOWN_ENTITY == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
    dispose((disposable_t*)&request_body);
END_TEST_F()

/**
 * End-to-end with the extended API works -- one entity sending an extended API
 * call to itself.
 */
BEGIN_TEST_F(extended_api_e2e)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;
    const string verb_string = "55757960-6f0c-41bd-b167-10784e2558af";
    const vpr_uuid verb_id = { .data = {
        0x55, 0x75, 0x79, 0x60, 0x6f, 0x0c, 0x41, 0xbd,
        0xb1, 0x67, 0x10, 0x78, 0x4e, 0x25, 0x58, 0xaf } };
    vccrypt_buffer_t request_body;
    protocol_resp_extended_api_client_request client_resp;
    capabilities_entry ext_api_auth = {
        fixture.authorized_entity_id_string, verb_string,
        fixture.authorized_entity_id_string };

    /* create dummy request body. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&request_body, &fixture.alloc_opts, 32));
    memset(request_body.data, 0x55, request_body.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* remove the extended api response capability. */
    fixture.entity_caps.erase(fixture.verb_extended_api_resp);

    /* add the ability to perform the requested verb id on this entity. */
    fixture.entity_caps.insert(make_pair(verb_string, ext_api_auth));

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up response. */
    dispose((disposable_t*)&response);

    /* send an extended API request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET,
                    (const vpr_uuid*)fixture.authorized_entity_id, &verb_id,
                    &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be the client request. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_CLIENTREQ == request_id);

    /* decode the client request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_decode_resp_extended_api_client_request(
                    &client_resp, &fixture.alloc_opts, response.data,
                    response.size));

    /* the request id should be valid. */
    TEST_EXPECT(
        UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_CLIENTREQ
            == client_resp.request_id);
    /* the offset should be what we expect. */
    TEST_EXPECT(1UL == client_resp.offset);
    /* the client id should match our authorized id. */
    TEST_EXPECT(
        0 == memcmp(fixture.authorized_entity_id, &client_resp.client_id, 16));
    /* the verb id should match. */
    TEST_EXPECT(0 == memcmp(&verb_id, &client_resp.verb_id, 16));
    /* the client encryption pubkey size should match. */
    TEST_ASSERT(
        sizeof(fixture.authorized_entity_enc_pubkey_buffer)
            == client_resp.client_enc_pubkey.size);
    /* the client encryption pubkey should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    fixture.authorized_entity_enc_pubkey_buffer,
                    client_resp.client_enc_pubkey.data,
                    client_resp.client_enc_pubkey.size));
    /* the client signing pubkey size should match. */
    TEST_ASSERT(
        sizeof(fixture.authorized_entity_sign_pubkey_buffer)
            == client_resp.client_sign_pubkey.size);
    /* the client signing pubkey should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    fixture.authorized_entity_sign_pubkey_buffer,
                    client_resp.client_sign_pubkey.data,
                    client_resp.client_sign_pubkey.size));
    /* the request body size should match. */
    TEST_ASSERT(request_body.size == client_resp.request_body.size);
    /* the request body should match. */
    TEST_EXPECT(
        0
            == memcmp(
                    request_body.data, client_resp.request_body.data,
                    request_body.size));

    /* clean up the response. */
    dispose((disposable_t*)&response);

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be the extended api send response. */
    TEST_EXPECT(
        UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV == request_id);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&client_resp);
    dispose((disposable_t*)&request_body);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * End-to-end with the extended API works -- one entity sending an extended API
 * call to itself, and it responds to this call.
 */
BEGIN_TEST_F(extended_api_e2e2)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;
    const uint32_t EXPECTED_RESPONSE_STATUS = 27;
    const string verb_string = "55757960-6f0c-41bd-b167-10784e2558af";
    const vpr_uuid verb_id = { .data = {
        0x55, 0x75, 0x79, 0x60, 0x6f, 0x0c, 0x41, 0xbd,
        0xb1, 0x67, 0x10, 0x78, 0x4e, 0x25, 0x58, 0xaf } };
    vccrypt_buffer_t request_body;
    protocol_resp_extended_api_client_request client_resp;
    protocol_resp_extended_api extresp;
    capabilities_entry ext_api_auth = {
        fixture.authorized_entity_id_string, verb_string,
        fixture.authorized_entity_id_string };

    /* create dummy request body. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&request_body, &fixture.alloc_opts, 32));
    memset(request_body.data, 0x55, request_body.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* add the ability to perform the requested verb id on this entity. */
    fixture.entity_caps.insert(make_pair(verb_string, ext_api_auth));

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up response. */
    dispose((disposable_t*)&response);

    /* send an extended API request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET,
                    (const vpr_uuid*)fixture.authorized_entity_id, &verb_id,
                    &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be the client request. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_CLIENTREQ == request_id);

    /* decode the client request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_decode_resp_extended_api_client_request(
                    &client_resp, &fixture.alloc_opts, response.data,
                    response.size));

    /* the request id should be valid. */
    TEST_EXPECT(
        UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_CLIENTREQ
            == client_resp.request_id);
    /* the offset should be what we expect. */
    TEST_EXPECT(1UL == client_resp.offset);
    /* the client id should match our authorized id. */
    TEST_EXPECT(
        0 == memcmp(fixture.authorized_entity_id, &client_resp.client_id, 16));
    /* the verb id should match. */
    TEST_EXPECT(0 == memcmp(&verb_id, &client_resp.verb_id, 16));
    /* the client encryption pubkey size should match. */
    TEST_ASSERT(
        sizeof(fixture.authorized_entity_enc_pubkey_buffer)
            == client_resp.client_enc_pubkey.size);
    /* the client encryption pubkey should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    fixture.authorized_entity_enc_pubkey_buffer,
                    client_resp.client_enc_pubkey.data,
                    client_resp.client_enc_pubkey.size));
    /* the client signing pubkey size should match. */
    TEST_ASSERT(
        sizeof(fixture.authorized_entity_sign_pubkey_buffer)
            == client_resp.client_sign_pubkey.size);
    /* the client signing pubkey should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    fixture.authorized_entity_sign_pubkey_buffer,
                    client_resp.client_sign_pubkey.data,
                    client_resp.client_sign_pubkey.size));
    /* the request body size should match. */
    TEST_ASSERT(request_body.size == client_resp.request_body.size);
    /* the request body should match. */
    TEST_EXPECT(
        0
            == memcmp(
                    request_body.data, client_resp.request_body.data,
                    request_body.size));

    /* clean up the response. */
    dispose((disposable_t*)&response);

    /* send the response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_response(
                    sock, &fixture.suite, &client_iv, &shared_secret, 1UL,
                    EXPECTED_RESPONSE_STATUS, &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be the extended api send response. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV == request_id);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(EXPECTED_RESPONSE_STATUS == status);

    /* decode the body. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_decode_resp_extended_api(
                    &extresp, &fixture.alloc_opts, response.data,
                    response.size));

    /* the request body should match. */
    TEST_ASSERT(request_body.size == extresp.response_body.size);
    TEST_EXPECT(
        0
            == memcmp(
                    request_body.data, extresp.response_body.data,
                    request_body.size));

    /* clean up the response. */
    dispose((disposable_t*)&response);

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be the extended api response send response. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRESP == request_id);
    TEST_EXPECT(1U == offset);
    TEST_EXPECT(STATUS_SUCCESS == status);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&client_resp);
    dispose((disposable_t*)&request_body);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&extresp);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * An unauthorized error is returned if a client attempts to perform an extended
 * api request without permission.
 */
BEGIN_TEST_F(extended_api_req_unauthorized)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;
    const vpr_uuid verb_id = { .data = {
        0x55, 0x75, 0x79, 0x60, 0x6f, 0x0c, 0x41, 0xbd,
        0xb1, 0x67, 0x10, 0x78, 0x4e, 0x25, 0x58, 0xaf } };
    vccrypt_buffer_t request_body;

    /* create dummy request body. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&request_body, &fixture.alloc_opts, 32));
    memset(request_body.data, 0x55, request_body.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* remove the extended api request capability. */
    fixture.entity_caps.erase(fixture.verb_extended_api_req);

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up response. */
    dispose((disposable_t*)&response);

    /* send an extended API request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET,
                    (const vpr_uuid*)fixture.authorized_entity_id, &verb_id,
                    &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be a sendrecv request. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV == request_id);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    /* it should have failed with an unauthorized error. */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == status);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&request_body);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()

/**
 * An unauthorized error is returned if a client attempts to perform an extended
 * api request without explicit permission for that entity and verb.
 */
BEGIN_TEST_F(extended_api_req_unauthorized2)
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    uint32_t request_id, offset, status;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t response;
    psock* sock;
    const uint32_t EXPECTED_OFFSET = 147;
    const vpr_uuid verb_id = { .data = {
        0x55, 0x75, 0x79, 0x60, 0x6f, 0x0c, 0x41, 0xbd,
        0xb1, 0x67, 0x10, 0x78, 0x4e, 0x25, 0x58, 0xaf } };
    vccrypt_buffer_t request_body;

    /* create dummy request body. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(&request_body, &fixture.alloc_opts, 32));
    memset(request_body.data, 0x55, request_body.size);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notifyservice->start();

    /* add the hardcoded keys. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == fixture.add_hardcoded_keys());

    /* do the handshake, populating the shared secret on success. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
              == fixture.do_handshake(&shared_secret, &server_iv, &client_iv));

    /* convert our socket to a psock instance to call the extended API. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_descriptor(
                    &sock, fixture.alloc, fixture.protosock));

    /* send the extended api enable request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api_enable(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* the request id should match what we expect. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_ENABLE == request_id);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up response. */
    dispose((disposable_t*)&response);

    /* send an extended API request. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_sendreq_extended_api(
                    sock, &fixture.suite, &client_iv, &shared_secret,
                    EXPECTED_OFFSET,
                    (const vpr_uuid*)fixture.authorized_entity_id, &verb_id,
                    &request_body));

    /* we should receive a response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_recvresp(
                    sock, fixture.alloc, &fixture.suite, &server_iv,
                    &shared_secret, &response));

    /* we should be able to decode this response. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == vcblockchain_protocol_response_decode_header(
                    &request_id, &offset, &status, &response));

    /* it should be a sendrecv request. */
    TEST_EXPECT(UNAUTH_PROTOCOL_REQ_ID_EXTENDED_API_SENDRECV == request_id);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    /* it should have failed with an unauthorized error. */
    TEST_EXPECT(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED == status);

    /* release the socket instance. */
    TEST_ASSERT(
        STATUS_SUCCESS == resource_release(psock_resource_handle(sock)));

    /* stop the mocks. */
    fixture.dataservice->stop();
    fixture.notifyservice->stop();

    /* clean up. */
    dispose((disposable_t*)&request_body);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&response);
END_TEST_F()
