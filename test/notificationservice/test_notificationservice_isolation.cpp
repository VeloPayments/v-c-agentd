/**
 * \file test/notificationservice/test_notificationservice_isolation.cpp
 *
 * Isolation tests for the notificationservice.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_notificationservice_isolation.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_uuid;

/**
 * Test that we can spawn the notificationservice.
 */
TEST_F(notificationservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, notify_proc_status);
}

/**
 * Test that we can reduce capabilities.
 */
TEST_F(notificationservice_isolation_test, reduce_caps)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* create a reduced capabilities set. */
    BITCAP(reducedcaps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* send reduce capabilities request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_reduce_caps(
            client1, alloc, EXPECTED_OFFSET, reducedcaps,
            sizeof(reducedcaps)));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify response. */
    EXPECT_EQ(AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS, method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that reducing capabilities to nothing fails the second time due to an
 * authorization error.
 */
TEST_F(notificationservice_isolation_test, reduce_caps_2x)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* create a reduced capabilities set. */
    BITCAP(reducedcaps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* send reduce capabilities request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_reduce_caps(
            client1, alloc, EXPECTED_OFFSET, reducedcaps,
            sizeof(reducedcaps)));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify response. */
    EXPECT_EQ(AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS, method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* send second reduce capabilities request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_reduce_caps(
            client1, alloc, EXPECTED_OFFSET, reducedcaps,
            sizeof(reducedcaps)));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify response. */
    EXPECT_EQ(AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS, method_id);
    EXPECT_EQ(AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that sending a block update returns a success status code.
 */
TEST_F(notificationservice_isolation_test, block_update_simple)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    rcpr_uuid block_id = { .data = {
        0xdd, 0x4c, 0x97, 0x97, 0xcb, 0x8d, 0x4e, 0xaa,
        0xaa, 0x1f, 0x4e, 0xf9, 0x8c, 0x1e, 0x3a, 0xac } };

    /* send block update request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_update(
            client1, alloc, EXPECTED_OFFSET, &block_id));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify response. */
    EXPECT_EQ(AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE, method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that a block update fails if not authorized.
 */
TEST_F(notificationservice_isolation_test, block_update_not_authorized)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    rcpr_uuid block_id = { .data = {
        0xdd, 0x4c, 0x97, 0x97, 0xcb, 0x8d, 0x4e, 0xaa,
        0xaa, 0x1f, 0x4e, 0xf9, 0x8c, 0x1e, 0x3a, 0xac } };

    /* create a reduced capabilities set. */
    BITCAP(reducedcaps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* send reduce capabilities request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_reduce_caps(
            client1, alloc, EXPECTED_OFFSET, reducedcaps,
            sizeof(reducedcaps)));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* reclaim memory for the response buffer. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));

    /* send block update request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_update(
            client1, alloc, EXPECTED_OFFSET, &block_id));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify that the request failed. */
    EXPECT_EQ(AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE, method_id);
    EXPECT_EQ(AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}
