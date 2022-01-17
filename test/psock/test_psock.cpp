/**
 * \file test_psock.cpp
 *
 * Test agentd psock methods.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <rcpr/socket_utilities.h>
#include <vpr/allocator.h>
#include <vpr/allocator/malloc_allocator.h>

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_socket_utilities;

/**
 * \brief We can read an authed packet from a psock instance.
 */
TEST(psock_test, psock_read_authed_data)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    constexpr size_t ENC_PAYLOAD_SIZE =
        sizeof(uint32_t) + /* type */
        sizeof(uint32_t) + /* size */
        32 + /* hmac */
        15; /* string length */
    char TEST_PAYLOAD[ENC_PAYLOAD_SIZE] = { 0 };
    uint64_t iv = 12345;
    rcpr_allocator* rcpr_alloc;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;

    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* initialize the malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create the rcpr allocator. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        rcpr_malloc_allocator_create(&rcpr_alloc));

    /* initialize the crypto suite. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1));

    /* create a socket pair for testing. */
    ASSERT_EQ(
        STATUS_SUCCESS,
        socket_utility_socketpair(
            AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create a psock instance for the rhs. */
    psock* sock;
    ASSERT_EQ(
        STATUS_SUCCESS,
        psock_create_from_descriptor(
            &sock, rcpr_alloc, rhs));

    /* create a key for the stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    ASSERT_EQ(
        0,
        vccrypt_buffer_init(
            &key, &alloc_opts, suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* create a stream cipher instance. */
    vccrypt_stream_context_t stream;
    ASSERT_EQ(0, vccrypt_suite_stream_init(&suite, &stream, &key));

    /* create a MAC instance. */
    vccrypt_mac_context_t mac;
    ASSERT_EQ(0, vccrypt_suite_mac_short_init(&suite, &mac, &key));

    /* create a MAC digest buffer. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    ASSERT_EQ(0,
        vccrypt_buffer_init(
            &digest, &alloc_opts, suite.mac_short_opts.mac_size));

    /* continue encryption from the current iv, offset 0. */
    ASSERT_EQ(
        0,
        vccrypt_stream_continue_encryption(&stream, &iv, sizeof(iv), 0));

    /* write the packet type to the buffer. */
    uint32_t type = htonl(AGENTD_PSOCK_BOXED_TYPE_AUTHED_PACKET);
    size_t offset = 0;
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, &type, sizeof(type), TEST_PAYLOAD, &offset));
    /* digest the packet type. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + offset - sizeof(type),
            sizeof(type)));

    /* write the payload size to the buffer. */
    uint32_t payload_size = htonl(15);
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, &payload_size, sizeof(payload_size), TEST_PAYLOAD,
            &offset));
    /* digest the payload size. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + offset - sizeof(payload_size),
            sizeof(payload_size)));

    /* write the payload to the buffer, skipping the hmac. */
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, TEST_STRING, 15, TEST_PAYLOAD + 32, &offset));
    /* digest the payload. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + 32 + offset - 15, 15));

    /* finalize the mac to the test payload. */
    ASSERT_EQ(0, vccrypt_mac_finalize(&mac, &digest));
    memcpy(
        TEST_PAYLOAD + sizeof(type) + sizeof(payload_size), digest.data,
        digest.size);

    /* write the payload to the lhs socket. */
    ASSERT_EQ((ssize_t)sizeof(TEST_PAYLOAD),
        write(lhs, TEST_PAYLOAD, sizeof(TEST_PAYLOAD)));

    /* read an authed packet from the rhs socket. */
    ASSERT_EQ(0,
        psock_read_authed_data(sock, iv, &str, &str_size, &suite, &key));

    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    dispose((disposable_t*)&mac);
    dispose((disposable_t*)&digest);
    dispose((disposable_t*)&stream);
    dispose((disposable_t*)&key);
    ASSERT_EQ(STATUS_SUCCESS, resource_release(psock_resource_handle(sock)));
    ASSERT_EQ(0, close(lhs));
    dispose((disposable_t*)&suite);
    ASSERT_EQ(
        STATUS_SUCCESS,
        resource_release(rcpr_allocator_resource_handle(rcpr_alloc)));
    dispose((disposable_t*)&alloc_opts);
}
