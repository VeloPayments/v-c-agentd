/**
 * \file
 * test/notificatonservice/test_notificationservice_api_sendreq_reduce_caps.cpp
 *
 * \brief Test notificationservice_api_sendreq_reduce_caps.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <gtest/gtest.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Test that the parameters are null checked.
 */
TEST(notificationservice_api_sendreq_reduce_caps_test, argument_nullchecks)
{
    rcpr_allocator* alloc;
    psock* sock;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    uint64_t offset = 1234;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create a dummy psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, nullptr, 0));

    /* if the socket is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_reduce_caps(
            nullptr, alloc, offset, caps, sizeof(caps)));

    /* if the allocator is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_reduce_caps(
            sock, nullptr, offset, caps, sizeof(caps)));

    /* if the bitcap array is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_reduce_caps(
            sock, alloc, offset, nullptr, sizeof(caps)));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief Test that cap size is checked.
 */
TEST(notificationservice_api_sendreq_reduce_caps_test, argument_cap_size)
{
    rcpr_allocator* alloc;
    psock* sock;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    uint64_t offset = 1234;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create a dummy psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, nullptr, 0));

    /* if the cap size is wrong, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_reduce_caps(
            sock, alloc, offset, caps, 1024));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief Test that the request is sent.
 */
TEST(notificationservice_api_sendreq_reduce_caps_test, basics)
{
    rcpr_allocator* alloc;
    psock* sock;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    uint64_t offset = 1234;

    /* create an allocator instance. */
    ASSERT_EQ(STATUS_SUCCESS, rcpr_malloc_allocator_create(&alloc));

    /* create an output buffer psock instance. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_buffer(&sock, alloc, nullptr, 0));

    /* The request should succeed. */
    EXPECT_EQ(
        STATUS_SUCCESS,
        notificationservice_api_sendreq_reduce_caps(
            sock, alloc, offset, caps, sizeof(caps)));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}
