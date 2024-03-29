/**
 * \file test_dataservice_encode.cpp
 *
 * Unit tests for encode methods in dataservice async_api.
 *
 * \copyright 2022-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cstring>
#include <minunit/minunit.h>
#include <vpr/allocator/malloc_allocator.h>

#include "../../src/dataservice/dataservice_protocol_internal.h"

using namespace std;

RCPR_IMPORT_uuid;

TEST_SUITE(dataservice_encode_test);

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_artifact_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid artifact_id = { .data = {
        0x9b, 0x3a, 0x83, 0x4a, 0x2c, 0x10, 0x47, 0x3e,
        0x9f, 0xfb, 0xfd, 0xaa, 0xb1, 0x3c, 0x57, 0x74 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_artifact_get(
                    nullptr, &alloc_opts, child, &artifact_id));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_artifact_get(
                    &buffer, nullptr, child, &artifact_id));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_artifact_get(
                    &buffer, &alloc_opts, child, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_artifact_get_decoded)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_artifact_get(
                    &buffer, &alloc_opts, child, &artifact_id));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_ARTIFACT_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_ARTIFACT_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_payload_artifact_read(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the artifact id should match. */
    TEST_EXPECT(0 == memcmp(req.artifact_id, &artifact_id, 16));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_block_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid block_id = { .data = {
        0x2e, 0x72, 0x67, 0x6d, 0xe0, 0xba, 0x4f, 0x34,
        0x8f, 0x57, 0x08, 0x14, 0x47, 0xd5, 0xf3, 0x1a } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_get(
                    nullptr, &alloc_opts, child, &block_id, true));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_get(
                    &buffer, nullptr, child, &block_id, true));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_get(
                    &buffer, &alloc_opts, child, nullptr, true));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_block_get_decoded)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_block_get(
                    &buffer, &alloc_opts, child, &block_id, false));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_BLOCK_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_BLOCK_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_block_read(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the block id should match. */
    TEST_EXPECT(0 == memcmp(req.block_id, &block_id, 16));

    /* the read cert flag should match. */
    TEST_EXPECT(!req.read_cert);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_block_id_by_height_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint32_t child = 0x1234;
    const uint64_t height = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_id_by_height_get(
                    nullptr, &alloc_opts, child, height));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_id_by_height_get(
                    &buffer, nullptr, child, height));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_block_id_by_height_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_block_id_by_height_read_t req;
    const uint32_t child = 0x1234;
    const uint64_t height = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_block_id_by_height_get(
                    &buffer, &alloc_opts, child, height));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_block_id_by_height_read(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the height should match. */
    TEST_EXPECT(height == req.block_height);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_block_make)
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
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_make(
                    nullptr, &alloc_opts, child, &block_id, block_cert,
                    block_cert_size));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_make(
                    &buffer, nullptr, child, &block_id, block_cert,
                    block_cert_size));

    /* a NULL block_id id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_make(
                    &buffer, &alloc_opts, child, nullptr, block_cert,
                    block_cert_size));

    /* a NULL block_cert id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_block_make(
                    &buffer, &alloc_opts, child, &block_id, nullptr,
                    block_cert_size));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_block_make_decoded)
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
    TEST_EXPECT(
        STATUS_SUCCESS
            == dataservice_encode_request_block_make(
                    &buffer, &alloc_opts, child, &block_id, block_cert,
                    block_cert_size));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_APP_BLOCK_WRITE */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_BLOCK_WRITE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_block_make(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the block id should match. */
    TEST_EXPECT(0 == memcmp(&block_id, req.block_id, 16));

    /* the block cert size should match. */
    TEST_EXPECT(block_cert_size == req.cert_size);

    /* the block cert should match. */
    TEST_EXPECT(0 == memcmp(block_cert, req.cert, req.cert_size));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_canonized_transaction_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0x23, 0x04, 0x8d, 0xa2, 0x35, 0xe7, 0x45, 0xec,
        0xba, 0xe6, 0xb3, 0x49, 0x22, 0xfa, 0x0a, 0x73 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_canonized_transaction_get(
                    nullptr, &alloc_opts, child, &txn_id, true));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_canonized_transaction_get(
                    &buffer, nullptr, child, &txn_id, true));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_canonized_transaction_get(
                    &buffer, &alloc_opts, child, nullptr, true));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_canonized_transaction_get_decoded)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_canonized_transaction_get(
                    &buffer, &alloc_opts, child, &txn_id, false));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_TRANSACTION_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_TRANSACTION_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_canonized_transaction_get(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the txn id should match. */
    TEST_EXPECT(0 == memcmp(req.txn_id, &txn_id, 16));

    /* the read cert flag should match. */
    TEST_EXPECT(!req.read_cert);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_child_context_close)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_child_context_close(
                    nullptr, &alloc_opts, child));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_child_context_close(
                    &buffer, nullptr, child));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_child_context_close_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_child_context_close_t req;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_child_context_close(
                    &buffer, &alloc_opts, child));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE */
    TEST_ASSERT(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_child_context_close(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_child_context_create)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_child_context_create(
                    nullptr, &alloc_opts, caps, sizeof(caps)));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_child_context_create(
                    &buffer, nullptr, caps, sizeof(caps)));

    /* a NULL caps buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_child_context_create(
                    &buffer, &alloc_opts, nullptr, sizeof(caps)));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_child_context_create_decoded)
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
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_child_context_create(
                    &buffer, &alloc_opts, caps, sizeof(caps)));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE */
    TEST_ASSERT(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_child_context_create(
                    breq, payload_size, &req));

    /* the capabilities should match. */
    TEST_EXPECT(0 == memcmp(caps, req.caps, sizeof(req.caps)));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_global_settings_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    uint32_t child = 0x1234;
    uint64_t key = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_global_settings_get(
                    nullptr, &alloc_opts, child, key));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_global_settings_get(
                    &buffer, nullptr, child, key));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_global_settings_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_global_setting_get_t req;
    uint32_t child = 0x1234;
    uint64_t key = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_global_settings_get(
                    &buffer, &alloc_opts, child, key));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_global_setting_get(
                    breq, payload_size, &req));

    /* the child context should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the key should match. */
    TEST_EXPECT(key == req.key);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_global_settings_set)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint8_t val[] = { 0x01, 0x02, 0x03, 0x04 };
    uint32_t child = 0x1234;
    uint64_t key = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_global_settings_set(
                    nullptr, &alloc_opts, child, key, val, sizeof(val)));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_global_settings_set(
                    &buffer, nullptr, child, key, val, sizeof(val)));

    /* a NULL value is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_global_settings_set(
                    &buffer, &alloc_opts, child, key, nullptr, sizeof(val)));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_global_settings_set_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint8_t val[] = { 0x01, 0x02, 0x03, 0x04 };
    dataservice_request_global_setting_set_t req;
    uint32_t child = 0x1234;
    uint64_t key = 0x98765432;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_global_settings_set(
                    &buffer, &alloc_opts, child, key, val, sizeof(val)));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_global_setting_set(
                    breq, payload_size, &req));

    /* the child context should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the key should match. */
    TEST_EXPECT(key == req.key);

    /* the value size should match. */
    TEST_ASSERT(sizeof(val) == req.val_size);

    /* the value should match. */
    TEST_EXPECT(0 == memcmp(req.val, val, req.val_size));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_latest_block_id_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_latest_block_id_get(
                    nullptr, &alloc_opts, child));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_latest_block_id_get(
                    &buffer, nullptr, child));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_latest_block_id_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_block_id_latest_read_t req;
    uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_latest_block_id_get(
                    &buffer, &alloc_opts, child));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_block_id_latest_read(
                    breq, payload_size, &req));

    /* the child context should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_root_context_init)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    uint64_t max_database_size = 10 * 1024 * 1024;
    const char* datadir = "/data";

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_init(
                    nullptr, &alloc_opts, max_database_size, datadir));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_init(
                    &buffer, nullptr, max_database_size, datadir));

    /* a NULL data directory is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_init(
                    &buffer, &alloc_opts, max_database_size, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_root_context_init_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    uint64_t max_database_size = 10 * 1024 * 1024;
    const char* datadir = "/data";
    dataservice_request_payload_root_context_init_t req;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_root_context_init(
                    &buffer, &alloc_opts, max_database_size, datadir));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE */
    TEST_ASSERT(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_root_context_init(
                    breq, &alloc_opts, payload_size, &req));

    /* the max database size should match. */
    TEST_EXPECT(max_database_size == req.max_database_size);

    /* the data dir string should match. */
    TEST_EXPECT(!strcmp(datadir, req.datadir));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_root_context_reduce_caps)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_reduce_caps(
                    nullptr, &alloc_opts, caps, sizeof(caps)));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_reduce_caps(
                    &buffer, nullptr, caps, sizeof(caps)));

    /* a NULL capabilities pointer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_root_context_reduce_caps(
                    &buffer, &alloc_opts, nullptr, sizeof(caps)));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_root_context_reduce_caps_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_payload_root_context_reduce_caps_t req;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    BITCAP_INIT_TRUE(caps);

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_root_context_reduce_caps(
                    &buffer, &alloc_opts, caps, sizeof(caps)));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS */
    TEST_ASSERT(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_root_context_reduce_caps(
                    breq, payload_size, &req));

    /* the capabilities should match. */
    TEST_EXPECT(0 == memcmp(caps, req.caps, sizeof(req.caps)));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_transaction_drop)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0x28, 0x6b, 0xe0, 0x32, 0x82, 0x7d, 0x4e, 0xab,
        0x80, 0x42, 0xdf, 0x83, 0xe1, 0x50, 0xb3, 0xab } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_drop(
                    nullptr, &alloc_opts, child, &txn_id));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_drop(
                    &buffer, nullptr, child, &txn_id));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_drop(
                    &buffer, &alloc_opts, child, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_transaction_drop_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_transaction_drop_t req;
    rcpr_uuid txn_id = { .data = {
        0x28, 0x6b, 0xe0, 0x32, 0x82, 0x7d, 0x4e, 0xab,
        0x80, 0x42, 0xdf, 0x83, 0xe1, 0x50, 0xb3, 0xab } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_transaction_drop(
                    &buffer, &alloc_opts, child, &txn_id));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_transaction_drop(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the txn id should match. */
    TEST_EXPECT(0 == memcmp(req.txn_id, &txn_id, 16));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_transaction_get)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0x26, 0xdb, 0x11, 0x43, 0x69, 0x99, 0x48, 0x49,
        0xaf, 0x3a, 0xd8, 0xc6, 0x83, 0x36, 0x85, 0xb9 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_get(
                    nullptr, &alloc_opts, child, &txn_id));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_get(
                    &buffer, nullptr, child, &txn_id));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_get(
                    &buffer, &alloc_opts, child, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_transaction_get_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_transaction_get_t req;
    rcpr_uuid txn_id = { .data = {
        0x26, 0xdb, 0x11, 0x43, 0x69, 0x99, 0x48, 0x49,
        0xaf, 0x3a, 0xd8, 0xc6, 0x83, 0x36, 0x85, 0xb9 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_transaction_get(
                    &buffer, &alloc_opts, child, &txn_id));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_transaction_get(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the txn id should match. */
    TEST_EXPECT(0 == memcmp(req.txn_id, &txn_id, 16));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_transaction_get_first)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_get_first(
                    nullptr, &alloc_opts, child));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_get_first(
                    &buffer, nullptr, child));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_transaction_get_first_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_transaction_get_first_t req;
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_transaction_get_first(
                    &buffer, &alloc_opts, child));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_transaction_get_first(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_transaction_promote)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0x32, 0x64, 0x56, 0xe9, 0x8c, 0x37, 0x4a, 0x4b,
        0x9a, 0x91, 0x98, 0xc1, 0x60, 0x12, 0x9a, 0x97 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_promote(
                    nullptr, &alloc_opts, child, &txn_id));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_promote(
                    &buffer, nullptr, child, &txn_id));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_promote(
                    &buffer, &alloc_opts, child, nullptr));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_transaction_promote_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_transaction_promote_t req;
    rcpr_uuid txn_id = { .data = {
        0x32, 0x64, 0x56, 0xe9, 0x8c, 0x37, 0x4a, 0x4b,
        0x9a, 0x91, 0x98, 0xc1, 0x60, 0x12, 0x9a, 0x97 } };
    const uint32_t child = 0x1234;

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_encode_request_transaction_promote(
                    &buffer, &alloc_opts, child, &txn_id));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* the method should be DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_PROMOTE */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_PROMOTE == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_transaction_promote(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the txn id should match. */
    TEST_EXPECT(0 == memcmp(req.txn_id, &txn_id, 16));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the encode function performs parameter checks.
 */
TEST(request_transaction_submit)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    rcpr_uuid txn_id = { .data = {
        0xfc, 0x62, 0x81, 0xfb, 0xd6, 0x56, 0x48, 0xd6,
        0xa7, 0x40, 0x4f, 0xd5, 0x3b, 0xd8, 0x5c, 0x56 } };
    rcpr_uuid artifact_id = { .data = {
        0xbf, 0x6a, 0x49, 0x44, 0x3d, 0xcd, 0x44, 0x1b,
        0x93, 0x62, 0x0d, 0x07, 0xb5, 0x4d, 0x4d, 0x3d } };
    const uint32_t child = 0x1234;
    const uint8_t txn_cert[] = { 0x00, 0x01, 0x02, 0x03 };
    const uint32_t txn_cert_size = sizeof(txn_cert);

    malloc_allocator_options_init(&alloc_opts);

    /* a NULL buffer is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_submit(
                    nullptr, &alloc_opts, child, &txn_id, &artifact_id,
                    txn_cert, txn_cert_size));

    /* a NULL allocator is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_submit(
                    &buffer, nullptr, child, &txn_id, &artifact_id, txn_cert,
                    txn_cert_size));

    /* a NULL transaction id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_submit(
                    &buffer, &alloc_opts, child, nullptr, &artifact_id,
                    txn_cert, txn_cert_size));

    /* a NULL artifact id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_submit(
                    &buffer, &alloc_opts, child, &txn_id, nullptr, txn_cert,
                    txn_cert_size));

    /* a NULL transaction id is invalid. */
    TEST_EXPECT(
        AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER
            == dataservice_encode_request_transaction_submit(
                    &buffer, &alloc_opts, child, &txn_id, &artifact_id, nullptr,
                    txn_cert_size));

    /* clean up. */
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that the decoded values match the encoded values.
 */
TEST(request_transaction_submit_decoded)
{
    allocator_options_t alloc_opts;
    vccrypt_buffer_t buffer;
    dataservice_request_transaction_submit_t req;
    rcpr_uuid txn_id = { .data = {
        0xfc, 0x62, 0x81, 0xfb, 0xd6, 0x56, 0x48, 0xd6,
        0xa7, 0x40, 0x4f, 0xd5, 0x3b, 0xd8, 0x5c, 0x56 } };
    rcpr_uuid artifact_id = { .data = {
        0xbf, 0x6a, 0x49, 0x44, 0x3d, 0xcd, 0x44, 0x1b,
        0x93, 0x62, 0x0d, 0x07, 0xb5, 0x4d, 0x4d, 0x3d } };
    const uint32_t child = 0x1234;
    const uint8_t txn_cert[] = { 0x00, 0x01, 0x02, 0x03 };
    const uint32_t txn_cert_size = sizeof(txn_cert);

    malloc_allocator_options_init(&alloc_opts);

    /* the encode call should succeed. */
    TEST_EXPECT(
        STATUS_SUCCESS
            == dataservice_encode_request_transaction_submit(
                    &buffer, &alloc_opts, child, &txn_id, &artifact_id,
                    txn_cert, txn_cert_size));

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)buffer.data;

    /* the payload should be at least large enough for the method. */
    TEST_ASSERT(buffer.size >= sizeof(uint32_t));

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* method should be DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT */
    TEST_ASSERT(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT == method);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = buffer.size - sizeof(uint32_t);

    /* the decode should succeed. */
    TEST_ASSERT(
        STATUS_SUCCESS
            == dataservice_decode_request_transaction_submit(
                    breq, payload_size, &req));

    /* the child index should match. */
    TEST_EXPECT(child == req.hdr.child_index);

    /* the transaction id should match. */
    TEST_EXPECT(0 == memcmp(&txn_id, req.txn_id, 16));

    /* the artifact id should match. */
    TEST_EXPECT(0 == memcmp(&artifact_id, req.artifact_id, 16));

    /* the cert size should match. */
    TEST_EXPECT(txn_cert_size == req.cert_size);

    /* the cert should match. */
    TEST_EXPECT(0 == memcmp(txn_cert, req.cert, req.cert_size));

    /* clean up. */
    dispose((disposable_t*)&buffer);
    dispose((disposable_t*)&req);
    dispose((disposable_t*)&alloc_opts);
}
