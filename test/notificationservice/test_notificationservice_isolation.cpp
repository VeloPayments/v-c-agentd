/**
 * \file test/notificationservice/test_notificationservice_isolation.cpp
 *
 * Isolation tests for the notificationservice.
 *
 * \copyright 2022-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <minunit/minunit.h>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_notificationservice_isolation.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_uuid;

TEST_SUITE(notificationservice_isolation_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    notificationservice_isolation_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the notificationservice.
 */
BEGIN_TEST_F(simple_spawn)
    TEST_ASSERT(0 == fixture.notify_proc_status);
END_TEST_F()

/**
 * Test that we can reduce capabilities.
 */
BEGIN_TEST_F(reduce_caps)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that reducing capabilities to nothing fails the second time due to an
 * authorization error.
 */
BEGIN_TEST_F(reduce_caps_2x)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* send second reduce capabilities request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS == method_id);
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that sending a block update returns a success status code.
 */
BEGIN_TEST_F(block_update_simple)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that a block update fails if not authorized.
 */
BEGIN_TEST_F(block_update_not_authorized)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* reclaim memory for the response buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* send block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify that the request failed. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE == method_id);
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that we are immediately invalidated when the latest block has not been
 * set.
 */
BEGIN_TEST_F(block_assertion_zero_block)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION
            == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that a block assertion against a block other than the latest block
 * update returns with an immediate invalidation.
 */
BEGIN_TEST_F(block_assertion_different_block)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &latest_block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* reclaim buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* send block assertion request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION
            == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that a block assertion for the latest block does not return an
 * invalidation until the block is updated.
 */
BEGIN_TEST_F(block_assertion_same_block)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.client1, fixture.alloc,
                    EXPECTED_BLOCK_UPDATE_OFFSET, &latest_block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* reclaim buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* send the block assertion request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &latest_block_id));

    /* send the next block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.client1, fixture.alloc,
                    EXPECTED_BLOCK_UPDATE_OFFSET, &next_block_id));

    /* get a response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode the response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* it should be the invalidation for the assertion. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* reclaim buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* get the next response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode this response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* it should be the block update response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_BLOCK_UPDATE_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that a block assertion fails if not authorized.
 * set.
 */
BEGIN_TEST_F(block_assertion_not_authorized)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* reclaim memory for the response buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* send block assertion request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    &block_id));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify response. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION == method_id);
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * When a block assertion has NOT been made, a block assertion cancellation
 * still succeeds.
 */
BEGIN_TEST_F(block_assertion_cancellation_empty)
    uint8_t* buf = nullptr;
    size_t size = 0U;
    const uint64_t EXPECTED_OFFSET = 7177;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* send block assertion cancellation request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_assertion_cancel(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode the response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify that the cancellation succeeded. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL
            == method_id);
    TEST_EXPECT(0 == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * Test that a block assertion cancellation request fails if not authorized.
 */
BEGIN_TEST_F(block_assertion_cancellation_not_authorized)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET,
                    reducedcaps, sizeof(reducedcaps)));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* reclaim memory for the response buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));

    /* send block assertion cancellation request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_assertion_cancel(
                    fixture.client1, fixture.alloc, EXPECTED_OFFSET));

    /* get response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.client1, fixture.alloc, &buf, &size));

    /* decode the response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* verify that the cancellation failed due to access control. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL
            == method_id);
    TEST_EXPECT(AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED == status_code);
    TEST_EXPECT(EXPECTED_OFFSET == offset);
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()
