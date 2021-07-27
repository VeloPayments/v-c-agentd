/**
 * \file test_ipc_psock_compatibility.cpp
 *
 * \brief To eliminate IPC and replace it with RCPR psock, we need to ensure
 * that the IPC I/O routines are compatible with the psock wire format.
 *
 * \copyright 2021 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <gtest/gtest.h>
#include <rcpr/psock.h>

#include "test_ipc.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

using namespace std;

/**
 * \brief ipc_write_data_block can be read by psock_read_boxed_data.
 */
TEST(ipc_psock_compatibility, ipc_write_data_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    rcpr_allocator* alloc;
    psock* sock;
    char* buf;
    size_t size;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write a data value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_data_block(lhs, TEST_STRING, strlen(TEST_STRING)));

    /* read the data from the psock. */
    ASSERT_EQ(0, psock_read_boxed_data(sock, alloc, (void**)&buf, &size));

    /* the size should be correct. */
    EXPECT_EQ(strlen(TEST_STRING), size);

    /* the data should match. */
    EXPECT_EQ(0, memcmp(buf, TEST_STRING, size));

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_write_string_block can be read by psock_read_boxed_string.
 */
TEST(ipc_psock_compatibility, ipc_write_string_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    rcpr_allocator* alloc;
    psock* sock;
    char* buf;
    size_t size;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write a string value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_string_block(lhs, TEST_STRING));

    /* read the string from the psock. */
    ASSERT_EQ(0, psock_read_boxed_string(sock, alloc, &buf, &size));

    /* the size should be correct. */
    EXPECT_EQ(strlen(TEST_STRING), size);

    /* the data should match. */
    EXPECT_EQ(0, memcmp(buf, TEST_STRING, size));

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_write_uint64_block can be read by psock_read_boxed_uint64.
 */
TEST(ipc_psock_compatibility, ipc_write_uint64_block)
{
    int lhs, rhs;
    const uint64_t EXPECTED_VAL = 92837;
    rcpr_allocator* alloc;
    psock* sock;
    uint64_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, EXPECTED_VAL));

    /* read the integer from the psock. */
    ASSERT_EQ(0, psock_read_boxed_uint64(sock, &val));

    /* the data should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_write_int64_block can be read by psock_read_boxed_int64.
 */
TEST(ipc_psock_compatibility, ipc_write_int64_block)
{
    int lhs, rhs;
    const int64_t EXPECTED_VAL = 92837;
    rcpr_allocator* alloc;
    psock* sock;
    int64_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int64_block(lhs, EXPECTED_VAL));

    /* read the integer from the psock. */
    ASSERT_EQ(0, psock_read_boxed_int64(sock, &val));

    /* the data should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_write_uint8_block can be read by psock_read_boxed_uint8.
 */
TEST(ipc_psock_compatibility, ipc_write_uint8_block)
{
    int lhs, rhs;
    const uint8_t EXPECTED_VAL = 92;
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, EXPECTED_VAL));

    /* read the integer from the psock. */
    ASSERT_EQ(0, psock_read_boxed_uint8(sock, &val));

    /* the data should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_write_int8_block can be read by psock_read_boxed_int8.
 */
TEST(ipc_psock_compatibility, ipc_write_int8_block)
{
    int lhs, rhs;
    const int8_t EXPECTED_VAL = 92;
    rcpr_allocator* alloc;
    psock* sock;
    int8_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the rhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, rhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int8_block(lhs, EXPECTED_VAL));

    /* read the integer from the psock. */
    ASSERT_EQ(0, psock_read_boxed_int8(sock, &val));

    /* the data should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(lhs);
}

/**
 * \brief ipc_read_data_block can read something written by
 * psock_write_boxed_data.
 */
TEST(ipc_psock_compatibility, ipc_read_data_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    rcpr_allocator* alloc;
    psock* sock;
    char* buf;
    uint32_t size;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write a data value to the lhs socket. */
    ASSERT_EQ(
        0, psock_write_boxed_data(sock, TEST_STRING, strlen(TEST_STRING)));

    /* read the data from the rhs socket. */
    ASSERT_EQ(0, ipc_read_data_block(rhs, (void**)&buf, &size));

    /* the size should be correct. */
    EXPECT_EQ(strlen(TEST_STRING), size);

    /* the data should match. */
    EXPECT_EQ(0, memcmp(buf, TEST_STRING, size));

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}

/**
 * \brief ipc_read_string_block can read something written by
 * psock_write_boxed_string.
 */
TEST(ipc_psock_compatibility, ipc_read_string_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    rcpr_allocator* alloc;
    psock* sock;
    char* buf;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write a data value to the lhs socket. */
    ASSERT_EQ(0, psock_write_boxed_string(sock, TEST_STRING));

    /* read the data from the rhs socket. */
    ASSERT_EQ(0, ipc_read_string_block(rhs, &buf));

    /* the strings should match. */
    EXPECT_STREQ(TEST_STRING, buf);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}

/**
 * \brief ipc_read_uint64_block can read something written by
 * psock_write_boxed_uint64.
 */
TEST(ipc_psock_compatibility, ipc_read_uint64_block)
{
    int lhs, rhs;
    const uint64_t EXPECTED_VAL = 284374;
    rcpr_allocator* alloc;
    psock* sock;
    uint64_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, psock_write_boxed_uint64(sock, EXPECTED_VAL));

    /* read the integer from the rhs socket. */
    ASSERT_EQ(0, ipc_read_uint64_block(rhs, &val));

    /* the values should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}

/**
 * \brief ipc_read_int64_block can read something written by
 * psock_write_boxed_int64.
 */
TEST(ipc_psock_compatibility, ipc_read_int64_block)
{
    int lhs, rhs;
    const int64_t EXPECTED_VAL = 284374;
    rcpr_allocator* alloc;
    psock* sock;
    int64_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, psock_write_boxed_int64(sock, EXPECTED_VAL));

    /* read the integer from the rhs socket. */
    ASSERT_EQ(0, ipc_read_int64_block(rhs, &val));

    /* the values should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}

/**
 * \brief ipc_read_uint8_block can read something written by
 * psock_write_boxed_uint8.
 */
TEST(ipc_psock_compatibility, ipc_read_uint8_block)
{
    int lhs, rhs;
    const uint8_t EXPECTED_VAL = 28;
    rcpr_allocator* alloc;
    psock* sock;
    uint8_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, psock_write_boxed_uint8(sock, EXPECTED_VAL));

    /* read the integer from the rhs socket. */
    ASSERT_EQ(0, ipc_read_uint8_block(rhs, &val));

    /* the values should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}

/**
 * \brief ipc_read_int8_block can read something written by
 * psock_write_boxed_int8.
 */
TEST(ipc_psock_compatibility, ipc_read_int8_block)
{
    int lhs, rhs;
    const int8_t EXPECTED_VAL = 28;
    rcpr_allocator* alloc;
    psock* sock;
    int8_t val;

    /* create the allocator. */
    ASSERT_EQ(0, rcpr_malloc_allocator_create(&alloc));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* wrap the lhs socket as a psock. */
    ASSERT_EQ(0, psock_create_from_descriptor(&sock, alloc, lhs));

    /* write an integer value to the lhs socket. */
    ASSERT_EQ(0, psock_write_boxed_int8(sock, EXPECTED_VAL));

    /* read the integer from the rhs socket. */
    ASSERT_EQ(0, ipc_read_int8_block(rhs, &val));

    /* the values should match. */
    EXPECT_EQ(EXPECTED_VAL, val);

    /* clean up. */
    ASSERT_EQ(0, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, resource_release(rcpr_allocator_resource_handle(alloc)));
    close(rhs);
}
