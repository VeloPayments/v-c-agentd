/**
 * \file attestationservice/attestationservice_internal.h
 *
 * \brief Internal header for the attestation service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD

#include <rcpr/fiber.h>
#include <rcpr/psock.h>
#include <rcpr/rbtree.h>
#include <rcpr/resource/protected.h>
#include <rcpr/thread.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#define SIGNAL_STATE_QUIESCE            0x00000000
#define SIGNAL_STATE_TERMINATE          0x00000001

/**
 * \brief The attestation service instance structure.
 */
typedef struct attestationservice_instance attestationservice_instance;
struct attestationservice_instance
{
    RCPR_SYM(resource) hdr;

    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(psock)* sleep_sock;
    RCPR_SYM(psock)* data_sock;
    RCPR_SYM(psock)* log_sock;
    RCPR_SYM(rbtree)* transaction_tree;
    RCPR_SYM(rbtree)* artifact_tree;
};

/**
 * \brief Create a signal thread for the attestation service.
 *
 * The signal thread listens for signals, and upon detecting one, translates the
 * signal into either a quiesce or a termination request.
 *
 * \param th            The thread instance to create.
 * \param alloc         The allocator to use for this operation.
 * \param signal_fd     Pointer to receive the descriptor to which the reaper
 *                      fiber should listen in order to forward quesce or
 *                      termination events to the fiber scheduler.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_signal_thread(
    RCPR_SYM(thread)** th, RCPR_SYM(allocator)* alloc, int* signal_fd);

/**
 * \brief Create a sleep thread for the attestation service.
 *
 * The sleep thread sleeps for a specified amount of time when signaled over its
 * descriptor, and then responds when it's time to wake up.
 *
 * \param th            The thread instance to create.
 * \param alloc         The allocator to use for this operation.
 * \param sleep_fd      Pointer to receive the sleep descriptor to be used by
 *                      the main fiber to communicate with this thread.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_sleep_thread(
    RCPR_SYM(thread)** th, RCPR_SYM(allocator)* alloc, int* sleep_fd);

/**
 * \brief Create a fiber to listen to quiesce / terminate events, and broadcast
 * these to all other fibers to reap them.
 *
 * \param th            The thread instance to create.
 * \param alloc         The allocator to use for this operation.
 * \param signal_fd     The descriptor that this fiber uses to listen for
 *                      events.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_reaper_fiber(
    RCPR_SYM(fiber)** fib, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int signal_fd);

/**
 * \brief Create an attestation service instance to pass to the attestation
 * service event loop.
 *
 * \param inst          Pointer to receive the instance.
 * \param alloc         The allocator to use for this operation.
 * \param sched         The fiber scheduler for this instance.
 * \param sleep_fd      The socket descriptor to use when communicating with the
 *                      sleep thread.
 * \param data_fd       The socket descriptor to use when communicating with the
 *                      data service instance dedicated to this attestation
 *                      service.
 * \param log_fd        The socket descriptor to use when communicating with the
 *                      logging service.
 * \param control_fd    The socket descriptor to use when communicating with the
 *                      supervisor during the bootstrap process.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_instance(
    attestationservice_instance** inst, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int sleep_fd, int data_fd, int log_fd,
    int control_fd);

/**
 * \brief The event loop for the attestation service. This event loop sleeps
 * until activation time, then queries the process queue for transactions that
 * have not yet been attested, and performs attestation on these.
 *
 * \param inst          The attestation service instance to use for this loop.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_event_loop(attestationservice_instance* inst);

/**
 * \brief Sleep for the given amount of time using the sleep thread.
 *
 * \param sleep_sock    Socket for sleep thread communication.
 * \param sleep_time    Sleep time in microseconds.
 *
 * \returns a status code on success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero failure code on failure.
 */
status attestationservice_sleep(
    RCPR_SYM(psock)* sleep_sock, uint64_t sleep_time);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD*/
