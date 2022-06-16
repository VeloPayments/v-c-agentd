/**
 * \file mocks/notificationservice.h
 *
 * Private header for the notificationservice mock.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#pragma once

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

#include <cstring>
#include <functional>
#include <list>
#include <memory>
#include <rcpr/allocator.h>
#include <sys/types.h>

namespace mock_notificationservice {
/**
     * \brief Mock request.
     */
struct mock_request
{
    size_t size;
    void* data;
};

/**
     * \brief Mock request deleter.
     */
struct mock_request_deleter
{
    void operator()(mock_request* p) const
    {
        std::memset(p->data, 0, p->size);
        std::free(p->data);
        std::memset(p, 0, sizeof(mock_request));
        std::free(p);
    }
};

/**
 * \brief Mock notification service.
 *
 * This class is used to mock the notificationservice for isolation tests.
 */
class mock_notificationservice {
public:
    /**
     * \brief Create a mock notificationservice instance that will listen on the
     * given socket when started.
     *
     * \param _notifysock       The socket used to listen for
     *                          notificationservice requests.
     */
    mock_notificationservice(int _notifysock);

    /**
     * \brief Make sure to stop the mock notificationservice if running on
     * destruction.
     */
    ~mock_notificationservice();

    /**
     * \brief Start the mock notificationservice with the current mock settings.
     */
    void start();

    /**
     * \brief Stop the mock notificationservice if running.
     */
    void stop();

private:
    int notifysock;
    bool running;
    int testsock;
    int mocksock;
    pid_t mock_pid;
    std::list<std::shared_ptr<mock_request>> request_list;
    RCPR_SYM(allocator)* rcpr_alloc;

    /**
     * \brief Run the mock notificationservice process.
     *
     * Read request packets from the process and write canned response
     * packets, possibly using the mock override callbacks.
     */
    void mock_process();

    /**
     * \brief Read and dispatch one request.
     *
     * \returns true if a request was read, and false if anything goes
     *          wrong (e.g. a socket was closed).
     */
    bool mock_read_and_dispatch();

    /**
     * \brief Write the status back to the caller.
     *
     * \param method    The method requested.
     * \param offset    The child offset.
     * \param status    The status code.
     * \param payload   The response payload, or ptr if none.
     * \param size      The response payload size.
     */
    void mock_write_status(
        uint32_t method, uint64_t offset, uint32_t status,
        const void* payload, size_t size);
};
}