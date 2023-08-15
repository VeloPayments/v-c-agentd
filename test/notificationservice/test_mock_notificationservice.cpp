/**
 * \file test_mock_notificationservice.cpp
 *
 * Test the mock notificationservice.
 *
 * \copyright 2022-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>
#include <ostream>

#include "test_mock_notificationservice.h"
#include "../mocks/notificationservice.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_uuid;

TEST_SUITE(mock_notificationservice_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    mock_notificationservice_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the mock notificationservice.
 */
BEGIN_TEST_F(basic_spawn)
    TEST_EXPECT(fixture.test_suite_valid);
END_TEST_F()

/**
 * If the block update mock is not set, then sending a block update request
 * always ends with success.
 */
BEGIN_TEST_F(default_block_update)
    uint64_t EXPECTED_OFFSET = 7177;
    rcpr_uuid EXPECTED_BLOCK_ID = { .data = {
        0xb3, 0x75, 0xb6, 0x40, 0x90, 0xe4, 0x46, 0x68,
        0x92, 0xb5, 0x51, 0x9f, 0x19, 0xff, 0xdc, 0xe3 } };
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET,
                    &EXPECTED_BLOCK_ID));

    /* we should be able to match on this request. */
    TEST_EXPECT(
        fixture.mock->request_matches_block_update(
            EXPECTED_OFFSET, &EXPECTED_BLOCK_ID));

    /* we should be able to receive a block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE == method_id);
    /* the status code should be success. */
    TEST_EXPECT(STATUS_SUCCESS == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * We can override the block update method to return a different status.
 */
BEGIN_TEST_F(block_update_override)
    uint64_t EXPECTED_OFFSET = 7177;
    rcpr_uuid EXPECTED_BLOCK_ID = { .data = {
        0xb3, 0x75, 0xb6, 0x40, 0x90, 0xe4, 0x46, 0x68,
        0x92, 0xb5, 0x51, 0x9f, 0x19, 0xff, 0xdc, 0xe3 } };
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    uint32_t EXPECTED_STATUS_CODE =
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;

    /* override the mock. */
    fixture.mock->register_callback_block_update(
        [=](uint64_t, const rcpr_uuid*) -> int {
            return EXPECTED_STATUS_CODE;
        });

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_update(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET,
                    &EXPECTED_BLOCK_ID));

    /* we should be able to receive a block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE == method_id);
    /* the status code should be success. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * If the reduce capabilities mock is not set, then sending a reduce
 * capabilities request always ends with success.
 */
BEGIN_TEST_F(default_reduce_caps)
    uint64_t EXPECTED_OFFSET = 7177;
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a reduce caps request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET, caps,
                    sizeof(caps)));

    /* we should be able to match on this request. */
    TEST_EXPECT(
        fixture.mock->request_matches_reduce_caps(
            EXPECTED_OFFSET, caps, sizeof(caps)));

    /* we should be able to receive a reduce caps response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this reduce caps response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS == method_id);
    /* the status code should be success. */
    TEST_EXPECT(STATUS_SUCCESS == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * We can override the reduce caps mock to return a different status.
 */
BEGIN_TEST_F(reduce_caps_override)
    uint64_t EXPECTED_OFFSET = 7177;
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    uint32_t EXPECTED_STATUS_CODE =
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;

    /* override the mock. */
    fixture.mock->register_callback_reduce_caps(
        [=](uint64_t, const uint32_t*, size_t) -> int {
            return EXPECTED_STATUS_CODE;
        });

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a reduce caps request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_reduce_caps(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET, caps,
                    sizeof(caps)));

    /* we should be able to receive a reduce caps response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this reduce caps response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS == method_id);
    /* the status code should be success. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * If the block assertion mock is not set, then sending a block assertion
 * request always ends with success.
 */
BEGIN_TEST_F(default_block_assertion)
    uint64_t EXPECTED_OFFSET = 7177;
    rcpr_uuid EXPECTED_BLOCK_ID = { .data = {
        0xb3, 0x75, 0xb6, 0x40, 0x90, 0xe4, 0x46, 0x68,
        0x92, 0xb5, 0x51, 0x9f, 0x19, 0xff, 0xdc, 0xe3 } };
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET,
                    &EXPECTED_BLOCK_ID));

    /* we should be able to match on this request. */
    TEST_EXPECT(
        fixture.mock->request_matches_block_assertion(
            EXPECTED_OFFSET, &EXPECTED_BLOCK_ID));

    /* we should be able to receive a block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION == method_id);
    /* the status code should be success. */
    TEST_EXPECT(STATUS_SUCCESS == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * We can override the block assertion method to return a different status.
 */
BEGIN_TEST_F(block_assertion_override)
    uint64_t EXPECTED_OFFSET = 7177;
    rcpr_uuid EXPECTED_BLOCK_ID = { .data = {
        0xb3, 0x75, 0xb6, 0x40, 0x90, 0xe4, 0x46, 0x68,
        0x92, 0xb5, 0x51, 0x9f, 0x19, 0xff, 0xdc, 0xe3 } };
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    uint32_t EXPECTED_STATUS_CODE =
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;

    /* override the mock. */
    fixture.mock->register_callback_block_assertion(
        [=](uint64_t, const rcpr_uuid*) -> int {
            return EXPECTED_STATUS_CODE;
        });

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block assertion request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_block_assertion(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET,
                    &EXPECTED_BLOCK_ID));

    /* we should be able to receive a block assertion response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block assertion response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION == method_id);
    /* the status code should be success. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * If the block assertion cancel mock is not set, then sending a block assertion
 * cancel request always ends with success.
 */
BEGIN_TEST_F(default_block_assertion_cancel)
    uint64_t EXPECTED_OFFSET = 7177;
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_assertion_cancel(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET));

    /* we should be able to match on this request. */
    TEST_EXPECT(
        fixture.mock->request_matches_block_assertion_cancel(EXPECTED_OFFSET));

    /* we should be able to receive a block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL
            == method_id);
    /* the status code should be success. */
    TEST_EXPECT(STATUS_SUCCESS == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()

/**
 * We can override the block assertion cancel method to return a different
 * status.
 */
BEGIN_TEST_F(block_assertion_cancel_override)
    uint64_t EXPECTED_OFFSET = 7177;
    uint8_t* buf = nullptr;
    size_t size = 0U;
    uint32_t method_id = 0U;
    uint32_t status_code = 0xFF;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    uint32_t EXPECTED_STATUS_CODE =
        AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;

    /* override the mock. */
    fixture.mock->register_callback_block_assertion_cancel(
        [=](uint64_t) -> int {
            return EXPECTED_STATUS_CODE;
        });

    /* start the mock notificationservice. */
    fixture.mock->start();

    /* we should be able to send a block update request. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_sendreq_assertion_cancel(
                    fixture.sock, fixture.alloc, EXPECTED_OFFSET));

    /* we should be able to receive a block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(
                    fixture.sock, fixture.alloc, &buf, &size));

    /* the response buffer should not be NULL. */
    TEST_ASSERT(nullptr != buf);

    /* we should be able to decode this block update response. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id should match. */
    TEST_EXPECT(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL
            == method_id);
    /* the status code should be success. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);
    /* the offset should match. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* clean up the buffer. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(fixture.alloc, buf));
END_TEST_F()
