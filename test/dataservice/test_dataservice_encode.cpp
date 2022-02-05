/**
 * \file test_dataservice_encode.cpp
 *
 * Unit tests for encode methods in dataservice async_api.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <gtest/gtest.h>
#include <vpr/allocator/malloc_allocator.h>

#include "../../src/dataservice/dataservice_protocol_internal.h"

using namespace std;

RCPR_IMPORT_uuid;

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_artifact_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid artifact_id = { .data = {
        0x9b, 0x3a, 0x83, 0x4a, 0x2c, 0x10, 0x47, 0x3e,
        0x9f, 0xfb, 0xfd, 0xaa, 0xb1, 0x3c, 0x57, 0x74 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_artifact_get(
            nullptr, &alloc_opts, child, &artifact_id));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_artifact_get(
            &buffer, nullptr, child, &artifact_id));

    /* a NULL artifact id is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_artifact_get(
            &buffer, &alloc_opts, child, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_artifact_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_payload_artifact_read_t req;
    rcpr_uuid artifact_id = { .data = {
        0x9b, 0x3a, 0x83, 0x4a, 0x2c, 0x10, 0x47, 0x3e,
        0x9f, 0xfb, 0xfd, 0xaa, 0xb1, 0x3c, 0x57, 0x74 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_artifact_get(
            &buffer, &alloc_opts, child, &artifact_id));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_ARTIFACT_READ */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_ARTIFACT_READ, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_payload_artifact_read(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* the artifact id should match. */
    EXPECT_EQ(0, memcmp(req.artifact_id, &artifact_id, 16));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_block_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid block_id = { .data = {
        0x2e, 0x72, 0x67, 0x6d, 0xe0, 0xba, 0x4f, 0x34,
        0x8f, 0x57, 0x08, 0x14, 0x47, 0xd5, 0xf3, 0x1a } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_get(
            nullptr, &alloc_opts, child, &block_id, true));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_get(
            &buffer, nullptr, child, &block_id, true));

    /* a NULL artifact id is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_get(
            &buffer, &alloc_opts, child, nullptr, true));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_block_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_block_read_t req;
    rcpr_uuid block_id = { .data = {
        0x2e, 0x72, 0x67, 0x6d, 0xe0, 0xba, 0x4f, 0x34,
        0x8f, 0x57, 0x08, 0x14, 0x47, 0xd5, 0xf3, 0x1a } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_block_get(
            &buffer, &alloc_opts, child, &block_id, false));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_BLOCK_READ */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_READ, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_block_read(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* the block id should match. */
    EXPECT_EQ(0, memcmp(req.block_id, &block_id, 16));

    /* the read cert flag should match. */
    EXPECT_FALSE(req.read_cert);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_block_id_by_height_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint32_t child = 0x1234;
    const uint64_t height = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_id_by_height_get(
            nullptr, &alloc_opts, child, height));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_id_by_height_get(
            &buffer, nullptr, child, height));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_block_id_by_height_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_block_id_by_height_read_t req;
    const uint32_t child = 0x1234;
    const uint64_t height = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_block_id_by_height_get(
            &buffer, &alloc_opts, child, height));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_block_id_by_height_read(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* the height should match. */
    EXPECT_EQ(height, req.block_height);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_block_make)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid block_id = { .data = {
        0xff, 0x48, 0x92, 0xce, 0x51, 0x18, 0x49, 0x8b,
        0xac, 0xcf, 0x35, 0xb5, 0xf1, 0x96, 0xcb, 0xb9 } };
    const uint32_t child = 0x1234;
    const uint8_t block_cert[] = { 0x00, 0x01, 0x02, 0x03 };
    const uint32_t block_cert_size = sizeof(block_cert);

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_make(
            nullptr, &alloc_opts, child, &block_id, block_cert,
            block_cert_size));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_make(
            &buffer, nullptr, child, &block_id, block_cert, block_cert_size));

    /* a NULL block_id id is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_make(
            &buffer, &alloc_opts, child, nullptr, block_cert, block_cert_size));

    /* a NULL block_cert id is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_block_make(
            &buffer, &alloc_opts, child, &block_id, nullptr, block_cert_size));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_block_make_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_block_make_t req;
    rcpr_uuid block_id = { .data = {
        0xff, 0x48, 0x92, 0xce, 0x51, 0x18, 0x49, 0x8b,
        0xac, 0xcf, 0x35, 0xb5, 0xf1, 0x96, 0xcb, 0xb9 } };
    const uint32_t child = 0x1234;
    const uint8_t block_cert[] = { 0x00, 0x01, 0x02, 0x03 };
    const uint32_t block_cert_size = sizeof(block_cert);

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    EXPECT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_block_make(
            &buffer, &alloc_opts, child, &block_id, block_cert,
            block_cert_size));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_APP_BLOCK_WRITE */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_WRITE, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_block_make(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* the block id should match. */
    EXPECT_EQ(0, memcmp(&block_id, req.block_id, 16));

    /* the block cert size should match. */
    EXPECT_EQ(block_cert_size, req.cert_size);

    /* the block cert should match. */
    EXPECT_EQ(0, memcmp(block_cert, req.cert, req.cert_size));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_canonized_transaction_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0x23, 0x04, 0x8d, 0xa2, 0x35, 0xe7, 0x45, 0xec,
        0xba, 0xe6, 0xb3, 0x49, 0x22, 0xfa, 0x0a, 0x73 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_canonized_transaction_get(
            nullptr, &alloc_opts, child, &txn_id, true));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_canonized_transaction_get(
            &buffer, nullptr, child, &txn_id, true));

    /* a NULL artifact id is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_canonized_transaction_get(
            &buffer, &alloc_opts, child, nullptr, true));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_canonized_transaction_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_canonized_transaction_get_t req;
    rcpr_uuid txn_id = { .data = {
        0x23, 0x04, 0x8d, 0xa2, 0x35, 0xe7, 0x45, 0xec,
        0xba, 0xe6, 0xb3, 0x49, 0x22, 0xfa, 0x0a, 0x73 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_canonized_transaction_get(
            &buffer, &alloc_opts, child, &txn_id, false));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_TRANSACTION_READ */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_TRANSACTION_READ, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_canonized_transaction_get(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* the txn id should match. */
    EXPECT_EQ(0, memcmp(req.txn_id, &txn_id, 16));

    /* the read cert flag should match. */
    EXPECT_FALSE(req.read_cert);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_child_context_close)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_child_context_close(
            nullptr, &alloc_opts, child));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_child_context_close(
            &buffer, nullptr, child));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_child_context_close_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_child_context_close_t req;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_child_context_close(
            &buffer, &alloc_opts, child));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_child_context_close(
            breq, payload_size, &req));

    /* the child index should match. */
    EXPECT_EQ(child, req.hdr.child_index);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(dataservice_encode_test, request_child_context_create)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_child_context_create(
            nullptr, &alloc_opts, caps, sizeof(caps)));

    /* a NULL allocator is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_child_context_create(
            &buffer, nullptr, caps, sizeof(caps)));

    /* a NULL caps buffer is invalid. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_encode_request_child_context_create(
            &buffer, &alloc_opts, nullptr, sizeof(caps)));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(dataservice_encode_test, request_child_context_create_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_child_context_create_t req;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    malloc_allocator_options_init(&alloc_opts);

    /* set a random arbitrary bit in the capabilities. */
    BITCAP_INIT_FALSE(caps);
    BITCAP_SET_TRUE(caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE);

    /* the encode call should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_encode_request_child_context_create(
            &buffer, &alloc_opts, caps, sizeof(caps)));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    ASSERT_GE(buffer.size, sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE, method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    ASSERT_EQ(STATUS_SUCCESS,
        dataservice_decode_request_child_context_create(
            breq, payload_size, &req));

    /* the capabilities should match. */
    EXPECT_EQ(0, memcmp(caps, req.caps, sizeof(req.caps)));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}
