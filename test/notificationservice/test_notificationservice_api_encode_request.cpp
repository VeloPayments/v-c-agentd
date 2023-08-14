/**
 * \file
 * test/notificatonservice/test_notificationservice_api_encode_request.cpp
 *
 * \brief Test notificationservice_api_encode_request.
 *
 * \copyright 2022-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

TEST_SUITE(notificationservice_api_encode_request_test);

/**
 * \brief Test that command-line parameters are null checked.
 */
TEST(argument_nullchecks)
{
    rcpr_allocator* alloc;
    uint8_t* buf;
    size_t size;
    uint32_t method_id = AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS;
    uint64_t offset = 1234;
    const uint8_t* payload = (const uint8_t*)"test";
    size_t payload_size = strlen("test");

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* If the buffer is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_encode_request(
                    nullptr, &size, alloc, method_id, offset, payload,
                    payload_size));

    /* If the buffer size is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_encode_request(
                    &buf, nullptr, alloc, method_id, offset, payload,
                    payload_size));

    /* If the allocator is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_encode_request(
                    &buf, &size, nullptr, method_id, offset, payload,
                    payload_size));

    /* clean up. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief Test that a buffer is properly encoded.
 */
TEST(basics)
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
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* The encode request succeeds. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_encode_request(
                    &buf, &size, alloc, EXPECTED_METHOD_ID, EXPECTED_OFFSET,
                    EXPECTED_PAYLOAD, EXPECTED_PAYLOAD_SIZE));

    /* the returned buffer is not null. */
    TEST_ASSERT(nullptr != buf);

    /* the returned size is valid. */
    size_t computed_size =
        sizeof(EXPECTED_METHOD_ID) + sizeof(EXPECTED_OFFSET)
      + EXPECTED_PAYLOAD_SIZE;
    TEST_ASSERT(computed_size == size);

    /* for convenience, use a separate pointer for enumeration. */
    const uint8_t* tmp = buf;

    /* verify the method id. */
    uint32_t net_method_id;
    memcpy(&net_method_id, tmp, sizeof(net_method_id));
    tmp += sizeof(net_method_id); size -= sizeof(net_method_id);
    TEST_EXPECT(EXPECTED_METHOD_ID == ntohl(net_method_id));

    /* verify the offset. */
    uint64_t net_offset;
    memcpy(&net_offset, tmp, sizeof(net_offset));
    tmp += sizeof(net_offset); size -= sizeof(net_offset);
    TEST_EXPECT(EXPECTED_OFFSET == (uint64_t)ntohll(net_offset));

    /* the remaining size should be equal to the payload size. */
    TEST_ASSERT(EXPECTED_PAYLOAD_SIZE == size);

    /* the payload should be equal to the expected payload. */
    TEST_EXPECT(0 == memcmp(tmp, EXPECTED_PAYLOAD, EXPECTED_PAYLOAD_SIZE));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(alloc, buf));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}
