/**
 * \file
 * test/notificatonservice/test_notificationservice_api_sendreq_block_update.cpp
 *
 * \brief Test notificationservice_api_sendreq_block_update.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>

/* GTEST DISABLED */
#if 0

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Test that the parameters are null checked.
 */
TEST(notificationservice_api_sendreq_block_update_test, argument_nullchecks)
{
    rcpr_allocator* alloc;
    psock* sock;
    rcpr_uuid block_id = {
        0x5f, 0xb5, 0x31, 0xc5, 0x7e, 0x64, 0x4f, 0xb5,
        0xbc, 0x86, 0xf4, 0x54, 0xe2, 0x88, 0x32, 0xfa };
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
        notificationservice_api_sendreq_block_update(
            nullptr, alloc, offset, &block_id));

    /* if the allocator is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_block_update(
            sock, nullptr, offset, &block_id));

    /* if the block id is null, an error is returned. */
    EXPECT_EQ(
        AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT,
        notificationservice_api_sendreq_block_update(
            sock, alloc, offset, nullptr));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}

/**
 * \brief Test that the request is sent.
 */
TEST(notificationservice_api_sendreq_block_update_test, basics)
{
    rcpr_allocator* alloc;
    psock* sock;
    rcpr_uuid block_id = {
        0x5f, 0xb5, 0x31, 0xc5, 0x7e, 0x64, 0x4f, 0xb5,
        0xbc, 0x86, 0xf4, 0x54, 0xe2, 0x88, 0x32, 0xfa };
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
        notificationservice_api_sendreq_block_update(
            sock, alloc, offset, &block_id));

    /* clean up. */
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(alloc)));
}
#endif
