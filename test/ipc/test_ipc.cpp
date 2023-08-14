/**
 * \file test_ipc.cpp
 *
 * Test ipc methods.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <minunit/minunit.h>
#include <stdint.h>
#include <sys/socket.h>
#include <time.h>
#include <vpr/disposable.h>

#include "test_ipc.h"

using namespace std;

TEST_SUITE(ipc_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    ipc_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * \brief Calling ipc_make_block on a socket should make it blocking.
 */
BEGIN_TEST_F(ipc_make_block)
    int flags;
    int lhs, rhs;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* set the lhs socket to non-blocking using fcntl. */
    flags = fcntl(lhs, F_GETFL);
    TEST_ASSERT(0 <= flags);
    flags |= O_NONBLOCK;
    TEST_ASSERT(0 <= fcntl(lhs, F_SETFL, flags));

    /* precondition: lhs is non-blocking. */
    flags = fcntl(lhs, F_GETFL);
    TEST_ASSERT(0 <= flags);
    TEST_ASSERT(O_NONBLOCK == (flags & O_NONBLOCK));

    /* set lhs socket to blocking. */
    TEST_ASSERT(0 == ipc_make_block(lhs));

    /* postcondition: lhs is blocking. */
    flags = fcntl(lhs, F_GETFL);
    TEST_ASSERT(0 <= flags);
    TEST_ASSERT(0 == (flags & O_NONBLOCK));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write a string value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_string_block)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char buf[100];
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_string_block(lhs, TEST_STRING));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_STRING. */
    TEST_EXPECT(IPC_DATA_TYPE_STRING == ntohl(type));

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    TEST_ASSERT(sizeof(nsize) == (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    uint32_t size = ntohl(nsize);

    /* size should be the length of the string. */
    TEST_EXPECT(strlen(TEST_STRING) == size);

    /* clear the buffer. */
    memset(buf, 0, sizeof(buf));

    /* read the string from the rhs socket. */
    TEST_ASSERT(size == (size_t)read(rhs, buf, size));

    /* the string value should match. */
    TEST_EXPECT(!strcmp(TEST_STRING, buf));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write a data value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_data_block)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char buf[100];
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a data block to the lhs socket. */
    TEST_ASSERT(
        0 == ipc_write_data_block(lhs, TEST_STRING, strlen(TEST_STRING)));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_DATA_PACKET. */
    TEST_EXPECT(IPC_DATA_TYPE_DATA_PACKET == ntohl(type));

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    TEST_ASSERT(sizeof(nsize) == (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    uint32_t size = ntohl(nsize);

    /* size should be the length of the string. */
    TEST_EXPECT(strlen(TEST_STRING) == size);

    /* clear the buffer. */
    memset(buf, 0, sizeof(buf));

    /* read the string from the rhs socket. */
    TEST_ASSERT(size == (size_t)read(rhs, buf, size));

    /* the string value should match. */
    TEST_EXPECT(!strcmp(TEST_STRING, buf));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write a uint64_t value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_uint64_block)
    int lhs, rhs;
    const uint64_t TEST_VAL = 98872;
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_UINT64. */
    TEST_EXPECT(IPC_DATA_TYPE_UINT64 == ntohl(type));

    /* read the uint64_t value from the stream. */
    uint64_t nval = 0;
    TEST_ASSERT(sizeof(nval) == (size_t)read(rhs, &nval, sizeof(nval)));

    /* swap the value to local endianness. */
    uint64_t val = ntohll(nval);

    /* the value should equal our original value. */
    TEST_EXPECT(TEST_VAL == val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write an int64_t value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_int64_block)
    int lhs, rhs;
    const int64_t TEST_VAL = -98872;
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int64_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_INT64. */
    TEST_EXPECT(IPC_DATA_TYPE_INT64 == ntohl(type));

    /* read the int64_t value from the stream. */
    int64_t nval = 0;
    TEST_ASSERT(sizeof(nval) == (size_t)read(rhs, &nval, sizeof(nval)));

    /* swap the value to local endianness. */
    int64_t val = ntohll(nval);

    /* the value should equal our original value. */
    TEST_EXPECT(TEST_VAL == val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write a uint8_t value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_uint8_block)
    int lhs, rhs;
    const uint8_t TEST_VAL = 76;
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint8_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_UINT8. */
    TEST_EXPECT(IPC_DATA_TYPE_UINT8 == ntohl(type));

    /* read the uint8_t value from the stream. */
    uint8_t val = 0;
    TEST_ASSERT(sizeof(val) == (size_t)read(rhs, &val, sizeof(val)));

    /* the value should equal our original value. */
    TEST_EXPECT(TEST_VAL == val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to write an int8_t value to a blocking socket.
 */
BEGIN_TEST_F(ipc_write_int8_block)
    int lhs, rhs;
    const int8_t TEST_VAL = -76;
    uint32_t type;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int8_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    TEST_ASSERT(sizeof(type) == read(rhs, &type, sizeof(type)));

    /* the type should be IPC_DATA_TYPE_INT8. */
    TEST_EXPECT(IPC_DATA_TYPE_INT8 == ntohl(type));

    /* read the int8_t value from the stream. */
    int8_t val = 0;
    TEST_ASSERT(sizeof(val) == (size_t)read(rhs, &val, sizeof(val)));

    /* the value should equal our original value. */
    TEST_EXPECT(TEST_VAL == val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a string value from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_string_block_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char* str = nullptr;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_string_block(lhs, TEST_STRING));

    /* read a string block from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_string_block(rhs, &str));

    /* the string is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string is a copy of the test string. */
    TEST_EXPECT(!strcmp(TEST_STRING, str));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a string, fail.
 */
BEGIN_TEST_F(ipc_read_string_block_bad_type)
    int lhs, rhs;
    uint64_t badval = 1;
    char* str = nullptr;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, badval));

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the connection is reset before reading type, return an error.
 */
BEGIN_TEST_F(ipc_read_string_block_reset_connection_1)
    int lhs, rhs;
    char* str = nullptr;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* reset the peer connection. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the size is not read, fail.
 */
BEGIN_TEST_F(ipc_read_string_block_bad_size)
    int lhs, rhs;
    char* str = nullptr;
    uint8_t type = IPC_DATA_TYPE_STRING;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the string type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the string is not read, fail.
 */
BEGIN_TEST_F(ipc_read_string_block_bad_data)
    int lhs, rhs;
    char* str = nullptr;
    uint8_t type = IPC_DATA_TYPE_STRING;
    uint32_t size = htonl(10);

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the string type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the string size to the lhs socket. */
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a data packet from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_data_block_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(
        0 == ipc_write_data_block(lhs, TEST_STRING, strlen(TEST_STRING)));

    /* read a data packet from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_data_block(rhs, &str, &str_size));

    /* the data is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string size is the length of our string. */
    TEST_ASSERT(strlen(TEST_STRING) == str_size);

    /* the data is a copy of the test string. */
    TEST_EXPECT(0 == memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a uint64_t value from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_uint64_block_success)
    int lhs, rhs;
    uint64_t val = 910028;
    uint64_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, val));

    /* read a uint64_t block from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_uint64_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a uint64_t, fail.
 */
BEGIN_TEST_F(ipc_read_uint64_block_bad_type)
    int lhs, rhs;
    uint8_t badval = 1U;
    uint64_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint8_block(lhs, badval));

    /* reading a uint64 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the peer socket is reset before the type is written, return an
 * error.
 */
BEGIN_TEST_F(ipc_read_uint64_reset_connection_1)
    int lhs, rhs;
    uint64_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint64 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the peer socket is reset before the size is written, return an
 * error.
 */
BEGIN_TEST_F(ipc_read_uint64_reset_connection_2)
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the size is invalid, return an error.
 */
BEGIN_TEST_F(ipc_read_uint64_block_bad_size)
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;
    uint32_t size = htonl(99);

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64_t type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint64_t size to the lhs socket. */
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the value is not read, fail.
 */
BEGIN_TEST_F(ipc_read_uint64_block_bad_data)
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64_t type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint64_t size to the lhs socket. */
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a int64_t value from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_int64_block_success)
    int lhs, rhs;
    int64_t val = -910028;
    int64_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a int64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int64_block(lhs, val));

    /* read a int64_t block from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_int64_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the peer connection is reset before the type is written, then
 * return an error.
 */
BEGIN_TEST_F(ipc_read_int64_block_reset_connection_1)
    int lhs, rhs;
    int64_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the connection before writing the type. */
    close(lhs);

    /* reading a int64 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a int64_t, fail.
 */
BEGIN_TEST_F(ipc_read_int64_block_bad_type)
    int lhs, rhs;
    uint8_t badval = 1U;
    int64_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint8_block(lhs, badval));

    /* reading a int64 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the connection is closed before the size is written, return an
 * error.
 */
BEGIN_TEST_F(ipc_read_int64_block_reset_connection_2)
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If a bad size is given, return an error.
 */
BEGIN_TEST_F(ipc_read_int64_block_bad_size)
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write a bad size to the lhs socket. */
    uint32_t size = htonl(99);
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the connection is closed before the data is written, return an
 * error.
 */
BEGIN_TEST_F(ipc_read_int64_block_reset_connection_3)
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write a valid size. */
    uint32_t size = htonl(sizeof(int64_t));
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a uint8_t value from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_uint8_block_success)
    int lhs, rhs;
    uint8_t val = 28;
    uint8_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint8_block(lhs, val));

    /* read a uint8_t block from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_uint8_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a uint8_t, fail.
 */
BEGIN_TEST_F(ipc_read_uint8_block_bad_type)
    int lhs, rhs;
    uint64_t badval = 1U;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, badval));

    /* reading a uint8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the socket connection is reset prior to reading the type, return an
 * error.
 */
BEGIN_TEST_F(ipc_read_uint8_reset_connection_1)
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the size is not read, fail.
 */
BEGIN_TEST_F(ipc_read_uint8_block_bad_size)
    int lhs, rhs;
    uint8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT8;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint8 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint8_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the socket connection is reset prior to reading the value, return
 * an error.
 */
BEGIN_TEST_F(ipc_read_uint8_reset_connection_2)
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the type. */
    uint8_t type = IPC_DATA_TYPE_UINT8;
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the size is invalid, return an error.
 */
BEGIN_TEST_F(ipc_read_uint8_bad_size)
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the type. */
    uint8_t type = IPC_DATA_TYPE_UINT8;
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the size. */
    uint32_t size = htonl(12);
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the value is not read, fail.
 */
BEGIN_TEST_F(ipc_read_uint8_block_bad_data)
    int lhs, rhs;
    uint8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT8;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint8_t type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint8_t size to the lhs socket. */
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint8_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a int8_t value from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_int8_block_success)
    int lhs, rhs;
    int8_t val = 28;
    int8_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a int8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int8_block(lhs, val));

    /* read a int8_t block from the rhs socket. */
    TEST_ASSERT(0 == ipc_read_int8_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the peer connection is reset, the int8 read fails.
 */
BEGIN_TEST_F(ipc_read_int8_block_reset_connection_1)
    int lhs, rhs;
    int8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a int8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a int8_t, fail.
 */
BEGIN_TEST_F(ipc_read_int8_block_bad_type)
    int lhs, rhs;
    uint64_t badval = 1U;
    int8_t read_val = 0U;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, badval));

    /* reading a int8 block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the peer connection is reset prior to writing size, an error code
 * is returned.
 */
BEGIN_TEST_F(ipc_read_int8_reset_connection_2)
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the size is invalid, return an error.
 */
BEGIN_TEST_F(ipc_read_int8_bad_size)
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8 type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write a bad size. */
    uint32_t size = htonl(12);
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the value is not read, fail.
 */
BEGIN_TEST_F(ipc_read_int8_block_bad_data)
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8_t type to the lhs socket. */
    TEST_ASSERT(sizeof(type) == (size_t)write(lhs, &type, sizeof(type)));

    /* write the int8_t size to the lhs socket. */
    TEST_ASSERT(sizeof(size) == (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If another value is seen instead of a data packet, fail.
 */
BEGIN_TEST_F(ipc_read_data_block_bad_type)
    int lhs, rhs;
    uint64_t badval = 1;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, badval));

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief If the socket is closed before a data block is written, it fails.
 */
BEGIN_TEST_F(ipc_read_data_block_connection_reset_1)
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the socket is closed in the middle of a write, reading fails.
 */
BEGIN_TEST_F(ipc_read_data_block_connection_reset_2)
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the packet type to the socket. */
    const uint8_t type = IPC_DATA_TYPE_DATA_PACKET;
    TEST_ASSERT(1 == write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief If the socket is closed in the middle of a write, reading fails.
 */
BEGIN_TEST_F(ipc_read_data_block_connection_reset_3)
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the packet type to the socket. */
    const uint8_t type = IPC_DATA_TYPE_DATA_PACKET;
    TEST_ASSERT(1 == write(lhs, &type, sizeof(type)));

    /* write the packet length to the socket. */
    uint32_t packet_len = htonl(10);
    TEST_ASSERT(4 == write(lhs, &packet_len, sizeof(packet_len)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    TEST_ASSERT(0 != ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    TEST_ASSERT(nullptr == str);

    /* clean up. */
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a uint8_t value from a non-blocking socket.
 */
BEGIN_TEST_F(ipc_read_uint8_noblock_success)
    int lhs, rhs;
    uint8_t val = 28;
    uint8_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint8_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    fixture.nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_uint8_noblock(
                        &fixture.nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == read_resp);
    /* we read a valid uint8_t. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read an int8_t value from a non-blocking socket.
 */
BEGIN_TEST_F(ipc_read_int8_noblock_success)
    int lhs, rhs;
    int8_t val = 28;
    int8_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write an int8_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int8_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    fixture.nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_int8_noblock(
                        &fixture.nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == read_resp);
    /* we read a valid uint8_t. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a uint64_t value from a non-blocking socket.
 */
BEGIN_TEST_F(ipc_read_uint64_noblock_success)
    int lhs, rhs;
    uint64_t val = 28;
    uint64_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_uint64_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    fixture.nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_uint64_noblock(
                        &fixture.nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == read_resp);
    /* we read a valid uint8_t. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read a int64_t value from a non-blocking socket.
 */
BEGIN_TEST_F(ipc_read_int64_noblock_success)
    int lhs, rhs;
    int64_t val = 28;
    int64_t read_val = 0;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write an int64_t block to the lhs socket. */
    TEST_ASSERT(0 == ipc_write_int64_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    fixture.nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_int64_noblock(
                        &fixture.nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    TEST_ASSERT(AGENTD_STATUS_SUCCESS == read_resp);
    /* we read a valid uint8_t. */
    TEST_EXPECT(val == read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
END_TEST_F()

/**
 * \brief It is possible to read an authed packet from a blocking socket.
 */
BEGIN_TEST_F(ipc_read_authed_block_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    constexpr size_t ENC_PAYLOAD_SIZE =
        sizeof(uint32_t) +  //type
        sizeof(uint32_t) +  //size
        32 +  //hmac
        15;  //string length
    char TEST_PAYLOAD[ENC_PAYLOAD_SIZE] = { 0 };
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    TEST_ASSERT(
        0
            == vccrypt_buffer_init(
                    &key, &fixture.alloc_opts,
                    fixture.suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* create a stream cipher instance. */
    vccrypt_stream_context_t stream;
    TEST_ASSERT(0 == vccrypt_suite_stream_init(&fixture.suite, &stream, &key));

    /* create a MAC instance. */
    vccrypt_mac_context_t mac;
    TEST_ASSERT(0 == vccrypt_suite_mac_short_init(&fixture.suite, &mac, &key));

    /* create a MAC digest buffer. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    TEST_ASSERT(
        0
            == vccrypt_buffer_init(
                    &digest, &fixture.alloc_opts,
                    fixture.suite.mac_short_opts.mac_size));

    /* continue encryption from the current iv, offset 0. */
    TEST_ASSERT(
        0
            == vccrypt_stream_continue_encryption(
                    &stream, &iv, sizeof(iv), 0));

    /* write the packet type to the buffer. */
    uint32_t type = htonl(IPC_DATA_TYPE_AUTHED_PACKET);
    size_t offset = 0;
    TEST_ASSERT(
        0
            == vccrypt_stream_encrypt(
                    &stream, &type, sizeof(type), TEST_PAYLOAD, &offset));
    /* digest the packet type. */
    TEST_ASSERT(
        0
            == vccrypt_mac_digest(
                    &mac, (const uint8_t*)TEST_PAYLOAD + offset - sizeof(type),
                    sizeof(type)));

    /* write the payload size to the buffer. */
    uint32_t payload_size = htonl(15);
    TEST_ASSERT(
        0
            == vccrypt_stream_encrypt(
                    &stream, &payload_size, sizeof(payload_size), TEST_PAYLOAD,
                    &offset));
    /* digest the payload size. */
    TEST_ASSERT(
        0
            == vccrypt_mac_digest(
                    &mac, (const uint8_t*)TEST_PAYLOAD + offset
                                - sizeof(payload_size),
                    sizeof(payload_size)));

    /* write the payload to the buffer, skipping the hmac. */
    TEST_ASSERT(
        0
            == vccrypt_stream_encrypt(
                    &stream, TEST_STRING, 15, TEST_PAYLOAD + 32, &offset));
    /* digest the payload. */
    TEST_ASSERT(
        0
            == vccrypt_mac_digest(
                    &mac, (const uint8_t*)TEST_PAYLOAD + 32 + offset - 15, 15));

    /* finalize the mac to the test payload. */
    TEST_ASSERT(0 == vccrypt_mac_finalize(&mac, &digest));
    memcpy(
        TEST_PAYLOAD + sizeof(type) + sizeof(payload_size), digest.data,
        digest.size);

    /* write the payload to the lhs socket. */
    TEST_ASSERT(
        (ssize_t)sizeof(TEST_PAYLOAD)
            == write(lhs, TEST_PAYLOAD, sizeof(TEST_PAYLOAD)));

    /* read an authed packet from the rhs socket. */
    TEST_ASSERT(
        0
            == ipc_read_authed_data_block(
                    rhs, iv, &str, &str_size, &fixture.suite, &key));

    /* the data is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string size is the length of our string. */
    TEST_ASSERT(strlen(TEST_STRING) == str_size);

    /* the data is a copy of the test string. */
    TEST_EXPECT(0 == memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
    dispose((disposable_t*)&stream);
    dispose((disposable_t*)&mac);
    dispose((disposable_t*)&digest);
END_TEST_F()

/**
 * \brief It is possible to read an authed packet from a blocking socket that
 * was written by ipc_write_authed_block.
 */
BEGIN_TEST_F(ipc_write_authed_block_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    TEST_ASSERT(
        0
            == vccrypt_buffer_init(
                    &key, &fixture.alloc_opts,
                    fixture.suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* writing to the socket should succeed. */
    TEST_ASSERT(
        0
            == ipc_write_authed_data_block(
                    lhs, iv, TEST_STRING, strlen(TEST_STRING),
                    &fixture.suite, &key));

    /* read an authed packet from the rhs socket. */
    TEST_ASSERT(
        0
            == ipc_read_authed_data_block(
                    rhs, iv, &str, &str_size, &fixture.suite, &key));

    /* the data is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string size is the length of our string. */
    TEST_ASSERT(strlen(TEST_STRING) == str_size);

    /* the data is a copy of the test string. */
    TEST_EXPECT(0 == memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
END_TEST_F()

/**
 * \brief It is possible to read an authed packet from a non-blocking socket
 * that was written by ipc_write_authed_block.
 */
BEGIN_TEST_F(ipc_read_authed_noblock_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    TEST_ASSERT(
        0
            == vccrypt_buffer_init(
                    &key, &fixture.alloc_opts,
                    fixture.suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* writing to the socket should succeed. */
    TEST_ASSERT(
        0
            == ipc_write_authed_data_block(
                    lhs, iv, TEST_STRING, strlen(TEST_STRING), &fixture.suite,
                    &key));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* read an authed packet from the rhs socket. */
    fixture.nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_authed_data_noblock(
                        &fixture.nonblockdatasock, iv, &str, &str_size,
                        &fixture.suite, &key);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    TEST_ASSERT(0 == read_resp);
    /* the data is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string size is the length of our string. */
    TEST_ASSERT(strlen(TEST_STRING) == str_size);

    /* the data is a copy of the test string. */
    TEST_EXPECT(0 == memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
END_TEST_F()

/**
 * \brief It is possible to write a packet via ipc_write_authed_noblock and read
 * it using ipc_read_authed_block.
 */
BEGIN_TEST_F(ipc_write_authed_noblock_success)
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    TEST_ASSERT(
        0
            == vccrypt_buffer_init(
                    &key, &fixture.alloc_opts,
                    fixture.suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    int write_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* writing to the socket should succeed. */
    fixture.nonblockmode(
        lhs,
        /* onRead */
        [&]() {
        },
        /* onWrite */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == write_resp)
            {
                write_resp =
                    ipc_write_authed_data_noblock(
                        &fixture.nonblockdatasock, iv, TEST_STRING,
                        strlen(TEST_STRING), &fixture.suite, &key);
            }
            else
            {
                if (ipc_socket_writebuffer_size(&fixture.nonblockdatasock) > 0)
                {
                    int bytes_written =
                        ipc_socket_write_from_buffer(&fixture.nonblockdatasock);

                    if (bytes_written == 0
                      || (bytes_written < 0
                            && (errno != EAGAIN && errno != EWOULDBLOCK)))
                    {
                        ipc_exit_loop(&fixture.loop);
                    }
                }
                else
                {
                    ipc_exit_loop(&fixture.loop);
                }
            }
        });
    /* the write should have succeeded. */
    TEST_ASSERT(0 == write_resp);

    /* read an authed packet from the rhs socket. */
    TEST_ASSERT(
        0
            == ipc_read_authed_data_block(
                    rhs, iv, &str, &str_size, &fixture.suite, &key));
    /* the data is valid. */
    TEST_ASSERT(nullptr != str);

    /* the string size is the length of our string. */
    TEST_ASSERT(strlen(TEST_STRING) == str_size);

    /* the data is a copy of the test string. */
    TEST_EXPECT(0 == memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
END_TEST_F()

static void test_timer_cb(ipc_timer_context_t*, void* user_context)
{
    function<void()>* func = (function<void()>*)user_context;

    (*func)();
}

/**
 * \brief It is possible to create a timer and have it fire.
 */
BEGIN_TEST_F(ipc_timer)
    int lhs, rhs;
    bool callback_called = false;
    timespec start_time;
    timespec callback_time;
    timespec expected_time;
    ipc_timer_context_t timer;

    function<void()> callback = [&]() {
        TEST_ASSERT(0 == clock_gettime(CLOCK_REALTIME, &callback_time));
        callback_called = true;
    };

    /* create a socket pair for testing. */
    TEST_ASSERT(0 == ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* set up the loop, using one of the sockets as a hack. */
    fixture.timermode_setup(lhs);

    /* initialize the timer event. */
    TEST_ASSERT(0 == ipc_timer_init(&timer, 250, &test_timer_cb, &callback));

    /* add the timer to the loop. */
    TEST_ASSERT(0 == ipc_event_loop_add_timer(&fixture.loop, &timer));

    /* get the current time. */
    TEST_ASSERT(0 == clock_gettime(CLOCK_REALTIME, &start_time));

    /* run the loop. */
    fixture.timermode();

    /* verify that the callback was called. */
    TEST_ASSERT(callback_called);

    /* we expect the callback to happen at least 250 milliseconds after
     * start_time. */
    memcpy(&expected_time, &start_time, sizeof(timespec));
    expected_time.tv_nsec += 250 * 1000 * 1000;
    expected_time.tv_sec += expected_time.tv_nsec / (1000 * 1000 * 1000);
    expected_time.tv_nsec %= 1000 * 1000 * 1000;

    /* the callback time should be greater than or equal to the expected time.*/
    TEST_EXPECT(
        ((callback_time.tv_sec == expected_time.tv_sec)
                ? (callback_time.tv_nsec >= expected_time.tv_nsec)
                : (callback_time.tv_sec >= expected_time.tv_sec)));

    /* reset for a second run. */
    callback_called = false;

    /* run again. */
    fixture.timermode();

    /* a timer is a single-shot timer. */
    TEST_EXPECT(!callback_called);

    /* tear down the loop. */
    fixture.timermode_teardown();

    /* clean up. */
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&timer);
END_TEST_F()
