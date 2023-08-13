/**
 * \file test_authservice_isolation.cpp
 *
 * Isolation tests for the auth service.
 *
 * \copyright 2019-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice/api.h>
#include <agentd/authservice/private/authservice.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>
#include <rcpr/status.h>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_auth_service_isolation.h"

using namespace std;

TEST_SUITE(auth_service_isolation_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    auth_service_isolation_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the auth service.
 */
BEGIN_TEST_F(simple_spawn)
    TEST_ASSERT(0 == fixture.auth_service_proc_status);
END_TEST_F()

/**
 * Test that we can initialize the auth service using BLOCKING calls.
 */
BEGIN_TEST_F(initialize_blocking)
    TEST_ASSERT(0 == fixture.auth_service_proc_status);

    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* initialize the agent ID buffer  */
    vccrypt_buffer_t agent_id_buffer;
    size_t sz_agent_id = sizeof(fixture.agent_id);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &agent_id_buffer, &fixture.alloc_opts, sz_agent_id));
    memcpy(agent_id_buffer.data, fixture.agent_id, sz_agent_id);

    /* initialize the public key buffer */
    vccrypt_buffer_t pubkey_buffer;
    size_t sz_pubkey = sizeof(fixture.agent_pubkey);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &pubkey_buffer, &fixture.alloc_opts, sz_pubkey));
    memcpy(pubkey_buffer.data, fixture.agent_pubkey, sz_pubkey);

    /* initialize the private key buffer */
    vccrypt_buffer_t privkey_buffer;
    size_t sz_privkey = sizeof(fixture.agent_privkey);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &privkey_buffer, &fixture.alloc_opts, sz_privkey));
    memcpy(privkey_buffer.data, fixture.agent_privkey, sz_privkey);

    TEST_ASSERT(
        STATUS_SUCCESS
            == auth_service_api_sendreq_initialize_block(
                    fixture.authsock, &agent_id_buffer, &pubkey_buffer,
                    &privkey_buffer));

    TEST_ASSERT(
        STATUS_SUCCESS
            == auth_service_api_recvresp_initialize_block(
                    fixture.authsock, &offset, &status));

    TEST_EXPECT(0U == offset);
    TEST_EXPECT(0U == status);

    /* clean up */
    dispose((disposable_t*)&privkey_buffer);
    dispose((disposable_t*)&pubkey_buffer);
    dispose((disposable_t*)&agent_id_buffer);
END_TEST_F()

/**
 * Test that we can initialize the auth service.
 */
BEGIN_TEST_F(initialize)
    TEST_ASSERT(0 == fixture.auth_service_proc_status);

    uint32_t offset = 0U;
    uint32_t status = 0U;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* initialize the agent ID buffer  */
    vccrypt_buffer_t agent_id_buffer;
    size_t sz_agent_id = sizeof(fixture.agent_id);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &agent_id_buffer, &fixture.alloc_opts, sz_agent_id));
    memcpy(agent_id_buffer.data, fixture.agent_id, sz_agent_id);

    /* initialize the public key */
    vccrypt_buffer_t pubkey_buffer;
    size_t sz_pubkey = sizeof(fixture.agent_pubkey);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &pubkey_buffer, &fixture.alloc_opts, sz_pubkey));
    memcpy(pubkey_buffer.data, fixture.agent_pubkey, sz_pubkey);

    /* initialize the private key */
    vccrypt_buffer_t privkey_buffer;
    size_t sz_privkey = sizeof(fixture.agent_privkey);
    TEST_ASSERT(
        STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &privkey_buffer, &fixture.alloc_opts, sz_privkey));
    memcpy(privkey_buffer.data, fixture.agent_privkey, sz_privkey);

    /* Run the send / receive on creating the root context. */
    fixture.nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    auth_service_api_recvresp_initialize(
                        &fixture.nonblockauthsock, &offset, &status);
                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status = auth_service_api_sendreq_initialize(
                    &fixture.nonblockauthsock, &agent_id_buffer, &pubkey_buffer,
                    &privkey_buffer);
            }
        });

    /* verify that everything ran correctly. */
    TEST_EXPECT(0 == sendreq_status);
    TEST_EXPECT(0 == recvresp_status);
    TEST_EXPECT(0U == offset);
    TEST_EXPECT(0U == status);

    /* clean up */
    dispose((disposable_t*)&privkey_buffer);
    dispose((disposable_t*)&pubkey_buffer);
    dispose((disposable_t*)&agent_id_buffer);
END_TEST_F()
