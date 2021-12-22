/**
 * \file agentd/signalthread.h
 *
 * \brief Signal management thread for agentd fiber services.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <rcpr/fiber.h>
#include <rcpr/psock.h>
#include <rcpr/status.h>
#include <rcpr/thread.h>
#include <unistd.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Signal states returned by the signal thread.
 */
enum signalstate_enum
{
    /** \brief Notify the main thread that all fibers should quiesce. */
    SIGNAL_STATE_QUIESCE,

    /** \brief Notify the main thread that all fibers should terminate. */
    SIGNAL_STATE_TERMINATE,

    /** \brief An invalid state.  Error out. */
    SIGNAL_STATE_INVALID,
};

/**
 * \brief Create a signal thread to manage signals for a given agentd service.
 *
 * The signal thread allows signals to be processed independently of the fiber
 * scheduler. This is a requirement for fiber management, since fibers can't be
 * interrupted by signals.
 *
 * \param th            Pointer to the thread pointer, populated by the thread
 *                      instance on success.
 * \param signal_sock   Pointer to the psock pointer, populated by the signal
 *                      psock on success.
 * \param alloc         The allocator to use for this operation.
 * \param calling_fiber The caller's fiber instance, which is tied to the signal
 *                      socket on success.
 * \param sleep_usecs   The number of microseconds to sleep between quiesce and
 *                      terminate.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_create(
    RCPR_SYM(thread)** th, RCPR_SYM(psock)** signal_sock,
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber)* calling_fiber,
    useconds_t sleep_usecs);

/**
 * \brief Perform a blocking read on the signal thread socket.
 *
 * \note that the signal thread socket is set up as an async wrapped socket, so
 * blocking will result in a yield to the fiber scheduler.
 *
 * \param signal_sock           The socket from which to read the state.
 * \param state                 Pointer to the uint64_t field to receive the
 *                              updated state on success.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_read_state(RCPR_SYM(psock)* signal_sock, uint64_t* state);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus
