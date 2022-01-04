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

#endif /* defined(AGENTD_NEW_PROTOCOL) */
