/**
 * \file test_random_service_isolation.cpp
 *
 * Isolation tests for the random service.
 *
 * \copyright 2020-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <minunit/minunit.h>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_random_service_isolation.h"

using namespace std;

TEST_SUITE(random_service_isolation_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    random_service_isolation_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the random service.
 */
BEGIN_TEST_F(simple_spawn)
    TEST_ASSERT(0 == fixture.random_proc_status);
    TEST_ASSERT(0 == fixture.ralloc_status);
    TEST_ASSERT(0 == fixture.proto_status);
END_TEST_F()

/**
 * Test that we can get one byte of random data from the random service.
 */
BEGIN_TEST_F(one_byte)
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    /* send a blocking request to get random bytes. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == random_service_api_sendreq_random_bytes_get(
                    fixture.proto, EXPECTED_OFFSET, 1));

    /* receive a blocking response to get random bytes. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == random_service_api_recvresp_random_bytes_get(
                    fixture.proto, fixture.ralloc, &offset, &status,
                    &random_byte_buffer, &random_byte_buffer_size));

    /* verify offset, status, and size. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    TEST_EXPECT(1U == random_byte_buffer_size);
END_TEST_F()

/**
 * Test that we can get many bytes of random data from the random service.
 */
BEGIN_TEST_F(many_bytes)
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    /* send a blocking request to get random bytes. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == random_service_api_sendreq_random_bytes_get(
                    fixture.proto, EXPECTED_OFFSET, 100));

    /* receive a blocking response to get random bytes. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == random_service_api_recvresp_random_bytes_get(
                    fixture.proto, fixture.ralloc, &offset, &status,
                    &random_byte_buffer, &random_byte_buffer_size));

    /* verify offset, status, and size. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    TEST_EXPECT(100U == random_byte_buffer_size);
END_TEST_F()

/**
 * Test that we can get one byte of random data from the random service.
 */
BEGIN_TEST_F(one_byte_deprecated)
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    fixture.nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    random_service_api_recvresp_random_bytes_get_old(
                        &fixture.nonblockrandomsock, &offset, &status,
                        &random_byte_buffer, &random_byte_buffer_size);

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
                sendreq_status =
                    random_service_api_sendreq_random_bytes_get_old(
                        &fixture.nonblockrandomsock, EXPECTED_OFFSET, 1);
            }
        });

    /* verify the send request status. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == sendreq_status);

    /* verify offset, status, and size. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == recvresp_status);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(1U == random_byte_buffer_size);
END_TEST_F()

/**
 * Test that we can get many bytes of random data from the random service.
 */
BEGIN_TEST_F(many_bytes_deprecated)
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    fixture.nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    random_service_api_recvresp_random_bytes_get_old(
                        &fixture.nonblockrandomsock, &offset, &status,
                        &random_byte_buffer, &random_byte_buffer_size);

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
                sendreq_status =
                    random_service_api_sendreq_random_bytes_get_old(
                        &fixture.nonblockrandomsock, EXPECTED_OFFSET, 100);
            }
        });

    /* verify the send request status. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == sendreq_status);

    /* verify offset, status, and size. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == recvresp_status);
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(100U == random_byte_buffer_size);
END_TEST_F()
