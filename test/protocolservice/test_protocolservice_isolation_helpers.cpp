/**
 * \file test_protocolservice_isolation_helpers.cpp
 *
 * Helpers for the protocol service isolation test.
 *
 * \copyright 2021 Velo-Payments, Inc.  All rights reserved.
 */

#include <algorithm>
#include <agentd/protocolservice.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <rcpr/uuid.h>
#include <signal.h>
#include <sys/wait.h>
#include <vpr/allocator/malloc_allocator.h>

#include "test_protocolservice_isolation.h"

using namespace std;
RCPR_IMPORT_uuid;

const uint8_t protocolservice_isolation_test::dir_key[32] = {
    0x7e, 0x4b, 0xb1, 0x5d, 0xb5, 0x00, 0x41, 0x95,
    0xb0, 0xed, 0x43, 0x59, 0x43, 0x20, 0x9b, 0x72,
    0x28, 0x07, 0xad, 0xbb, 0x87, 0x70, 0x49, 0x8a,
    0xac, 0x89, 0x44, 0xcb, 0x23, 0x56, 0x67, 0x3f };

const string
    protocolservice_isolation_test::authorized_entity_id_string(
    "6c362b3e-9081-4fcb-80fe-16354e0ae28f");

const uint8_t
    protocolservice_isolation_test::authorized_entity_id[16] = {
        0x6c, 0x36, 0x2b, 0x3e, 0x90, 0x81, 0x4f, 0xcb,
        0x80, 0xfe, 0x16, 0x35, 0x4e, 0x0a, 0xe2, 0x8f };

const uint8_t
protocolservice_isolation_test::
authorized_entity_enc_privkey_buffer[32] = {
        0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
        0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
        0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
        0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a };

const uint8_t
protocolservice_isolation_test::
authorized_entity_enc_pubkey_buffer[32] = {
        0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
        0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
        0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
        0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a };

const uint8_t
protocolservice_isolation_test::
authorized_entity_sign_privkey_buffer[64] = {
        0x8a, 0x8f, 0xba, 0x09, 0xd4, 0xa7, 0xd6, 0x16,
        0x9b, 0x2a, 0xf6, 0xc2, 0x79, 0x69, 0xf7, 0x05,
        0xeb, 0x7a, 0x68, 0x53, 0xb6, 0x46, 0xa2, 0xec,
        0x8d, 0x75, 0x26, 0xa8, 0x0d, 0x86, 0x6b, 0x2d,
        0x99, 0xc8, 0x12, 0x1a, 0x69, 0xbb, 0x8e, 0x32,
        0x9f, 0xf6, 0xc6, 0xcd, 0x5d, 0x48, 0x7e, 0x47,
        0x3e, 0xb1, 0xbf, 0x04, 0xbf, 0xdf, 0x30, 0xcb,
        0x57, 0xf2, 0xdb, 0xe0, 0x93, 0xeb, 0xa5, 0x14 };

const uint8_t
protocolservice_isolation_test::
authorized_entity_sign_pubkey_buffer[32] = {
        0x99, 0xc8, 0x12, 0x1a, 0x69, 0xbb, 0x8e, 0x32,
        0x9f, 0xf6, 0xc6, 0xcd, 0x5d, 0x48, 0x7e, 0x47,
        0x3e, 0xb1, 0xbf, 0x04, 0xbf, 0xdf, 0x30, 0xcb,
        0x57, 0xf2, 0xdb, 0xe0, 0x93, 0xeb, 0xa5, 0x14 };

const string
    protocolservice_isolation_test::agent_id_string(
    "3d963f54-83e2-4b0d-86a1-81b6aaaa5c1b");

const uint8_t protocolservice_isolation_test::agent_id[16] = {
    0x3d, 0x96, 0x3f, 0x54, 0x83, 0xe2, 0x4b, 0x0d,
    0x86, 0xa1, 0x81, 0xb6, 0xaa, 0xaa, 0x5c, 0x1b };

const uint8_t
protocolservice_isolation_test::agent_enc_pubkey_buffer[32] = {
    0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
    0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
    0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
    0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f };

const uint8_t
protocolservice_isolation_test::agent_enc_privkey_buffer[32] = {
    0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
    0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
    0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
    0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb };

const uint8_t
protocolservice_isolation_test::agent_sign_pubkey_buffer[32] = {
    0x3b, 0xcb, 0xc2, 0xdc, 0x1e, 0xed, 0x49, 0xa4,
    0x99, 0x0a, 0x12, 0xe8, 0x73, 0x79, 0xa0, 0x64,
    0xeb, 0x20, 0xc7, 0xe8, 0x16, 0x7d, 0x9e, 0x82,
    0xa3, 0xf0, 0x1e, 0x34, 0x36, 0x23, 0x9e, 0x2a };

const uint8_t
protocolservice_isolation_test::agent_sign_privkey_buffer[64] = {
    0x01, 0xa8, 0xc4, 0xe2, 0xcf, 0x41, 0xd2, 0x4f,
    0x80, 0x43, 0x14, 0xc8, 0xc2, 0x4a, 0x46, 0xc4,
    0xb1, 0x31, 0x74, 0xc3, 0x0d, 0xcd, 0xe0, 0x80,
    0xd8, 0x2d, 0x87, 0x75, 0xc1, 0x74, 0x47, 0xf3,
    0x3b, 0xcb, 0xc2, 0xdc, 0x1e, 0xed, 0x49, 0xa4,
    0x99, 0x0a, 0x12, 0xe8, 0x73, 0x79, 0xa0, 0x64,
    0xeb, 0x20, 0xc7, 0xe8, 0x16, 0x7d, 0x9e, 0x82,
    0xa3, 0xf0, 0x1e, 0x34, 0x36, 0x23, 0x9e, 0x2a };

const uint32_t
    protocolservice_isolation_test::EXPECTED_CHILD_INDEX = 17U;

const string protocolservice_isolation_test::blank_uuid(
    "00000000-0000-0000-0000-000000000000");
const string protocolservice_isolation_test::verb_latest_block_id_get(
    "c5b0eb04-6b24-48be-b7d9-bf9083a4be5d");
const string protocolservice_isolation_test::verb_block_id_by_height_get(
    "915a5ef4-8f96-4ef5-9588-0a75b1cae68d");
const string protocolservice_isolation_test::verb_block_get(
    "f382e365-1224-43b4-924a-1de4d9f4cf25");
const string protocolservice_isolation_test::verb_transaction_get(
    "7df210d6-f00b-47c4-a608-6f3f1df7511a");
const string protocolservice_isolation_test::verb_transaction_submit(
    "ef560d24-eea6-4847-9009-464b127f249b");
const string protocolservice_isolation_test::verb_artifact_get(
    "fc0e22ea-1e77-4ea4-a2ae-08be5ff73ccc");
const string protocolservice_isolation_test::verb_assert_latest_block_id(
    "447617b4-a847-437c-b62b-5bc6a94206fa");
const string protocolservice_isolation_test::verb_assert_latest_block_id_cancel(
    "d848b118-7c34-46c5-80db-d4ffd921bb50");
const string protocolservice_isolation_test::verb_sentinel_extend_api(
    "c41b053c-6b4a-40a1-981b-882bdeffe978");

const capabilities_map protocolservice_isolation_test::global_caps{
    {   protocolservice_isolation_test::verb_latest_block_id_get,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_latest_block_id_get,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_block_id_by_height_get,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_block_id_by_height_get,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_block_get,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_block_get,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_transaction_get,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_transaction_get,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_transaction_submit,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_transaction_submit,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_artifact_get,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_artifact_get,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_assert_latest_block_id,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_assert_latest_block_id,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_assert_latest_block_id_cancel,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_assert_latest_block_id_cancel,
            protocolservice_isolation_test::blank_uuid } },
    {   protocolservice_isolation_test::verb_sentinel_extend_api,
        {   protocolservice_isolation_test::blank_uuid,
            protocolservice_isolation_test::verb_sentinel_extend_api,
            protocolservice_isolation_test::blank_uuid } } };

void protocolservice_isolation_test::SetUp()
{
    vccrypt_suite_register_velo_v1();

    /* initialize allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_suite_options_init(
            &suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        suite_instance_initialized = true;
    }
    else
    {
        suite_instance_initialized = false;
    }

    /* set up the client private key. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_buffer_init(
            &client_private_key, &alloc_opts,
            sizeof(authorized_entity_enc_privkey_buffer)))
    {
        memcpy(
            client_private_key.data, authorized_entity_enc_privkey_buffer,
            client_private_key.size);
        client_private_key_initialized = true;
    }
    else
    {
        client_private_key_initialized = false;
    }

    if (suite_instance_initialized && client_private_key_initialized)
    {
        suite_initialized = true;
    }

    /* set the path for running agentd. */
    const char* agentd_path = getenv("AGENTD_PATH");
    if (NULL != agentd_path)
    {
        strcpy(wd, agentd_path);
        oldpath = getenv("PATH");
        if (NULL != oldpath)
        {
            path =
                strcatv(wd, ":", oldpath, NULL);
        }
        else
        {
            path = strcatv(wd, NULL);
        }
    }

    setenv("PATH", path, 1);

    /* log to standard error. */
    logsock = dup(STDERR_FILENO);
    rlogsock = dup(STDERR_FILENO);

    /* create the socket pair for the datasock. */
    int datasock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &datasock, &datasock_srv);

    /* create the socket pair for the acceptsock. */
    int acceptsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_DGRAM, 0, &acceptsock, &acceptsock_srv);

    /* create the socket pair for the controlsock. */
    int controlsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &controlsock, &controlsock_srv);

    /* create the socket pair for the notifysock. */
    int notifysock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &notifysock, &notifysock_srv);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* spawn the random service process. */
    random_proc_status =
        randomservice_proc(
            &bconf, &conf, &rlogsock, &rprotosock, &randompid, false);

    /* spawn the unauthorized protocol service process. */
    proto_proc_status =
        protocolservice_proc(
            &bconf, &conf, rprotosock, logsock, acceptsock_srv, controlsock_srv,
            datasock_srv, notifysock_srv, &protopid, false);

    /* create the mock dataservice. */
    dataservice = make_unique<mock_dataservice::mock_dataservice>(datasock);

    /* create the mock notificationservice. */
    notifyservice =
        make_unique<mock_notificationservice::mock_notificationservice>(
            notifysock);

    /* if the spawn is successful, send the service the other half of a protocol
     * socket. */
    if (0 == proto_proc_status)
    {
        int protosock_srv;
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &protosock, &protosock_srv);
        ipc_sendsocket_block(acceptsock, protosock_srv);
        close(protosock_srv);
    }

    /* transform the global caps into local entity caps. */
    transform(
        global_caps.begin(), global_caps.end(),
        inserter(entity_caps, entity_caps.end()),
        [this](const pair<string, capabilities_entry>& e) {
            return make_pair(
                e.first,
                make_tuple(
                    authorized_entity_id_string,
                    get<cap_verb>(e.second),
                    agent_id_string)); });

    /* set up directory test helper. */
    string dbpath = "build/test/isolation/databases/";
    directory_test_helper::SetUp(dir_key, dbpath.c_str());
}

void protocolservice_isolation_test::TearDown()
{
    directory_test_helper::TearDown();

    /* terminate the random service. */
    if (0 == random_proc_status)
    {
        int status = 0;
        kill(randompid, SIGTERM);
        waitpid(randompid, &status, 0);
    }

    /* terminate the unauthorized protocol service process. */
    if (0 == proto_proc_status)
    {
        int status = 0;
        close(protosock);
        kill(protopid, SIGTERM);
        waitpid(protopid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dataservice->stop();
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    close(logsock);
    if (rlogsock >= 0)
        close(rlogsock);
    close(datasock);
    close(acceptsock);
    close(controlsock);
    close(notifysock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    if (client_private_key_initialized)
    {
        dispose((disposable_t*)&client_private_key);
    }
    dispose((disposable_t*)&alloc_opts);
}

/** \brief Helper to perform handshake, returning the shared secret. */
int protocolservice_isolation_test::do_handshake(
    vccrypt_buffer_t* shared_secret, uint64_t* server_iv,
    uint64_t* client_iv)
{
    int retval = 0;
    uint32_t offset, status;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    if (!suite_initialized)
    {
        retval = 1;
        goto done;
    }

    /* set the client and server IVs to sane start values. */
    *server_iv = *client_iv = 0UL;

    /* attempt to send the handshake request. */
    retval =
        protocolservice_api_sendreq_handshake_request_block(
            protosock, &suite, authorized_entity_id, &client_key_nonce,
            &client_challenge_nonce);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* attempt to read the handshake response. */
    retval =
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce, &client_challenge_nonce,
            &server_challenge_nonce, shared_secret, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval || AGENTD_STATUS_SUCCESS != (int)status)
    {
        if (AGENTD_STATUS_SUCCESS == retval)
            retval = (int)status;
        goto cleanup_nonces;
    }

    /* attempt to send the handshake ack request. */
    retval =
        protocolservice_api_sendreq_handshake_ack_block(
            protosock, &suite, client_iv, shared_secret,
            &server_challenge_nonce);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_request_buffers_on_fail;
    }

    /* receive the handshake ack response. */
    retval =
        protocolservice_api_recvresp_handshake_ack_block(
            protosock, &suite, server_iv, shared_secret, &offset, &status);

    /* use the status if I/O completed successfully. */
    if (AGENTD_STATUS_SUCCESS == retval)
        retval = (int)status;

    /* if the remote call failed, clean up everything. */
    if (AGENTD_STATUS_SUCCESS != retval)
        goto cleanup_request_buffers_on_fail;

    /* on success, clean up only the buffers we don't return to the caller. */
    goto cleanup_request_buffers_on_success;

cleanup_request_buffers_on_fail:
    dispose((disposable_t*)shared_secret);

cleanup_request_buffers_on_success:
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&server_challenge_nonce);

cleanup_nonces:
    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);

done:
    return retval;
}

int protocolservice_isolation_test::dataservice_mock_register_helper()
{
    /* mock the child context create call. */
    dataservice->register_callback_child_context_create(
        [&](const dataservice_request_child_context_create_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_child_context_create(
                    &payload, &payload_size, EXPECTED_CHILD_INDEX);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the child context close call. */
    dataservice->register_callback_child_context_close(
        [&](const dataservice_request_child_context_close_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    return 0;
}

int protocolservice_isolation_test::
    dataservice_mock_valid_connection_setup()
{
    /* a child context should have been created. */
    BITCAP(testbits, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(testbits);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    BITCAP_SET_TRUE(testbits, DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);
    if (!dataservice->request_matches_child_context_create(testbits))
    {
        return 1;
    }

    return 0;
}

int protocolservice_isolation_test::
    dataservice_mock_valid_connection_teardown()
{
    /* the child index should have been closed. */
    if (!dataservice->request_matches_child_context_close(EXPECTED_CHILD_INDEX))
        return 1;

    return 0;
}

int protocolservice_isolation_test::add_hardcoded_keys()
{
    int retval;
    uint32_t offset, status;
    vccrypt_buffer_t agent_enc_pubkey;
    vccrypt_buffer_t agent_enc_privkey;
    vccrypt_buffer_t agent_sign_pubkey;
    vccrypt_buffer_t agent_sign_privkey;
    vccrypt_buffer_t entity_enc_pubkey;
    vccrypt_buffer_t entity_sign_pubkey;

    /* initialize agent_enc_pubkey. */
    retval =
        vccrypt_buffer_init(
            &agent_enc_pubkey, suite.alloc_opts,
            sizeof(agent_enc_pubkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* copy the encryption pubkey. */
    memcpy(
        agent_enc_pubkey.data, agent_enc_pubkey_buffer, agent_enc_pubkey.size);

    /* initialize agent_enc_privkey. */
    retval =
        vccrypt_buffer_init(
            &agent_enc_privkey, suite.alloc_opts,
            sizeof(agent_enc_privkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agent_enc_pubkey;
    }

    /* copy the encryption privkey. */
    memcpy(
        agent_enc_privkey.data, agent_enc_privkey_buffer,
        agent_enc_privkey.size);

    /* initialize agent_sign_pubkey. */
    retval =
        vccrypt_buffer_init(
            &agent_sign_pubkey, suite.alloc_opts,
            sizeof(agent_sign_pubkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agent_enc_privkey;
    }

    /* copy the signature pubkey. */
    memcpy(
        agent_sign_pubkey.data, agent_sign_pubkey_buffer,
        agent_sign_pubkey.size);

    /* initialize agent_sign_privkey. */
    retval =
        vccrypt_buffer_init(
            &agent_sign_privkey, suite.alloc_opts,
            sizeof(agent_sign_privkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agent_sign_pubkey;
    }

    /* copy the signature privkey. */
    memcpy(
        agent_sign_privkey.data, agent_sign_privkey_buffer,
        agent_sign_privkey.size);

    /* initialize entity_enc_pubkey. */
    retval =
        vccrypt_buffer_init(
            &entity_enc_pubkey, suite.alloc_opts,
            sizeof(authorized_entity_enc_pubkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agent_sign_privkey;
    }

    /* copy the entity encryption pubkey. */
    memcpy(
        entity_enc_pubkey.data, authorized_entity_enc_pubkey_buffer,
        entity_enc_pubkey.size);

    /* initialize entity_sign_pubkey. */
    retval =
        vccrypt_buffer_init(
            &entity_sign_pubkey, suite.alloc_opts,
            sizeof(authorized_entity_sign_pubkey_buffer));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_enc_pubkey;
    }

    /* copy the entity signature pubkey. */
    memcpy(
        entity_sign_pubkey.data, authorized_entity_sign_pubkey_buffer,
        entity_sign_pubkey.size);

    /* send the private key request. */
    retval =
        protocolservice_control_api_sendreq_private_key_set(
            controlsock, suite.alloc_opts, agent_id, &agent_enc_pubkey,
            &agent_enc_privkey, &agent_sign_pubkey, &agent_sign_privkey);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_sign_pubkey;
    }

    /* receive the private key response. */
    retval =
        protocolservice_control_api_recvresp_private_key_set(
            controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_sign_pubkey;
    }

    /* verify that the key was set. */
    if (AGENTD_STATUS_SUCCESS != (int)status)
    {
        retval = (int)status;
        goto cleanup_entity_sign_pubkey;
    }

    /* send the authorized entity add request. */
    retval =
        protocolservice_control_api_sendreq_authorized_entity_add(
            controlsock, suite.alloc_opts, authorized_entity_id,
            &entity_enc_pubkey, &entity_sign_pubkey);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_sign_pubkey;
    }

    /* receive the authorized entity add response. */
    retval =
        protocolservice_control_api_recvresp_authorized_entity_add(
            controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_sign_pubkey;
    }

    /* verify that the authorized entity was added. */
    if (AGENTD_STATUS_SUCCESS != (int)status)
    {
        retval = (int)status;
        goto cleanup_entity_sign_pubkey;
    }

    /* iterate through all capabilities, adding them to this entity. */
    for (auto e : entity_caps)
    {
        rcpr_uuid entity_id;
        rcpr_uuid subject_id;
        rcpr_uuid verb_id;
        rcpr_uuid object_id;

        /* attempt to parse the entity id. */
        retval =
            rcpr_uuid_parse_string(
                &entity_id, get<cap_subject>(e.second).c_str());
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* attempt to parse the subject id. */
        retval =
            rcpr_uuid_parse_string(
                &subject_id, get<cap_subject>(e.second).c_str());
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* attempt to parse the verb id. */
        retval =
            rcpr_uuid_parse_string(
                &verb_id, get<cap_verb>(e.second).c_str());
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* attempt to parse the object id. */
        retval =
            rcpr_uuid_parse_string(
                &object_id, get<cap_object>(e.second).c_str());
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* add this capability to the authorized entity. */
        retval =
            protocolservice_control_api_sendreq_authorized_entity_capability_add(
                controlsock, suite.alloc_opts, (const uint8_t*)&entity_id,
                (const uint8_t*)&subject_id, (const uint8_t*)&verb_id,
                (const uint8_t*)&object_id);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* receive a response from the cap add call. */
        retval =
            protocolservice_control_api_recvresp_authorized_entity_capability_add(
                controlsock, &offset, &status);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_entity_sign_pubkey;
        }

        /* verify that the authorized entity capability was added. */
        if (AGENTD_STATUS_SUCCESS != (int)status)
        {
            retval = (int)status;
            goto cleanup_entity_sign_pubkey;
        }
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_entity_sign_pubkey;

cleanup_entity_sign_pubkey:
    dispose((disposable_t*)&entity_sign_pubkey);

cleanup_entity_enc_pubkey:
    dispose((disposable_t*)&entity_enc_pubkey);

cleanup_agent_sign_privkey:
    dispose((disposable_t*)&agent_sign_privkey);

cleanup_agent_sign_pubkey:
    dispose((disposable_t*)&agent_sign_pubkey);

cleanup_agent_enc_privkey:
    dispose((disposable_t*)&agent_enc_privkey);

cleanup_agent_enc_pubkey:
    dispose((disposable_t*)&agent_enc_pubkey);

done:
    return retval;
}
