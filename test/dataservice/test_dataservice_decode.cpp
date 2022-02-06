/**
 * \file test_dataservice_decode.cpp
 *
 * Unit tests for decode methods in dataservice async_api.
 *
 * \copyright 2019-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <gtest/gtest.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_protocol_internal.h"

using namespace std;

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, request_root_context_init_bad_sizes)
{
    uint8_t req[100] = { 0 };
    dataservice_request_payload_root_context_init_t dreq;
    allocator_options_t alloc_opts;

    malloc_allocator_options_init(&alloc_opts);

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE,
        dataservice_decode_request_root_context_init(
            req, &alloc_opts, 0, &dreq));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE,
        dataservice_decode_request_root_context_init(
            req, &alloc_opts,
            8 /* must have at least one byte for data dir. */, &dreq));

    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, request_root_context_init_null_checks)
{
    uint8_t req[100] = { 0 };
    dataservice_request_payload_root_context_init_t dreq;
    allocator_options_t alloc_opts;

    malloc_allocator_options_init(&alloc_opts);

    /* a null request packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_decode_request_root_context_init(
            nullptr, &alloc_opts, 25, &dreq));

    /* a null allocator pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_decode_request_root_context_init(
            req, nullptr, 25, &dreq));

    /* a null decoded request structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_decode_request_root_context_init(
            req, &alloc_opts, 25, nullptr));

    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that a request packet payload is successfully decoded.
 */
TEST(dataservice_decode_test, request_root_context_init_decoded)
{
    uint8_t req[13] = {
        /* size == 16383 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF,

        /* datadir == "/data" */
        '/', 'd', 'a', 't', 'a'
    };
    dataservice_request_payload_root_context_init_t dreq;
    allocator_options_t alloc_opts;

    malloc_allocator_options_init(&alloc_opts);

    /* a valid request is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_request_root_context_init(
            req, &alloc_opts, sizeof(req), &dreq));

    /* the size is correct. */
    ASSERT_EQ(16383UL, dreq.max_database_size);
    /* the data directory is correct. */
    ASSERT_STREQ("/data", dreq.datadir);

    dispose((disposable_t*)&dreq);
    dispose((disposable_t*)&alloc_opts);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, request_root_context_reduce_caps_sizes)
{
    uint8_t req[100] = { 0 };
    dataservice_request_payload_root_context_reduce_caps_t dreq;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE,
        dataservice_decode_request_root_context_reduce_caps(
            req, 0, &dreq));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE,
        dataservice_decode_request_root_context_reduce_caps(
            req, 2, &dreq));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, request_root_context_reduce_caps_null_checks)
{
    uint8_t req[100] = { 0 };
    dataservice_request_payload_root_context_reduce_caps_t dreq;

    /* a null request packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_decode_request_root_context_reduce_caps(
            nullptr, sizeof(dreq.caps), &dreq));

    /* a null decoded request structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER,
        dataservice_decode_request_root_context_reduce_caps(
            req, sizeof(dreq.caps), nullptr));
}

/**
 * Test that a request packet payload is successfully decoded.
 */
TEST(dataservice_decode_test, request_root_context_reduce_caps_decoded)
{
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);
    dataservice_request_payload_root_context_reduce_caps_t dreq;

    BITCAP_INIT_TRUE(caps);

    /* a valid request is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_request_root_context_reduce_caps(
            &caps, sizeof(caps), &dreq));

    /* the caps match. */
    EXPECT_EQ(0, memcmp(&caps, &dreq.caps, sizeof(caps)));

    /* clean up. */
    dispose((disposable_t*)&dreq);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_root_context_init_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_init_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_root_context_init_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_init_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_init(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_init(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_root_context_init_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_init_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_root_context_init(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_root_context_init_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_init_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_root_context_init(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_reduce_caps(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x01,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_root_context_reduce_caps(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_child_context_create_bad_sizes)
{
    uint8_t resp[100] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x02,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x00000000 */
        0x00, 0x00, 0x00, 0x00
    };
    dataservice_response_child_context_create_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_create(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE,
        dataservice_decode_response_child_context_create(
            resp, 3 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_child_context_create_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_create_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_create(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_create(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_context_create_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_create_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_child_context_create(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_child_context_create_decoded)
{
    uint8_t resp[16] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x02,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x00000000 */
        0x00, 0x00, 0x00, 0x00,

        /* child index == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_create_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_child_context_create(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status code is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the child index is correct. */
    ASSERT_EQ(0x12345678U, dresp.child);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_child_context_close_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_close_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_child_context_close_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_close_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_close(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_close(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_context_close_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_close_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_child_context_close(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_child_context_close_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x03,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_close_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_child_context_close(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_global_settings_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_global_settings_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_global_settings_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_global_settings_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_global_settings_get_decoded)
{
    uint8_t resp[15] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x07,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS */
        0x00, 0x00, 0x00, 0x00,

        /* global setting data. */
        0x01, 0x02, 0x03
    };
    dataservice_response_global_settings_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_global_settings_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the data pointer should be set correctly. */
    ASSERT_EQ(resp + 12, dresp.data);
    /* the data_size should be correct. */
    ASSERT_EQ(3U, dresp.data_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_global_settings_set_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_set_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_global_settings_set_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_set_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_set(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_set(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_global_settings_set_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_set_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_global_settings_set(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_global_settings_set_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x08,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_set_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_global_settings_set(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_submit_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_submit_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_submit_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_submit_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_submit(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_submit(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_submit_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_submit_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_submit(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_submit_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0F,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_submit_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_submit(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_get_first_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_first_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get_first(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get_first(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_get_first_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_first_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get_first(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get_first(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_get_first_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_get_first_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x11,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_transaction_get_first_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    uint8_t resp[84] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x11,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* node.net_txn_state */
        0x00, 0x00, 0x00, 0x01,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the node net state should match */
    ASSERT_EQ(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED,
        ntohl(dresp.node.net_txn_state));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 80, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x12,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_transaction_get_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    uint8_t resp[84] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x12,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* node.net_txn_state */
        0x00, 0x00, 0x00, 0x01,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node state should match. */
    ASSERT_EQ(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED,
        ntohl(dresp.node.net_txn_state));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 80, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_canonized_transaction_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_canonized_transaction_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_canonized_transaction_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_canonized_transaction_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test,
    response_canonized_transaction_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0E,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_canonized_transaction_get_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    const uint8_t EXPECTED_NODE_BLOCK_ID[] = {
        0x43, 0x9b, 0xd7, 0xe6, 0xd9, 0xea, 0x43, 0x78,
        0x97, 0x6a, 0xa3, 0x6e, 0x9b, 0x22, 0x0a, 0xbd
    };

    uint8_t resp[100] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0E,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* node.block_id */
        0x43, 0x9b, 0xd7, 0xe6, 0xd9, 0xea, 0x43, 0x78,
        0x97, 0x6a, 0xa3, 0x6e, 0x9b, 0x22, 0x0a, 0xbd,

        /* transaction state. */
        0x00, 0x00, 0x00, 0x01,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node block_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_BLOCK_ID, dresp.node.block_id, 16));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the node state should match. */
    ASSERT_EQ(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED,
        ntohl(dresp.node.net_txn_state));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 96, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_drop_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_drop_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_drop_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_drop_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_drop(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_drop(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_drop_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_drop_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_drop(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_drop_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x13,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_drop_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_drop(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_promote_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_promote_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_promote(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_promote(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_promote(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_promote_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_promote_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_promote(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_promote(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_promote_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_promote_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_promote(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_promote_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x10,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_promote_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_promote(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_PROMOTE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_block_make_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_make_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_block_make_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_make_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_make(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_make(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_block_make_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_make_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_block_make(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_block_make_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x15,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_make_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_make(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_WRITE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_block_id_by_height_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_id_by_height_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_id_by_height_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_id_by_height_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_block_id_by_height_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_id_by_height_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_id_by_height_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_id_by_height_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_block_id_by_height_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_id_by_height_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_block_id_by_height_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_block_id_by_height_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x16,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_id_by_height_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_id_by_height_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_block_id_by_height_get_decoded_full_payload)
{
    const uint8_t EXPECTED_BLOCK_ID[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    uint8_t resp[28] = {
        /* method code. */
        0x00,
        0x00,
        0x00,
        0x16,

        /* offset == 1023 */
        0x00,
        0x00,
        0x03,
        0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00,
        0x00,
        0x00,
        0x00,

        /* block_id */
        0x37,
        0xfb,
        0x38,
        0xd3,
        0xfe,
        0x6b,
        0x4e,
        0x9c,
        0xba,
        0x15,
        0x91,
        0xbe,
        0xf7,
        0xf3,
        0x87,
        0xef,
    };
    dataservice_response_block_id_by_height_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_id_by_height_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_BLOCK_ID, dresp.block_id, 16));
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_latest_block_id_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_latest_block_id_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_latest_block_id_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_latest_block_id_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_latest_block_id_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_latest_block_id_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_latest_block_id_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_latest_block_id_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_latest_block_id_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_latest_block_id_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_latest_block_id_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_latest_block_id_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x09,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_latest_block_id_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_latest_block_id_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_latest_block_id_get_decoded_full_payload)
{
    const uint8_t EXPECTED_BLOCK_ID[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    uint8_t resp[28] = {
        /* method code. */
        0x00,
        0x00,
        0x00,
        0x09,

        /* offset == 1023 */
        0x00,
        0x00,
        0x03,
        0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00,
        0x00,
        0x00,
        0x00,

        /* block_id */
        0x37,
        0xfb,
        0x38,
        0xd3,
        0xfe,
        0x6b,
        0x4e,
        0x9c,
        0xba,
        0x15,
        0x91,
        0xbe,
        0xf7,
        0xf3,
        0x87,
        0xef,
    };
    dataservice_response_latest_block_id_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_latest_block_id_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_BLOCK_ID, dresp.block_id, 16));
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_artifact_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_artifact_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_artifact_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_artifact_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_artifact_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_artifact_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_artifact_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_artifact_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_artifact_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_artifact_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_artifact_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_artifact_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x14,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_artifact_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_artifact_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_ARTIFACT_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_artifact_get_decoded_full_payload)
{
    const uint8_t EXPECTED_RECORD_KEY[] = {
        0x66, 0x60, 0x2f, 0x1e, 0x39, 0x71, 0x44, 0xd3,
        0xb9, 0x26, 0xbe, 0x73, 0xd8, 0x53, 0x19, 0x9f
    };

    const uint8_t EXPECTED_RECORD_TXN_FIRST[] = {
        0x85, 0x02, 0x75, 0x5a, 0x98, 0xbb, 0x4a, 0xc7,
        0xa7, 0xd5, 0x05, 0xa6, 0x5a, 0x60, 0x25, 0xcd
    };

    const uint8_t EXPECTED_RECORD_TXN_LATEST[] = {
        0xef, 0x97, 0x82, 0xb4, 0xfe, 0xac, 0x4d, 0x39,
        0x8c, 0x19, 0xb4, 0xd7, 0xc2, 0xfe, 0xdf, 0x2b
    };

    const uint64_t EXPECTED_RECORD_NET_HEIGHT_FIRST = htonll(12);

    const uint64_t EXPECTED_RECORD_NET_HEIGHT_LATEST = htonll(71);

    const uint32_t EXPECTED_RECORD_NET_STATE_LATEST = htonl(9);

    uint8_t resp[80] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x14,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* record.key */
        0x66, 0x60, 0x2f, 0x1e, 0x39, 0x71, 0x44, 0xd3,
        0xb9, 0x26, 0xbe, 0x73, 0xd8, 0x53, 0x19, 0x9f,

        /* record.txn_first */
        0x85, 0x02, 0x75, 0x5a, 0x98, 0xbb, 0x4a, 0xc7,
        0xa7, 0xd5, 0x05, 0xa6, 0x5a, 0x60, 0x25, 0xcd,

        /* record.txn_latest */
        0xef, 0x97, 0x82, 0xb4, 0xfe, 0xac, 0x4d, 0x39,
        0x8c, 0x19, 0xb4, 0xd7, 0xc2, 0xfe, 0xdf, 0x2b,

        /* record.net_height_first */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,

        /* record.net_height_latest */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47,

        /* record.net_state_latest */
        0x00, 0x00, 0x00, 0x09
    };
    dataservice_response_artifact_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_artifact_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_ARTIFACT_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the record key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_RECORD_KEY, dresp.record.key, 16));
    /* the record txn first should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_RECORD_TXN_FIRST, dresp.record.txn_first, 16));
    /* the record txn latest should match. */
    ASSERT_EQ(
        0, memcmp(EXPECTED_RECORD_TXN_LATEST, dresp.record.txn_latest, 16));
    /* the record net height first should match. */
    ASSERT_EQ(EXPECTED_RECORD_NET_HEIGHT_FIRST, dresp.record.net_height_first);
    /* the record net height latest should match. */
    ASSERT_EQ(
        EXPECTED_RECORD_NET_HEIGHT_LATEST, dresp.record.net_height_latest);
    /* the record net state latest should match. */
    ASSERT_EQ(
        EXPECTED_RECORD_NET_STATE_LATEST, dresp.record.net_state_latest);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_block_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_block_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_block_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_block_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_block_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0d,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_block_get_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_FIRST_TRANSACTION_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    const uint64_t EXPECTED_NODE_NET_BLOCK_HEIGHT = htonll(97);

    uint8_t resp[88] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0d,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.first_txn_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* node.net_block_height */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_block_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0,
        memcmp(
            EXPECTED_NODE_FIRST_TRANSACTION_ID,
            dresp.node.first_transaction_id, 16));
    /* the node block height is correct. */
    ASSERT_EQ(EXPECTED_NODE_NET_BLOCK_HEIGHT, dresp.node.net_block_height);
    /* the node net size should match */
    ASSERT_EQ(4U, dresp.data_size);
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 84, dresp.data);
}
