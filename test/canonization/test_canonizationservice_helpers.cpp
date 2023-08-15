/**
 * \file test_canonizationservice_isolation_helpers.cpp
 *
 * Helpers for the canonization service isolation test.
 *
 * \copyright 2019-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>
#include <ostream>
#include <signal.h>
#include <sys/wait.h>
#include <vpr/allocator/malloc_allocator.h>

#include "test_canonizationservice_isolation.h"

using namespace std;

const uint32_t
    canonizationservice_isolation_test::EXPECTED_CHILD_INDEX = 19U;

const uint8_t canonizationservice_isolation_test::agent_id[16] = {
    0x3d, 0x96, 0x3f, 0x54, 0x83, 0xe2, 0x4b, 0x0d,
    0x86, 0xa1, 0x81, 0xb6, 0xaa, 0xaa, 0x5c, 0x1b };

const uint8_t canonizationservice_isolation_test::agent_enc_pubkey[32] = {
    0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
    0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
    0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
    0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f };

const uint8_t canonizationservice_isolation_test::agent_enc_privkey[32] = {
    0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
    0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
    0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
    0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb };

const uint8_t canonizationservice_isolation_test::agent_sign_pubkey[32] = {
    0x3b, 0xcb, 0xc2, 0xdc, 0x1e, 0xed, 0x49, 0xa4,
    0x99, 0x0a, 0x12, 0xe8, 0x73, 0x79, 0xa0, 0x64,
    0xeb, 0x20, 0xc7, 0xe8, 0x16, 0x7d, 0x9e, 0x82,
    0xa3, 0xf0, 0x1e, 0x34, 0x36, 0x23, 0x9e, 0x2a };

const uint8_t canonizationservice_isolation_test::agent_sign_privkey[64] = {
    0x01, 0xa8, 0xc4, 0xe2, 0xcf, 0x41, 0xd2, 0x4f,
    0x80, 0x43, 0x14, 0xc8, 0xc2, 0x4a, 0x46, 0xc4,
    0xb1, 0x31, 0x74, 0xc3, 0x0d, 0xcd, 0xe0, 0x80,
    0xd8, 0x2d, 0x87, 0x75, 0xc1, 0x74, 0x47, 0xf3,
    0x3b, 0xcb, 0xc2, 0xdc, 0x1e, 0xed, 0x49, 0xa4,
    0x99, 0x0a, 0x12, 0xe8, 0x73, 0x79, 0xa0, 0x64,
    0xeb, 0x20, 0xc7, 0xe8, 0x16, 0x7d, 0x9e, 0x82,
    0xa3, 0xf0, 0x1e, 0x34, 0x36, 0x23, 0x9e, 0x2a };

void canonizationservice_isolation_test::setUp()
{
    vccrypt_suite_register_velo_v1();

    /* initialize allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_suite_options_init(
            &suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        suite_instance_initialized = true;
    }
    else
    {
        suite_instance_initialized = false;
    }

    /* set the path for running agentd. */
    const char* agentd_path = getenv("AGENTD_PATH");
    if (NULL != agentd_path)
    {
        strcpy(wd, agentd_path);
        oldpath = getenv("PATH");
        if (NULL != oldpath)
        {
            path =
                strcatv(wd, ":", oldpath, NULL);
        }
        else
        {
            path = strcatv(wd, NULL);
        }
    }

    setenv("PATH", path, 1);

    /* log to standard error. */
    logsock = dup(STDERR_FILENO);
    rlogsock = dup(STDERR_FILENO);

    /* create the socket pair for the datasock. */
    int datasock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &datasock, &datasock_srv);

    /* create the control socket pair for the canonization service. */
    int controlsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &controlsock, &controlsock_srv);

    /* create the notificationservice socket pair. */
    int notifysock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &notifysock, &notifysock_srv);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* spawn the random service process. */
    random_proc_status =
        randomservice_proc(
            &bconf, &conf, &rlogsock, &rprotosock, &randompid, false);

    /* spawn the canonization service process. */
    canonization_proc_status =
        start_canonization_proc(
            &bconf, &conf, &logsock, &datasock_srv, &rprotosock,
            &controlsock_srv, notifysock_srv, &canonizationpid, false);

    /* create the mock dataservice. */
    dataservice = make_unique<mock_dataservice::mock_dataservice>(datasock);

    /* create the mock notificationservice. */
    notificationservice =
        make_unique<mock_notificationservice::mock_notificationservice>(
            notifysock);
}

void canonizationservice_isolation_test::tearDown()
{
    /* terminate the random service. */
    if (0 == random_proc_status)
    {
        int status = 0;
        kill(randompid, SIGTERM);
        waitpid(randompid, &status, 0);
    }

    /* terminate the canonization service process. */
    if (0 == canonization_proc_status)
    {
        int status = 0;
        close(controlsock);
        close(notifysock);
        kill(canonizationpid, SIGTERM);
        waitpid(canonizationpid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dataservice->stop();
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    if (logsock >= 0)
        close(logsock);
    if (rlogsock >= 0)
        close(rlogsock);
    close(datasock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    dispose((disposable_t*)&alloc_opts);
}

int canonizationservice_isolation_test::dataservice_mock_register_helper()
{
    /* mock the child context create call. */
    dataservice->register_callback_child_context_create(
        [&](const dataservice_request_child_context_create_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_child_context_create(
                    &payload, &payload_size, EXPECTED_CHILD_INDEX);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the child context close call. */
    dataservice->register_callback_child_context_close(
        [&](const dataservice_request_child_context_close_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    return 0;
}

int canonizationservice_isolation_test::
    dataservice_mock_valid_connection_setup()
{
    /* a child context should have been created. */
    BITCAP(testbits, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(testbits);
    /* TODO - add valid bits here. */
    if (!dataservice->request_matches_child_context_create(testbits))
        return 1;

    return 0;
}

int canonizationservice_isolation_test::
    dataservice_mock_valid_connection_teardown()
{
    /* the child index should have been closed. */
    if (!dataservice->request_matches_child_context_close(EXPECTED_CHILD_INDEX))
        return 1;

    return 0;
}

int canonizationservice_isolation_test::
    canonizationservice_configure_and_start(int max_milliseconds, int max_txns)
{
    int retval;
    uint32_t status, offset;
    agent_config_t conf;
    vccrypt_buffer_t entity_encryption_pubkey;
    vccrypt_buffer_t entity_encryption_privkey;
    vccrypt_buffer_t entity_signing_pubkey;
    vccrypt_buffer_t entity_signing_privkey;

    /* set config values for canonization service. */
    conf.block_max_milliseconds_set = true;
    conf.block_max_milliseconds = max_milliseconds;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = max_txns;

    /* initialize the entity encryption pubkey buffer. */
    retval =
        vccrypt_buffer_init(&entity_encryption_pubkey, &alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* copy the key to the buffer. */
    memcpy(entity_encryption_pubkey.data, agent_enc_pubkey, 32);

    /* initialize the entity encryption privkey buffer. */
    retval =
        vccrypt_buffer_init(&entity_encryption_privkey, &alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_encryption_pubkey;
    }

    /* copy the key to the buffer. */
    memcpy(entity_encryption_privkey.data, agent_enc_privkey, 32);

    /* initialize the entity signing pubkey buffer. */
    retval =
        vccrypt_buffer_init(&entity_signing_pubkey, &alloc_opts, 32);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_encryption_privkey;
    }

    /* copy the key to the buffer. */
    memcpy(entity_signing_pubkey.data, agent_sign_pubkey, 32);

    /* initialize the entity signing privkey buffer. */
    retval =
        vccrypt_buffer_init(&entity_signing_privkey, &alloc_opts, 64);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_pubkey;
    }

    /* copy the key to the buffer. */
    memcpy(entity_signing_privkey.data, agent_sign_privkey, 64);

    /* send the configure service request. */
    retval =
        canonization_api_sendreq_configure(controlsock, &conf);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* receive the configure service response. */
    retval =
        canonization_api_recvresp_configure(controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* verify that the configure request was successful. */
    retval = (int)status;
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* send the private key set request. */
    retval =
        canonization_api_sendreq_private_key_set(
            controlsock, &alloc_opts, agent_id,
            &entity_encryption_pubkey, &entity_encryption_privkey,
            &entity_signing_pubkey, &entity_signing_privkey);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    retval =
        canonization_api_recvresp_private_key_set(
            controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* verify that the private key set request was successful. */
    retval = (int)status;
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* send the start request. */
    retval = canonization_api_sendreq_start(controlsock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* receive the start response. */
    retval = canonization_api_recvresp_start(controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* verify that the start request was successful. */
    retval = (int)status;
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_entity_signing_privkey;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_entity_signing_privkey:
    dispose((disposable_t*)&entity_signing_privkey);

cleanup_entity_signing_pubkey:
    dispose((disposable_t*)&entity_signing_pubkey);

cleanup_entity_encryption_privkey:
    dispose((disposable_t*)&entity_encryption_privkey);

cleanup_entity_encryption_pubkey:
    dispose((disposable_t*)&entity_encryption_pubkey);

done:
    return retval;
}
