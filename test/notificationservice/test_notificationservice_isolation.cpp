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

/**
 * Test that we are immediately invalidated when the latest block has not been
 * set.
 */
TEST_F(notificationservice_isolation_test, block_assertion_zero_block)
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

    /* send block assertion request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_assertion(
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
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
        method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that a block assertion against a block other than the latest block
 * update returns with an immediate invalidation.
 */
TEST_F(notificationservice_isolation_test, block_assertion_different_block)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    rcpr_uuid latest_block_id = { .data = {
        0xa4, 0xcf, 0x44, 0x00, 0x80, 0x0f, 0x48, 0x27,
        0xba, 0xc3, 0x54, 0x2c, 0xfc, 0x56, 0xdf, 0x9d } };
    rcpr_uuid block_id = { .data = {
        0xdd, 0x4c, 0x97, 0x97, 0xcb, 0x8d, 0x4e, 0xaa,
        0xaa, 0x1f, 0x4e, 0xf9, 0x8c, 0x1e, 0x3a, 0xac } };

    /* send block update request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_update(
            client1, alloc, EXPECTED_OFFSET, &latest_block_id));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* reclaim buffer. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));

    /* send block assertion request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_assertion(
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
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
        method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that a block assertion for the latest block does not return an
 * invalidation until the block is updated.
 */
TEST_F(notificationservice_isolation_test, block_assertion_same_block)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    const uint64_t EXPECTED_BLOCK_UPDATE_OFFSET = 17;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    rcpr_uuid latest_block_id = { .data = {
        0xa4, 0xcf, 0x44, 0x00, 0x80, 0x0f, 0x48, 0x27,
        0xba, 0xc3, 0x54, 0x2c, 0xfc, 0x56, 0xdf, 0x9d } };
    rcpr_uuid next_block_id = { .data = {
        0xdd, 0x4c, 0x97, 0x97, 0xcb, 0x8d, 0x4e, 0xaa,
        0xaa, 0x1f, 0x4e, 0xf9, 0x8c, 0x1e, 0x3a, 0xac } };

    /* send block update request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_update(
            client1, alloc, EXPECTED_BLOCK_UPDATE_OFFSET, &latest_block_id));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* reclaim buffer. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));

    /* send the block assertion request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_assertion(
            client1, alloc, EXPECTED_OFFSET, &latest_block_id));

    /* send the next block update request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_update(
            client1, alloc, EXPECTED_BLOCK_UPDATE_OFFSET, &next_block_id));

    /* get a response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode the response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* it should be the invalidation for the assertion. */
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
        method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* reclaim buffer. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));

    /* get the next response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode this response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* it should be the block update response. */
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE,
        method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_BLOCK_UPDATE_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * Test that a block assertion fails if not authorized.
 * set.
 */
TEST_F(notificationservice_isolation_test, block_assertion_not_authorized)
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

    /* send block assertion request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_block_assertion(
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
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
        method_id);
    EXPECT_EQ(AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}

/**
 * When a block assertion has NOT been made, a block assertion cancellation
 * still succeeds.
 */
TEST_F(notificationservice_isolation_test, block_assertion_cancellation_empty)
{
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* send block assertion cancellation request. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_assertion_cancel(
            client1, alloc, EXPECTED_OFFSET));

    /* get response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(
            client1, alloc, &buf, &size));

    /* decode the response. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_decode_response(
            buf, size, &method_id, &status_code, &offset, &payload,
            &payload_size));

    /* verify that the cancellation succeeded. */
    EXPECT_EQ(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL,
        method_id);
    EXPECT_EQ(0, status_code);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(nullptr, payload);

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
}
