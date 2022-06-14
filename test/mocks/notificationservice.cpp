/**
 * \file mocks/notificationservice.cpp
 *
 * Mock notificationservice methods.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

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

    /* read a request to the data service mock. */
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

    /* decode this message. */
    if (AGENTD_STATUS_SUCCESS !=
            notificationservice_api_decode_request(
                val, size, &method, &offset, &payload, &payload_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* for now, just write a success status. */
    mock_write_status(
        method, 0U,
        AGENTD_STATUS_SUCCESS, nullptr, 0);
    retval = true;

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
