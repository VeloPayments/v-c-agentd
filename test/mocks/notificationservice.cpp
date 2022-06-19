/**
 * \file mocks/notificationservice.cpp
 *
 * Mock notificationservice methods.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/ipc.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#include "notificationservice.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Create a mock notificationservice instance that will listen on the
 * given socket when started.
 *
 * \param _notifysock       The socket used to listen for
 *                          notificationservice requests.
 */
mock_notificationservice::mock_notificationservice::mock_notificationservice(
    int _notifysock)
    : notifysock(_notifysock)
    , running(false)
    , testsock(-1)
    , mocksock(-1)
    , reduce_caps_status_override(false)
{
    status retval = rcpr_malloc_allocator_create(&rcpr_alloc);
    (void)retval;
}

/**
 * \brief Make sure to stop the mock notificationservice if running on
 * destruction.
 */
mock_notificationservice::mock_notificationservice::~mock_notificationservice()
{
    if (-1 != notifysock)
        close(notifysock);

    if (-1 != mocksock)
        close(mocksock);

    if (-1 != testsock)
        close(testsock);

    stop();

    status retval =
        resource_release(rcpr_allocator_resource_handle(rcpr_alloc));
    (void)retval;
}

/**
 * \brief Start the mock notificationservice with the current mock settings.
 */
void mock_notificationservice::mock_notificationservice::start()
{
    /* only start the mock dataservice once. */
    if (running)
    {
        return;
    }

    /* set up the socketpair for running the test service. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &testsock, &mocksock))
    {
        return;
    }

    /* fork the process. */
    mock_pid = fork();
    if (mock_pid < 0)
    {
        goto cleanup_socketpair;
    }

    /* child */
    if (0 == mock_pid)
    {
        close(testsock);
        testsock = -1;
        mock_process();
        exit(0);
    }
    /* parent */
    else
    {
        close(notifysock);
        notifysock = -1;
        close(mocksock);
        mocksock = -1;
        running = true;
        return;
    }

cleanup_socketpair:
    close(testsock);
    testsock = -1;
    close(mocksock);
    mocksock = -1;
}

/**
 * \brief Stop the mock notificationservice if running.
 */
void mock_notificationservice::mock_notificationservice::stop()
{
    /* only stop the mock dataservice if running. */
    if (!running)
        return;

    /* sleep for a sec to propagate the close. */
    usleep(10000);

    /* kill the child process. */
    kill(mock_pid, SIGTERM);

    /* wait on the pid to terminate.*/
    int wstatus = 0;
    waitpid(mock_pid, &wstatus, 0);

    /* We are no longer running. */
    running = false;
}

/**
 * \brief Run the mock notificationservice process.
 *
 * Read request packets from the process and write canned response
 * packets, possibly using the mock override callbacks.
 */
void mock_notificationservice::mock_notificationservice::mock_process()
{
    /* read every transaction on the socket. */
    while (mock_read_and_dispatch())
        ;

    /* close the socket. */
    close(notifysock);
    notifysock = -1;
}

/**
 * \brief Read and dispatch one request.
 *
 * \returns true if a request was read, and false if anything goes
 *          wrong (e.g. a socket was closed).
 */
bool mock_notificationservice::mock_notificationservice::mock_read_and_dispatch()
{
    bool retval = false;
    uint8_t* val = nullptr;
    uint32_t size = 0U;
    uint32_t method;
    uint64_t offset;
    const uint8_t* payload;
    size_t payload_size;

    /* read a request to the notificationservice mock. */
    if (AGENTD_STATUS_SUCCESS !=
            ipc_read_data_block(notifysock, (void**)&val, &size))
    {
        retval = false;
        goto done;
    }

    /* immediately write this request to the mocksock to log it. */
    if (AGENTD_STATUS_SUCCESS != ipc_write_data_block(mocksock, val, size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* immediately write this request to the mocksock to log it. */
    if (AGENTD_STATUS_SUCCESS != ipc_write_data_block(mocksock, val, size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* decode this message. */
    if (AGENTD_STATUS_SUCCESS !=
            notificationservice_api_decode_request(
                val, size, &method, &offset, &payload, &payload_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* decode the method id. */
    switch (method)
    {
        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS:
            retval =
                mock_decode_and_dispatch_reduce_caps(
                    offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE:
            retval =
                mock_decode_and_dispatch_block_update(
                    offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION:
            retval =
                mock_decode_and_dispatch_block_assertion(
                    offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL:
            retval =
                mock_decode_and_dispatch_block_assertion_cancel(
                    offset, payload, payload_size);
            break;

        default:
            /* for now, just write a success status. */
            mock_write_status(
                method, offset,
                AGENTD_STATUS_SUCCESS, nullptr, 0);
            retval = true;
            break;
    }

    /* done */
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Write the status back to the caller.
 *
 * \param method    The method requested.
 * \param offset    The child offset.
 * \param status    The status code.
 * \param payload   The response payload, or ptr if none.
 * \param size      The response payload size.
 */
void mock_notificationservice::mock_notificationservice::mock_write_status(
    uint32_t method, uint64_t offset, uint32_t status_code,
    const void* payload, size_t size)
{
    status retval;
    uint8_t* buf;
    size_t buf_size;

    /* encode a response packet. */
    retval =
        notificationservice_api_encode_response(
            &buf, &buf_size, rcpr_alloc, method, status_code, offset,
            (const uint8_t*)payload, size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    ipc_write_data_block(notifysock, buf, buf_size);

    retval = rcpr_allocator_reclaim(rcpr_alloc, buf);
    (void)retval;

done:;
}

/**
 * \brief Decode and dispatch a block update request.
 *
 * \returns true if the request was dispatched successfully and false
 *          otherwise.
 */
bool mock_notificationservice::mock_notificationservice::
    mock_decode_and_dispatch_block_update(
    uint64_t offset, const uint8_t* payload, size_t payload_size)
{
    bool retval = false;
    uint32_t status = STATUS_SUCCESS;
    rcpr_uuid block_id;

    /* parse the request payload. */
    if (payload_size != sizeof(block_id))
    {
        retval = false;
        status = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto done;
    }

    /* copy the block id. */
    memcpy(&block_id, payload, payload_size);
    (void)block_id;

    /* if the mock callback is set, call it. */
    if (!!block_update_callback)
    {
        status = block_update_callback(offset, &block_id);
    }

    retval = true;
    goto done;

done:
    mock_write_status(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE, offset, status, 
        nullptr, 0U);

    return retval;
}

/**
 * \brief Register a mock callback for block update.
 *
 * \param cb        The callback to register.
 */
void mock_notificationservice::mock_notificationservice::
    register_callback_block_update(
    std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
    cb)
{
    block_update_callback = cb;
}

/**
 * \brief Decode and dispatch a reduce capabilities request.
 *
 * \returns true if the request was dispatched successfully and false
 *          otherwise.
 */
bool mock_notificationservice::mock_notificationservice::
    mock_decode_and_dispatch_reduce_caps(
    uint64_t offset, const uint8_t* payload, size_t payload_size)
{
    bool retval = false;
    uint32_t status = STATUS_SUCCESS;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);

    /* parse the request payload. */
    if (payload_size != sizeof(caps))
    {
        retval = false;
        status = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto done;
    }

    /* copy the caps. */
    memcpy(caps, payload, payload_size);

    /* if the mock callback is set, call it. */
    if (!!reduce_caps_callback)
    {
        status = reduce_caps_callback(offset, caps, payload_size);
    }

    retval = true;
    goto done;

done:
    if (!reduce_caps_status_override)
    {
        mock_write_status(
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS, offset,
            status, nullptr, 0U);
    }

    return retval;
}

/**
 * \brief Register a mock callback for reduce caps request.
 *
 * \param cb        The callback to register.
 */
void mock_notificationservice::mock_notificationservice::
    register_callback_reduce_caps(
    std::function<
        int(uint64_t offset, const uint32_t* caps, size_t size)> cb)
{
    reduce_caps_callback = cb;
}

/**
 * \brief Decode and dispatch a block assertion request.
 *
 * \returns true if the request was dispatched successfully and false
 *          otherwise.
 */
bool mock_notificationservice::mock_notificationservice::
    mock_decode_and_dispatch_block_assertion(
    uint64_t offset, const uint8_t* payload, size_t payload_size)
{
    bool retval = false;
    uint32_t status = STATUS_SUCCESS;
    rcpr_uuid block_id;

    /* parse the request payload. */
    if (payload_size != sizeof(block_id))
    {
        retval = false;
        status = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto done;
    }

    /* copy the block id. */
    memcpy(&block_id, payload, payload_size);
    (void)block_id;

    /* if the mock callback is set, call it. */
    if (!!block_assertion_callback)
    {
        status = block_assertion_callback(offset, &block_id);
    }

    retval = true;
    goto done;

done:
    mock_write_status(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION, offset,
        status, nullptr, 0U);

    return retval;
}

/**
 * \brief Register a mock callback for block assertion.
 *
 * \param cb        The callback to register.
 */
void mock_notificationservice::mock_notificationservice::
    register_callback_block_assertion(
    std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
    cb)
{
    block_assertion_callback = cb;
}

/**
 * \brief Decode and dispatch a block assertion cancel request.
 *
 * \returns true if the request was dispatched successfully and false
 *          otherwise.
 */
bool mock_notificationservice::mock_notificationservice::
    mock_decode_and_dispatch_block_assertion_cancel(
    uint64_t offset, const uint8_t* payload, size_t payload_size)
{
    bool retval = false;
    uint32_t status = STATUS_SUCCESS;

    /* parse the request payload. */
    if (payload_size != 0U)
    {
        retval = false;
        status = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto done;
    }

    (void)payload;

    /* if the mock callback is set, call it. */
    if (!!block_assertion_cancel_callback)
    {
        status = block_assertion_cancel_callback(offset);
    }

    retval = true;
    goto done;

done:
    mock_write_status(
        AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL, offset,
        status, nullptr, 0U);

    return retval;
}

/**
 * \brief Register a mock callback for block assertion cancel.
 *
 * \param cb        The callback to register.
 */
void mock_notificationservice::mock_notificationservice::
    register_callback_block_assertion_cancel(
    std::function<int(uint64_t offset)>
    cb)
{
    block_assertion_cancel_callback = cb;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param offset        The request offset.
 * \param block_id      The request block id.
 */
bool mock_notificationservice::mock_notificationservice::
    request_matches_block_update(
    uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)
{
    status status_code;
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    uint32_t method = 0U;
    uint64_t read_offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* read a request from the test socket. */
    status_code = ipc_read_data_block(testsock, &val, &size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto done;
    }

    /* decode the request. */
    status_code =
        notificationservice_api_decode_request(
            (const uint8_t*)val, size, &method, &read_offset, &payload,
            &payload_size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto cleanup_val;
    }

    /* check the payload size. */
    if (payload_size != sizeof(*block_id))
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the method id. */
    if (AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the offset. */
    if (read_offset != offset)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the block id. */
    if (memcmp(block_id, payload, payload_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* success. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param offset        The request offset.
 * \param caps          The caps buffer.
 * \param caps_size     The size of the caps buffer.
 */
bool mock_notificationservice::mock_notificationservice::
    request_matches_reduce_caps(
    uint64_t offset, const uint32_t* caps, size_t caps_size)
{
    status status_code;
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    uint32_t method = 0U;
    uint64_t read_offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;
    BITCAP(bitcaps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);

    /* read a request from the test socket. */
    status_code = ipc_read_data_block(testsock, &val, &size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto done;
    }

    /* decode the request. */
    status_code =
        notificationservice_api_decode_request(
            (const uint8_t*)val, size, &method, &read_offset, &payload,
            &payload_size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto cleanup_val;
    }

    /* check the payload size. */
    if (payload_size != sizeof(bitcaps) || payload_size != caps_size)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the method id. */
    if (AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the offset. */
    if (read_offset != offset)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the block id. */
    if (memcmp(caps, payload, payload_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* success. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param offset        The request offset.
 * \param block_id      The request block id.
 */
bool mock_notificationservice::mock_notificationservice::
    request_matches_block_assertion(
    uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)
{
    status status_code;
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    uint32_t method = 0U;
    uint64_t read_offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* read a request from the test socket. */
    status_code = ipc_read_data_block(testsock, &val, &size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto done;
    }

    /* decode the request. */
    status_code =
        notificationservice_api_decode_request(
            (const uint8_t*)val, size, &method, &read_offset, &payload,
            &payload_size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto cleanup_val;
    }

    /* check the payload size. */
    if (payload_size != sizeof(*block_id))
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the method id. */
    if (AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the offset. */
    if (read_offset != offset)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the block id. */
    if (memcmp(block_id, payload, payload_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* success. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param offset        The request offset.
 */
bool mock_notificationservice::mock_notificationservice::
    request_matches_block_assertion_cancel(uint64_t offset)
{
    status status_code;
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    uint32_t method = 0U;
    uint64_t read_offset = 0U;
    const uint8_t* payload = nullptr;
    size_t payload_size = 0U;

    /* read a request from the test socket. */
    status_code = ipc_read_data_block(testsock, &val, &size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto done;
    }

    /* decode the request. */
    status_code =
        notificationservice_api_decode_request(
            (const uint8_t*)val, size, &method, &read_offset, &payload,
            &payload_size);
    if (STATUS_SUCCESS != status_code)
    {
        retval = false;
        goto cleanup_val;
    }

    /* check the payload size. */
    if (payload_size != 0U)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the method id. */
    if (AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL
        != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* compare the offset. */
    if (read_offset != offset)
    {
        retval = false;
        goto cleanup_val;
    }

    /* success. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Override the return status for the reduce caps call.
 *
 * \param override_flag     Set to true to disable the status write, and
 *                          false to enable it.
 */
void mock_notificationservice::mock_notificationservice::
    override_reduce_caps_status(bool override_flag)
{
    reduce_caps_status_override = override_flag;
}
