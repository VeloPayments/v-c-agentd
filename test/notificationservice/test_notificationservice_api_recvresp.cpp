/**
 * \file test/notificatonservice/test_notificationservice_api_recvresp.cpp
 *
 * \brief Test notificationservice_api_recvresp.
 *
 * \copyright 2022-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

TEST_SUITE(notificationservice_api_recvresp_test);

/**
 * \brief Test that command-line parameters are null checked.
 */
TEST(argument_nullchecks)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "Test";
    size_t size;

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create a dummy psock instance. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_buffer(&sock, alloc, input, sizeof(input)));

    /* if the socket is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_recvresp(nullptr, alloc, &buf, &size));

    /* if the allocator is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_recvresp(sock, nullptr, &buf, &size));

    /* if the buffer pointer is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_recvresp(sock, alloc, nullptr, &size));

    /* if the size pointer is null, an error is returned. */
    TEST_EXPECT(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT
            == notificationservice_api_recvresp(sock, alloc, &buf, nullptr));

    /* clean up. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(psock_resource_handle(sock)));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief If anything other than a data packet is written to the psock instance,
 * an error is returned.
 */
TEST(bad_data_packet)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "X";
    size_t size;

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create a test psock instance. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_buffer(&sock, alloc, input, strlen(input)));

    /* reading from the socket will fail. */
    TEST_EXPECT(
        STATUS_SUCCESS
            != notificationservice_api_recvresp(sock, alloc, &buf, &size));

    /* clean up. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(psock_resource_handle(sock)));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief The receive response method reads a data packet from the socket.
 */
TEST(basics)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "X";
    size_t size;
    char* test_buffer;
    size_t test_size;

    /* create an allocator instance. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_malloc_allocator_create(&alloc));

    /* create a writing psock instance. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_buffer(&sock, alloc, NULL, 0));

    /* write a data packet to this socket. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_write_boxed_data(sock, input, strlen(input)));

    /* get the output buffer. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_from_buffer_get_output_buffer(
                    sock, alloc, (void**)&test_buffer, &test_size));

    /* release the output psock. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(psock_resource_handle(sock)));

    /* create a test psock instance, backed by the output buffer. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == psock_create_from_buffer(&sock, alloc, test_buffer, test_size));

    /* reading from the socket will succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == notificationservice_api_recvresp(sock, alloc, &buf, &size));

    /* clean up. */
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(alloc, test_buffer));
    TEST_ASSERT(STATUS_SUCCESS == rcpr_allocator_reclaim(alloc, buf));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(psock_resource_handle(sock)));
    TEST_ASSERT(
        STATUS_SUCCESS
            == resource_release(rcpr_allocator_resource_handle(alloc)));
}
