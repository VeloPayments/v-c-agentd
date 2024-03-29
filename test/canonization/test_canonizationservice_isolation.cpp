/**
 * \file test_canonizationservice_isolation.cpp
 *
 * Isolation tests for the canonization service.
 *
 * \copyright 2019-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <minunit/minunit.h>
#include <string>
#include <unistd.h>
#include <vccert/certificate_types.h>
#include <vccrypt/compare.h>
#include <vpr/disposable.h>

#include "test_canonizationservice_isolation.h"

TEST_SUITE(canonizationservice_isolation_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    canonizationservice_isolation_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that we can spawn the canonization service.
 */
BEGIN_TEST_F(simple_spawn)
    TEST_ASSERT(0 == fixture.canonization_proc_status);
END_TEST_F()

/**
 * Test that calling start before calling configure results in an error.
 */
BEGIN_TEST_F(start_before_configure_fail)
    uint32_t offset = 0, status = AGENTD_STATUS_SUCCESS;

    /* we should be able to successfully call start. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_start(fixture.controlsock));

    /* we should be able to receive a response from the start call. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_start(
                    fixture.controlsock, &offset, &status));

    /* the status should NOT be success. */
    TEST_ASSERT(
        AGENTD_ERROR_CANONIZATIONSERVICE_START_BEFORE_CONFIGURE
            == (int)status);
END_TEST_F()

/**
 * Test that we can configure the canonization service.
 */
BEGIN_TEST_F(configure)
    agent_config_t conf;
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* set config values for canonization service. */
    conf.block_max_milliseconds_set = true;
    conf.block_max_milliseconds = 2;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = 1000;

    /* we should be able to successfully call config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_configure(fixture.controlsock, &conf));

    /* we should be able to receive a response from config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_configure(
                    fixture.controlsock, &offset, &status));

    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);
END_TEST_F()

/**
 * Test that we can set the private key for the canonization service.
 */
BEGIN_TEST_F(set_private_key)
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    const uint8_t entity_id[16] = {
        0x33, 0xa5, 0x17, 0x73, 0xbd, 0x72, 0x41, 0xc9,
        0xba, 0xba, 0xe1, 0xb5, 0x98, 0x94, 0x9e, 0x05 };
    vccrypt_buffer_t entity_encryption_pubkey;
    vccrypt_buffer_t entity_encryption_privkey;
    vccrypt_buffer_t entity_signing_pubkey;
    vccrypt_buffer_t entity_signing_privkey;

    /* create dummy entity encryption pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_encryption_pubkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_pubkey.data, 0xFF, 32);

    /* create dummy entity encryption privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_encryption_privkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_privkey.data, 0xFF, 32);

    /* create dummy entity signing pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_signing_pubkey, &fixture.alloc_opts, 32));
    memset(entity_signing_pubkey.data, 0xFF, 32);

    /* create dummy entity signing privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_signing_privkey, &fixture.alloc_opts, 64));
    memset(entity_signing_privkey.data, 0xFF, 64);

    /* we should be able to successfully call private_key_set. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_private_key_set(
                    fixture.controlsock, &fixture.alloc_opts, entity_id,
                    &entity_encryption_pubkey, &entity_encryption_privkey,
                    &entity_signing_pubkey, &entity_signing_privkey));

    /* we should be able to receive a response from private_key_set. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_private_key_set(
                    fixture.controlsock, &offset, &status));

    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);

    /* clean up. */
    dispose((disposable_t*)&entity_encryption_pubkey);
    dispose((disposable_t*)&entity_encryption_privkey);
    dispose((disposable_t*)&entity_signing_pubkey);
    dispose((disposable_t*)&entity_signing_privkey);
END_TEST_F()

/**
 * Test that we can't start the canonization service until setting the private
 * key.
 */
BEGIN_TEST_F(start_without_private_key_set)
    agent_config_t conf;
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* set config values for canonization service. */
    conf.block_max_milliseconds_set = true;
    conf.block_max_milliseconds = 2;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = 1000;

    /* we should be able to successfully call config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_configure(fixture.controlsock, &conf));

    /* we should be able to receive a response from config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_configure(
                    fixture.controlsock, &offset, &status));

    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);

    /* we should be able to successfully call start. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_start(fixture.controlsock));

    /* we should be able to receive a response from the start call. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_start(
                    fixture.controlsock, &offset, &status));

    /* starting should have failed, because the private key is not set. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS != (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);
END_TEST_F()

/**
 * Test that we can start the canonization service after configuring it and
 * setting the private key.
 */
BEGIN_TEST_F(start)
    agent_config_t conf;
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    const uint8_t entity_id[16] = {
        0x33, 0xa5, 0x17, 0x73, 0xbd, 0x72, 0x41, 0xc9,
        0xba, 0xba, 0xe1, 0xb5, 0x98, 0x94, 0x9e, 0x05 };
    vccrypt_buffer_t entity_encryption_pubkey;
    vccrypt_buffer_t entity_encryption_privkey;
    vccrypt_buffer_t entity_signing_pubkey;
    vccrypt_buffer_t entity_signing_privkey;

    /* create dummy entity encryption pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_encryption_pubkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_pubkey.data, 0xFF, 32);

    /* create dummy entity encryption privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_encryption_privkey, &fixture.alloc_opts, 32));
    memset(entity_encryption_privkey.data, 0xFF, 32);

    /* create dummy entity signing pubkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_signing_pubkey, &fixture.alloc_opts, 32));
    memset(entity_signing_pubkey.data, 0xFF, 32);

    /* create dummy entity signing privkey. */
    TEST_ASSERT(
        VCCRYPT_STATUS_SUCCESS
            == vccrypt_buffer_init(
                    &entity_signing_privkey, &fixture.alloc_opts, 64));
    memset(entity_signing_privkey.data, 0xFF, 64);

    /* we should be able to successfully call private_key_set. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_private_key_set(
                    fixture.controlsock, &fixture.alloc_opts, entity_id,
                    &entity_encryption_pubkey, &entity_encryption_privkey,
                    &entity_signing_pubkey, &entity_signing_privkey));

    /* we should be able to receive a response from private_key_set. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_private_key_set(
                    fixture.controlsock, &offset, &status));

    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);

    /* set config values for canonization service. */
    conf.block_max_milliseconds_set = true;
    conf.block_max_milliseconds = 2;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = 1000;

    /* we should be able to successfully call config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_configure(fixture.controlsock, &conf));

    /* we should be able to receive a response from config. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_configure(
                    fixture.controlsock, &offset, &status));

    /* the status should be success. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);

    /* we should be able to successfully call start. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_sendreq_start(fixture.controlsock));

    /* we should be able to receive a response from the start call. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == canonization_api_recvresp_start(
                    fixture.controlsock, &offset, &status));

    /* starting should have failed, because the private key is not set. */
    TEST_EXPECT(AGENTD_STATUS_SUCCESS == (int)status);
    /* the offset should be zero. */
    TEST_EXPECT(0U == offset);

    /* clean up. */
    dispose((disposable_t*)&entity_encryption_pubkey);
    dispose((disposable_t*)&entity_encryption_privkey);
    dispose((disposable_t*)&entity_signing_pubkey);
    dispose((disposable_t*)&entity_signing_privkey);
END_TEST_F()

/**
 * Test that the canonization service tries again when there are no
 * transactions.
 */
BEGIN_TEST_F(no_txn_retry)
    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream&) {
            return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 10));

    usleep(30000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()

/**
 * Test that the canonization service tries again when there are no
 * transactions and a block exists.
 */
BEGIN_TEST_F(no_txn_retry_with_block)
    const uint8_t dummy_block_id[16] = {
        0x53, 0x25, 0xb2, 0xa7, 0xc8, 0xa9, 0x45, 0x60,
        0xb9, 0xea, 0xca, 0x23, 0xc3, 0xf7, 0xb0, 0x72
    };
    const uint8_t dummy_block_end[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    const uint8_t dummy_block_cert[68] = {
        0x00, 0x51, 0x00, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream&) {
            return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)dummy_block_id, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the block read call. */
    fixture.dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& out) {
            uint64_t height = htonll(16);

            out.write((const char*)dummy_block_id, 16);
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);
            out.write((const char*)dummy_block_end, 16);
            out.write((const char*)dummy_block_end, 16);
            out.write((const char*)&height, sizeof(height));
            out.write((const char*)&height, sizeof(height));
            out.write((const char*)dummy_block_id, 16);
            out.write((const char*)&dummy_block_cert, sizeof(dummy_block_cert));

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 10));

    usleep(30000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get block call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, dummy_block_id));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get block call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_read(
            fixture.EXPECTED_CHILD_INDEX, dummy_block_id));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()

/**
 * Test that the canonization service tries again when the first transaction
 * hasn't been attested.
 */
BEGIN_TEST_F(no_attested_retry)
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0xb8, 0x4e, 0x5b, 0xe9, 0x0c, 0x4b, 0x49, 0x88,
        0x92, 0x50, 0xe0, 0xb0, 0x3f, 0xb2, 0xfe, 0x36
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xf2, 0x66, 0xf1, 0x55, 0x5f, 0xc1, 0x4b, 0x06,
        0xac, 0xd2, 0x08, 0x66, 0x83, 0xe3, 0x41, 0xc1
    };
    const uint8_t EXPECTED_TRANSACTION_BEGIN[16] = {
        0x00, 0X00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t EXPECTED_TRANSACTION_END[16] = {
        0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const uint8_t EXPECTED_CERT[] = { 0x00, 0x01, 0x02, 0x03 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream& out) {
            void* payload;
            size_t payload_size;

            int retval =
                dataservice_encode_response_transaction_get_first(
                    &payload, &payload_size, EXPECTED_TRANSACTION_ID,
                    EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_END,
                    EXPECTED_ARTIFACT_ID,
                    htonl(DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED),
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
            {
                return retval;
            }

            out.write((const char*)payload, payload_size);
            free(payload);

            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 10));

    usleep(30000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()

/**
 * Test that the canonization service builds a block with a single attested
 * record.
 */
BEGIN_TEST_F(one_attested_block)
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0xb8, 0x4e, 0x5b, 0xe9, 0x0c, 0x4b, 0x49, 0x88,
        0x92, 0x50, 0xe0, 0xb0, 0x3f, 0xb2, 0xfe, 0x36
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xf2, 0x66, 0xf1, 0x55, 0x5f, 0xc1, 0x4b, 0x06,
        0xac, 0xd2, 0x08, 0x66, 0x83, 0xe3, 0x41, 0xc1
    };
    const uint8_t EXPECTED_TRANSACTION_BEGIN[16] = {
        0x00, 0X00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t EXPECTED_TRANSACTION_END[16] = {
        0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const uint8_t EXPECTED_CERT[] = { 0x00, 0x01, 0x02, 0x03 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the first transaction query api call. */
    bool first_run = true;
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream& out) {
            /* only return a result the first time. */
            if (first_run)
            {
                void* payload;
                size_t payload_size;

                first_run = false;

                int retval =
                    dataservice_encode_response_transaction_get_first(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID,
                        EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_END,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            else
            {
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
            }
        });

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get(
        [&](const dataservice_request_transaction_get_t&,
            std::ostream&) {
            /* only the first record is found. */
            return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 10));

    usleep(30000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a block make call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_make(
            fixture.EXPECTED_CHILD_INDEX, NULL, 0, NULL));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()

/**
 * Test that the canonization service builds a block with a multiple attested
 * records.
 */
BEGIN_TEST_F(multiple_attested_txns_one_block)
    const uint8_t EXPECTED_TRANSACTION_ID_01[16] = {
        0xb8, 0x4e, 0x5b, 0xe9, 0x0c, 0x4b, 0x49, 0x88,
        0x92, 0x50, 0xe0, 0xb0, 0x3f, 0xb2, 0xfe, 0x36
    };
    const uint8_t EXPECTED_TRANSACTION_ID_02[16] = {
        0xad, 0x32, 0xff, 0x01, 0xb9, 0x63, 0x41, 0x28,
        0x83, 0x38, 0x12, 0xa4, 0x23, 0x54, 0x5f, 0xcd
    };
    const uint8_t EXPECTED_TRANSACTION_ID_03[16] = {
        0x16, 0xc0, 0x8c, 0xde, 0xfd, 0x24, 0x43, 0xb9,
        0x92, 0x48, 0x95, 0x23, 0x33, 0xec, 0xa1, 0x43
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xf2, 0x66, 0xf1, 0x55, 0x5f, 0xc1, 0x4b, 0x06,
        0xac, 0xd2, 0x08, 0x66, 0x83, 0xe3, 0x41, 0xc1
    };
    const uint8_t EXPECTED_TRANSACTION_BEGIN[16] = {
        0x00, 0X00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t EXPECTED_TRANSACTION_END[16] = {
        0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const uint8_t EXPECTED_CERT[] = { 0x00, 0x01, 0x02, 0x03 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the first transaction query api call. */
    bool first_run = true;
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream& out) {
            /* only return a result the first time. */
            if (first_run)
            {
                void* payload;
                size_t payload_size;

                first_run = false;

                int retval =
                    dataservice_encode_response_transaction_get_first(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_01,
                        EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_ID_02,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            else
            {
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
            }
        });

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get(
        [&](const dataservice_request_transaction_get_t& txn,
            std::ostream& out) {
            void* payload;
            size_t payload_size;
            int retval;

            if (!crypto_memcmp(txn.txn_id, EXPECTED_TRANSACTION_ID_02, 16))
            {
                retval =
                    dataservice_encode_response_transaction_get(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_02,
                        EXPECTED_TRANSACTION_ID_01, EXPECTED_TRANSACTION_ID_03,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            else if (!crypto_memcmp(txn.txn_id, EXPECTED_TRANSACTION_ID_03, 16))
            {
                retval =
                    dataservice_encode_response_transaction_get(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_03,
                        EXPECTED_TRANSACTION_ID_02, EXPECTED_TRANSACTION_END,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            else
            {
                /* no more records found. */
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
            }
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 10));

    usleep(30000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TRANSACTION_ID_02));

    /* a get call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get(
            fixture.EXPECTED_CHILD_INDEX, EXPECTED_TRANSACTION_ID_03));

    /* a block make call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_make(
            fixture.EXPECTED_CHILD_INDEX, NULL, 0, NULL));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()

/**
 * Test that the canonization service builds multiple blocks with attested
 * transactions in them.
 */
BEGIN_TEST_F(multiple_attested_multiple_blocks)
    const uint8_t EXPECTED_TRANSACTION_ID_01[16] = {
        0xb8, 0x4e, 0x5b, 0xe9, 0x0c, 0x4b, 0x49, 0x88,
        0x92, 0x50, 0xe0, 0xb0, 0x3f, 0xb2, 0xfe, 0x36
    };
    const uint8_t EXPECTED_TRANSACTION_ID_02[16] = {
        0xad, 0x32, 0xff, 0x01, 0xb9, 0x63, 0x41, 0x28,
        0x83, 0x38, 0x12, 0xa4, 0x23, 0x54, 0x5f, 0xcd
    };
    const uint8_t EXPECTED_TRANSACTION_ID_03[16] = {
        0x16, 0xc0, 0x8c, 0xde, 0xfd, 0x24, 0x43, 0xb9,
        0x92, 0x48, 0x95, 0x23, 0x33, 0xec, 0xa1, 0x43
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xf2, 0x66, 0xf1, 0x55, 0x5f, 0xc1, 0x4b, 0x06,
        0xac, 0xd2, 0x08, 0x66, 0x83, 0xe3, 0x41, 0xc1
    };
    const uint8_t EXPECTED_TRANSACTION_BEGIN[16] = {
        0x00, 0X00, 0X00, 0X00, 0X00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t EXPECTED_TRANSACTION_END[16] = {
        0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const uint8_t EXPECTED_CERT[] = { 0x00, 0x01, 0x02, 0x03 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);

    /* register dataservice helper mocks. */
    TEST_ASSERT(0 == fixture.dataservice_mock_register_helper());

    /* mock the first transaction query api call. */
    int run_count = 0;
    fixture.dataservice->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream& out) {
            void* payload;
            size_t payload_size;
            int retval;

            /* on the first run, return the first attested txn. */
            if (run_count < 1)
            {
                ++run_count;

                retval =
                    dataservice_encode_response_transaction_get_first(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_01,
                        EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_ID_02,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            /* on the second run, return the second attested txn. */
            else if (run_count < 2)
            {
                ++run_count;

                retval =
                    dataservice_encode_response_transaction_get_first(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_02,
                        EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_ID_03,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            /* on the second run, return the third attested txn. */
            else if (run_count < 3)
            {
                ++run_count;

                retval =
                    dataservice_encode_response_transaction_get_first(
                        &payload, &payload_size, EXPECTED_TRANSACTION_ID_03,
                        EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_END,
                        EXPECTED_ARTIFACT_ID,
                        htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                        EXPECTED_CERT, EXPECTED_CERT_SIZE);
                if (AGENTD_STATUS_SUCCESS != retval)
                {
                    return retval;
                }

                out.write((const char*)payload, payload_size);
                free(payload);

                return AGENTD_STATUS_SUCCESS;
            }
            else
            {
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
            }
        });

    /* mock the transaction query api call. */
    fixture.dataservice->register_callback_transaction_get(
        [&](const dataservice_request_transaction_get_t& txn,
            std::ostream& out) {
            void* payload;
            size_t payload_size;
            int retval;

            /* return a dummy txn. */
            retval =
                dataservice_encode_response_transaction_get_first(
                    &payload, &payload_size, txn.txn_id,
                    EXPECTED_TRANSACTION_BEGIN, EXPECTED_TRANSACTION_ID_01,
                    EXPECTED_ARTIFACT_ID,
                    htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED),
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
            {
                return retval;
            }

            out.write((const char*)payload, payload_size);
            free(payload);

            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the latest block id query api call. */
    fixture.dataservice->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& out) {
            out.write((const char*)vccert_certificate_type_uuid_root_block, 16);

            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mocks. */
    fixture.dataservice->start();
    fixture.notificationservice->start();

    /* we should be able to configure and start the canonization service. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == fixture.canonizationservice_configure_and_start(1, 1));

    usleep(40000);

    /* stop the mock. */
    fixture.dataservice->stop();

    /* set our expected caps. */
    BITCAP(EXPECTED_CAPS, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(EXPECTED_CAPS);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        EXPECTED_CAPS, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a block make call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_make(
            fixture.EXPECTED_CHILD_INDEX, NULL, 0, NULL));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a block make call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_make(
            fixture.EXPECTED_CHILD_INDEX, NULL, 0, NULL));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a block make call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_make(
            fixture.EXPECTED_CHILD_INDEX, NULL, 0, NULL));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child create should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_create(
            EXPECTED_CAPS));

    /* a get latest block id call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_block_id_latest_read(
            fixture.EXPECTED_CHILD_INDEX));

    /* a second get first call should have been made. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_transaction_get_first(
            fixture.EXPECTED_CHILD_INDEX));

    /* a child close should have occurred. */
    TEST_EXPECT(
        fixture.dataservice->request_matches_child_context_close(
            fixture.EXPECTED_CHILD_INDEX));
END_TEST_F()
