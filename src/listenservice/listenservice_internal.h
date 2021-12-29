/**
 * \file listenservice/listenservice_internal.h
 *
 * \brief Internal header for the listen service.
 *
 * \copyright 2019-2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/listenservice.h>
#include <rcpr/allocator.h>
#include <rcpr/fiber.h>
#include <rcpr/psock.h>
#include <rcpr/message.h>
#include <rcpr/resource/protected.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/** \brief The accept endpoint fiber stack size. */
#define ACCEPT_ENDPOINT_STACK_SIZE 16384

/** \brief The listen fiber stack size. */
#define LISTEN_FIBER_STACK_SIZE 16384

/** \brief The manager fiber stack size. */
#define MANAGER_FIBER_STACK_SIZE 16384

/**
 * \brief Context structure for a listen fiber.
 */
typedef struct listenservice_listen_fiber_context
listenservice_listen_fiber_context;

struct listenservice_listen_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(psock)* listen_socket;
    RCPR_SYM(fiber_scheduler)* sched;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) endpoint_addr;
    RCPR_SYM(mailbox_address) return_addr;
    RCPR_SYM(fiber)* fib;
    bool quiesce;
};

/**
 * \brief Context structure for the accept endpoint.
 */
typedef struct listenservice_accept_endpoint_context
listenservice_accept_endpoint_context;

struct listenservice_accept_endpoint_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(psock)* accept_socket;
    RCPR_SYM(fiber_scheduler)* sched;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) endpoint_addr;
    RCPR_SYM(fiber)* fib;
    bool quiesce;
};

/**
 * \brief Payload for an accept message.
 */
typedef struct listenservice_accept_message listenservice_accept_message;

struct listenservice_accept_message
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int desc;
};

/**
 * \brief Create and add the management fiber for the listen service.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this management fiber should be
 *                      assigned.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_management_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched);

/**
 * \brief Create and add a listen fiber for the listen service.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this listen fiber should be
 *                      assigned.
 * \param endpoint_addr The endpoint's mailbox address.
 * \param desc          The descriptor on which this fiber listens.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched,
    RCPR_SYM(mailbox_address) endpoint_addr, int desc);

/**
 * \brief Create and add the listen service accept endpoint fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this endpoint fiber should be
 *                      assigned.
 * \param endpoint_addr Pointer to receive the endpoint's mailbox address.
 * \param acceptsock    The socket descriptor to send accepted sockets.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched,
    RCPR_SYM(mailbox_address)* endpoint_addr, int acceptsock);

/**
 * \brief Entry point for the listen service fiber manager fiber.
 *
 * This fiber manages cleanup for fibers as they stop.
 *
 * \param vsched        The type erased scheduler.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_fiber_manager_entry(void* vsched);

/**
 * \brief Release a listen fiber context.
 *
 * \param r             The listen fiber context resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the listen service listen fiber.
 *
 * This fiber listens to a socket for new connections, and passes these to the
 * accept endpoint, where they are sent to the protocol service.
 *
 * \param vctx          The type erased context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_entry(void* vctx);

/**
 * \brief Handle unexpected resume events in the listen fiber.
 *
 * \param context                   Opaque reference to listenservice listen
 *                                  fiber context.
 * \param fib                       The fiber experiencing this event.
 * \param resume_disc_id            The unexpected resume discipline id.
 * \param resume_event              The unexpected resume event.
 * \param resume_param              The unexpected resume parameter.
 * \param expected_resume_disc_id   The expected discipline id.
 * \param expected_resume_event     The expected resume event.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS if the fiber should retry the yield.
 *      - a non-zero error code if the fiber should exit.
 */
status listenservice_listen_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* fib,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* resume_param, const RCPR_SYM(rcpr_uuid)* expected_resume_disc_id,
    int expected_resume_event);

/**
 * \brief Release the accept endpoint fiber context.
 *
 * \param r             The accept endpoint fiber context resource to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the accept endpoint fiber.
 *
 * This fiber receives sockets from each of the listen fibers and forwards these
 * to the protocol service.
 *
 * \param vctx          The type erased context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_fiber_entry(void* vctx);

/**
 * \brief Handle unexpected resume events in the accept endpoint fiber.
 *
 * \param context                   Opaque reference to listenservice accept
 *                                  endpoint fiber context.
 * \param fib                       The fiber experiencing this event.
 * \param resume_disc_id            The unexpected resume discipline id.
 * \param resume_event              The unexpected resume event.
 * \param resume_param              The unexpected resume parameter.
 * \param expected_resume_disc_id   The expected discipline id.
 * \param expected_resume_event     The expected resume event.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS if the fiber should retry the yield.
 *      - a non-zero error code if the fiber should exit.
 */
status listenservice_accept_endpoint_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* fib,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* resume_param, const RCPR_SYM(rcpr_uuid)* expected_resume_disc_id,
    int expected_resume_event);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD*/
