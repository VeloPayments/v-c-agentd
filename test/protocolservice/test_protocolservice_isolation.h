/**
 * \file test_protocolservice_isolation.h
 *
 * Private header for the protocol service isolation tests.
 *
 * \copyright 2021 Velo-Payments, Inc.  All rights reserved.
 */

#pragma once

#include "../directory_test_helper.h"
#include "../mocks/dataservice.h"
#include <agentd/config.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/string.h>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <vpr/disposable.h>

extern "C" {
#include "agentd.tab.h"
#include "agentd.yy.h"
}

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

/**
 * The protocol service isolation test class deals with the drudgery of
 * communicating with the protocol service.  It provides a registration
 * mechanism so that data can be sent to the service and received from the
 * service.
 */
class protocolservice_isolation_test :
    public ::testing::Test, public directory_test_helper
{
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    bootstrap_config_t bconf;
    agent_config_t conf;
    int acceptsock;
    int controlsock;
    int datasock;
    int logsock;
    int protosock;
    int rlogsock;
    int rprotosock;
    pid_t protopid;
    pid_t randompid;
    int proto_proc_status;
    int random_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    bool suite_instance_initialized;
    bool suite_initialized;
    vccrypt_buffer_t client_private_key;
    bool client_private_key_initialized;
    std::unique_ptr<mock_dataservice::mock_dataservice> dataservice;

    static const uint8_t dir_key[32];
    static const uint8_t authorized_entity_id[16];
    static const uint8_t authorized_entity_enc_pubkey_buffer[32];
    static const uint8_t authorized_entity_enc_privkey_buffer[32];
    static const uint8_t authorized_entity_sign_pubkey_buffer[32];
    static const uint8_t authorized_entity_sign_privkey_buffer[64];
    static const uint8_t agent_id[16];
    static const uint8_t agent_enc_pubkey_buffer[32];
    static const uint8_t agent_enc_privkey_buffer[32];
    static const uint8_t agent_sign_pubkey_buffer[32];
    static const uint8_t agent_sign_privkey_buffer[64];

    static const uint32_t EXPECTED_CHILD_INDEX;

    /** \brief Helper to perform handshake, returning the shared secret. */
    int do_handshake(
        vccrypt_buffer_t* shared_secret, uint64_t* server_iv,
        uint64_t* client_iv);

    /** \brief Helper to register dataservice boilerplate methods. */
    int dataservice_mock_register_helper();

    /** \brief Helper to verify dataservice calls on connection setup. */
    int dataservice_mock_valid_connection_setup();

    /** \brief Helper to verify dataservice calls on connection teardown. */
    int dataservice_mock_valid_connection_teardown();

    /** \brief Add hardcoded keys to the protocol service. */
    int add_hardcoded_keys();
};
