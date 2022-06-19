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
#include <rcpr/uuid.h>
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

    /**
     * \brief Return true if the next popped request matches this request.
     *
     * \param offset        The request offset.
     * \param caps          The caps buffer.
     * \param caps_size     The size of the caps buffer.
     */
    bool request_matches_reduce_caps(
        uint64_t offset, const uint32_t* caps, size_t caps_size);

    /**
     * \brief Return true if the next popped request matches this request.
     *
     * \param offset        The request offset.
     * \param block_id      The request block id.
     */
    bool request_matches_block_update(
        uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id);

    /**
     * \brief Return true if the next popped request matches this request.
     *
     * \param offset        The request offset.
     * \param block_id      The request block id.
     */
    bool request_matches_block_assertion(
        uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id);

    /**
     * \brief Return true if the next popped request matches this request.
     *
     * \param offset        The request offset.
     */
    bool request_matches_block_assertion_cancel(uint64_t offset);

    /**
     * \brief Register a mock callback for reduce caps request.
     *
     * \param cb        The callback to register.
     */
    void register_callback_reduce_caps(
        std::function<
            int(uint64_t offset, const uint32_t* caps, size_t size)> cb);

    /**
     * \brief Override the return status for the reduce caps call.
     *
     * \param override_flag     Set to true to disable the status write, and
     *                          false to enable it.
     */
    void override_reduce_caps_status(bool override_flag);

    /**
     * \brief Register a mock callback for block update.
     *
     * \param cb        The callback to register.
     */
    void register_callback_block_update(
        std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
        cb);

    /**
     * \brief Override the return status for the block update call.
     *
     * \param override_flag     Set to true to disable the status write, and
     *                          false to enable it.
     */
    void override_block_update_status(bool override_flag);

    /**
     * \brief Register a mock callback for block assertion.
     *
     * \param cb        The callback to register.
     */
    void register_callback_block_assertion(
        std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
        cb);

    /**
     * \brief Override the return status for the block assertion call.
     *
     * \param override_flag     Set to true to disable the status write, and
     *                          false to enable it.
     */
    void override_block_assertion_status(bool override_flag);

    /**
     * \brief Register a mock callback for block assertion cancel.
     *
     * \param cb        The callback to register.
     */
    void register_callback_block_assertion_cancel(
        std::function<int(uint64_t offset)>
        cb);

    /**
     * \brief Override the return status for the block assertion cancel call.
     *
     * \param override_flag     Set to true to disable the status write, and
     *                          false to enable it.
     */
    void override_block_assertion_cancel_status(bool override_flag);

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

private:
    int notifysock;
    bool running;
    int testsock;
    int mocksock;
    pid_t mock_pid;
    std::list<std::shared_ptr<mock_request>> request_list;
    RCPR_SYM(allocator)* rcpr_alloc;
    std::function<int(uint64_t offset, const uint32_t* caps, size_t size)>
    reduce_caps_callback;
    std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
    block_update_callback;
    std::function<int(uint64_t offset, const RCPR_SYM(rcpr_uuid)* block_id)>
    block_assertion_callback;
    std::function<int(uint64_t offset)>
    block_assertion_cancel_callback;
    bool reduce_caps_status_override;

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
     * \brief Decode and dispatch a reduce capabilities request.
     *
     * \returns true if the request was dispatched successfully and false
     *          otherwise.
     */
    bool mock_decode_and_dispatch_reduce_caps(
        uint64_t offset, const uint8_t* payload, size_t payload_size);

    /**
     * \brief Decode and dispatch a block update request.
     *
     * \returns true if the request was dispatched successfully and false
     *          otherwise.
     */
    bool mock_decode_and_dispatch_block_update(
        uint64_t offset, const uint8_t* payload, size_t payload_size);

    /**
     * \brief Decode and dispatch a block assertion request.
     *
     * \returns true if the request was dispatched successfully and false
     *          otherwise.
     */
    bool mock_decode_and_dispatch_block_assertion(
        uint64_t offset, const uint8_t* payload, size_t payload_size);

    /**
     * \brief Decode and dispatch a block assertion cancel request.
     *
     * \returns true if the request was dispatched successfully and false
     *          otherwise.
     */
    bool mock_decode_and_dispatch_block_assertion_cancel(
        uint64_t offset, const uint8_t* payload, size_t payload_size);
};
}
