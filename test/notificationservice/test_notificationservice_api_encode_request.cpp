/**
 * \file
 * test/notificatonservice/test_notificationservice_api_encode_request.cpp
 *
 * \brief Test notificationservice_api_encode_request.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Test that command-line parameters are null checked.
 */
TEST(notificationservice_api_encode_request_test, argument_nullchecks)
{
    rcpr_allocator* alloc;
    uint8_t* buf;
    size_t size;
    uint32_t method_id = AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS;
    uint64_t offset = 1234;
    const uint8_t* payload = (const uint8_t*)"test";
    size_t payload_size = strlen("test");

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* If the buffer is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_encode_request(
            nullptr, &size, alloc, method_id, offset, payload, payload_size));

    /* If the buffer size is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_encode_request(
            &buf, nullptr, alloc, method_id, offset, payload, payload_size));

    /* If the allocator is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_encode_request(
            &buf, &size, nullptr, method_id, offset, payload, payload_size));

    /* clean up. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief Test that a buffer is properly encoded.
 */
TEST(notificationservice_api_encode_request_test, basics)
{
    rcpr_allocator* alloc;
    uint8_t* buf = NULL;
    size_t size;
    uint32_t EXPECTED_METHOD_ID =
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS;
    uint64_t EXPECTED_OFFSET = 1234;
    const uint8_t* EXPECTED_PAYLOAD = (const uint8_t*)"test";
    size_t EXPECTED_PAYLOAD_SIZE = strlen("test");

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* The encode request succeeds. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_encode_request(
            &buf, &size, alloc, EXPECTED_METHOD_ID, EXPECTED_OFFSET,
            EXPECTED_PAYLOAD, EXPECTED_PAYLOAD_SIZE));

    /* the returned buffer is not null. */
    ASSERT_NE(nullptr, buf);

    /* the returned size is valid. */
    size_t computed_size =
        sizeof(EXPECTED_METHOD_ID) + sizeof(EXPECTED_OFFSET)
      + EXPECTED_PAYLOAD_SIZE;
    ASSERT_EQ(computed_size, size);

    /* for convenience, use a separate pointer for enumeration. */
    const uint8_t* tmp = buf;

    /* verify the method id. */
    uint32_t net_method_id;
    memcpy(&net_method_id, tmp, sizeof(net_method_id));
    tmp += sizeof(net_method_id); size -= sizeof(net_method_id);
    EXPECT_EQ(EXPECTED_METHOD_ID, ntohl(net_method_id));

    /* verify the offset. */
    uint64_t net_offset;
    memcpy(&net_offset, tmp, sizeof(net_offset));
    tmp += sizeof(net_offset); size -= sizeof(net_offset);
    EXPECT_EQ(EXPECTED_OFFSET, ntohll(net_offset));

    /* the remaining size should be equal to the payload size. */
    ASSERT_EQ(EXPECTED_PAYLOAD_SIZE, size);

    /* the payload should be equal to the expected payload. */
    EXPECT_EQ(0, memcmp(tmp, EXPECTED_PAYLOAD, EXPECTED_PAYLOAD_SIZE));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}
