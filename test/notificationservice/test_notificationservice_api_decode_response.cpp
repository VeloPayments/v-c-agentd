/**
 * \file
 * test/notificatonservice/test_notificationservice_api_decode_response.cpp
 *
 * \brief Test notificationservice_api_decode_response.
 *
 * \copyright 2022-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

TEST_SUITE(notificationservice_api_decode_response_test);

/**
 * \brief Test that command-line parameters are null checked.
 */
TEST(argument_nullchecks)
{
    const uint8_t buf[] = { 'T', 'e', 's', 't' };
    size_t size = sizeof(buf);
    uint32_t method_id = 0U;
    uint32_t status_code = 0U;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* If the buffer is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    nullptr, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* If the method_id is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, nullptr, &status_code, &offset, &payload,
                    &payload_size));

    /* If the status_code is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, &method_id, nullptr, &offset, &payload,
                    &payload_size));

    /* If the offset is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, nullptr, &payload,
                    &payload_size));

    /* If the payload is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, nullptr,
                    &payload_size));

    /* If the payload_size is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    nullptr));
}

/**
 * \brief If the size is too small, an error is returned.
 */
TEST(size_too_small)
{
    const uint8_t buf[] = { 'T', 'e', 's', 't' };
    size_t size = sizeof(buf);
    uint32_t method_id = 0U;
    uint32_t status_code = 0U;
    uint64_t offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* If the size is too small, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));
}

/**
 * \brief A buffer without a payload encoded by the encode method can be decoded
 * by the decode method.
 */
TEST(encode_decode_no_payload)
{
    rcpr_allocator* alloc;
    uint8_t* buf = NULL;
    size_t size;
    uint32_t EXPECTED_METHOD_ID =
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS;
    uint32_t EXPECTED_STATUS_CODE = STATUS_SUCCESS;
    uint64_t EXPECTED_OFFSET = 1234;
    const uint8_t* EXPECTED_PAYLOAD = NULL;
    size_t EXPECTED_PAYLOAD_SIZE = 0U;
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload;
    size_t payload_size;

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* The encode request succeeds. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_encode_response(
                    &buf, &size, alloc, EXPECTED_METHOD_ID,
                    EXPECTED_STATUS_CODE, EXPECTED_OFFSET, EXPECTED_PAYLOAD,
                    EXPECTED_PAYLOAD_SIZE));

    /* the returned buffer is not null. */
    TEST_ASSERT(nullptr != buf);

    /* the decode request succeeds. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id matches. */
    TEST_EXPECT(EXPECTED_METHOD_ID == method_id);

    /* the status code matches. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);

    /* the offset matches. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* the payload is NULL. */
    TEST_EXPECT(nullptr == payload);

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(alloc, buf));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief A buffer with a payload encoded by the encode method can be decoded
 * by the decode method.
 */
TEST(encode_decode_with_payload)
{
    rcpr_allocator* alloc;
    uint8_t* buf = NULL;
    size_t size;
    uint32_t EXPECTED_METHOD_ID =
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS;
    uint32_t EXPECTED_STATUS_CODE = STATUS_SUCCESS;
    uint64_t EXPECTED_OFFSET = 1234;
    const uint8_t EXPECTED_PAYLOAD[] = { 'T', 'e', 's', 't' };
    size_t EXPECTED_PAYLOAD_SIZE = sizeof(EXPECTED_PAYLOAD);
    uint32_t method_id;
    uint32_t status_code;
    uint64_t offset;
    const uint8_t* payload;
    size_t payload_size;

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* The encode request succeeds. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_encode_response(
                    &buf, &size, alloc, EXPECTED_METHOD_ID,
                    EXPECTED_STATUS_CODE, EXPECTED_OFFSET, EXPECTED_PAYLOAD,
                    EXPECTED_PAYLOAD_SIZE));

    /* the returned buffer is not null. */
    TEST_ASSERT(nullptr != buf);

    /* the decode request succeeds. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_decode_response(
                    buf, size, &method_id, &status_code, &offset, &payload,
                    &payload_size));

    /* the method id matches. */
    TEST_EXPECT(EXPECTED_METHOD_ID == method_id);

    /* the status code matches. */
    TEST_EXPECT(EXPECTED_STATUS_CODE == status_code);

    /* the offset matches. */
    TEST_EXPECT(EXPECTED_OFFSET == offset);

    /* the payload is NOT NULL. */
    TEST_ASSERT(nullptr != payload);

    /* the payload size matches the expected payload size. */
    TEST_ASSERT(EXPECTED_PAYLOAD_SIZE == payload_size);

    /* the payload matches the expected payload. */
    TEST_EXPECT(0 == memcmp(payload, EXPECTED_PAYLOAD, payload_size));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(alloc, buf));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}
