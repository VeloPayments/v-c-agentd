/**
 * \file test/notificatonservice/test_notificationservice_api_recvresp.cpp
 *
 * \brief Test notificationservice_api_recvresp.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Test that command-line parameters are null checked.
 */
TEST(notificationservice_api_recvresp_test, argument_nullchecks)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "Test";
    size_t size;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create a dummy psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, input, sizeof(input)));

    /* if the socket is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_recvresp(nullptr, alloc, &buf, &size));

    /* if the allocator is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_recvresp(sock, nullptr, &buf, &size));

    /* if the buffer pointer is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_recvresp(sock, alloc, nullptr, &size));

    /* if the size pointer is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_recvresp(sock, alloc, &buf, nullptr));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief If anything other than a data packet is written to the psock instance,
 * an error is returned.
 */
TEST(notificationservice_api_recvresp_test, bad_data_packet)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "X";
    size_t size;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create a test psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, input, strlen(input)));

    /* reading from the socket will fail. */
    EXPECT_NE(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(sock, alloc, &buf, &size));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief The receive response method reads a data packet from the socket.
 */
TEST(notificationservice_api_recvresp_test, basics)
{
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t* buf;
    const char* input = "X";
    size_t size;
    char* test_buffer;
    size_t test_size;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create a writing psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, NULL, 0));

    /* write a data packet to this socket. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_write_boxed_data(sock, input, strlen(input)));

    /* get the output buffer. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_from_buffer_get_output_buffer(
            sock, alloc, (void**)&test_buffer, &test_size));

    /* release the output psock. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));

    /* create a test psock instance, backed by the output buffer. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, test_buffer, test_size));

    /* reading from the socket will succeed. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_recvresp(sock, alloc, &buf, &size));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, test_buffer));
    ASSERT_EQ(STATUS_SUCCESS, rcpr_allocator_reclaim(alloc, buf));
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}
