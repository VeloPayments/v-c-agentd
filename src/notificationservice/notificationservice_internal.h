/**
 * \file notificationservice/notificationservice_internal.h
 *
 * \brief Internal header for the notification service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <config.h>
#include <agentd/bitcap.h>
#include <agentd/notificationservice/api.h>
#include <rcpr/allocator.h>
#include <rcpr/fiber.h>
#include <rcpr/message.h>
#include <rcpr/psock.h>
#include <rcpr/rbtree.h>
#include <rcpr/resource/protected.h>
#include <rcpr/uuid.h>
#include <stdbool.h>

/** \brief The notificationservice protocol fiber stack size. */
#define NOTIFICATIONSERVICE_PROTOCOL_FIBER_STACK_SIZE (1024*1024)

/** \brief The notificationservice protocol endpoint fiber stack size. */
#define NOTIFICATIONSERVICE_PROTOCOL_ENDPOINT_FIBER_STACK_SIZE 16384

/**
 * \brief The notificationservice context is the main context for the service.
 */
typedef struct notificationservice_context notificationservice_context;

struct notificationservice_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(fiber_scheduler)* sched;
    RCPR_SYM(fiber)* main_fiber;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    bool quiesce;
    bool terminate;
};

/**
 * \brief The notificationservice instance is a specific socket protocol
 * instance.
 */
typedef struct notificationservice_instance notificationservice_instance;

struct notificationservice_instance
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(psock)* protosock;
    RCPR_SYM(mailbox_address) outbound_addr;
    notificationservice_context* ctx;
    BITCAP(caps, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
};

/**
 * \brief The notificationservice protocol fiber context.
 */
typedef struct notificationservice_protocol_fiber_context
notificationservice_protocol_fiber_context;

struct notificationservice_protocol_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    notificationservice_instance* inst;
    RCPR_SYM(mailbox_address) return_addr;
    RCPR_SYM(fiber)* fib;
};

/**
 * \brief The notificationservice protocol outbound endpoint fiber context.
 */
typedef struct notificationservice_protocol_outbound_endpoint_fiber_context
notificationservice_protocol_outbound_endpoint_fiber_context;

struct notificationservice_protocol_outbound_endpoint_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    notificationservice_instance* inst;
    RCPR_SYM(fiber)* fib;
};

/**
 * \brief The notificationservice protocol outbound endpoint message payload.
 */
typedef struct notificationservice_protocol_outbound_endpoint_message_payload
notificationservice_protocol_outbound_endpoint_message_payload;

struct notificationservice_protocol_outbound_endpoint_message_payload
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    uint8_t* payload_data;
    size_t payload_data_size;
};

/**
 * \brief Create a notificationservice context.
 *
 * \param ctx       Pointer to receive the context on success.
 * \param alloc     The allocator to use for this operation.
 * \param sched     The scheduler to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_context_create(
    notificationservice_context** ctx, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched);

/**
 * \brief Release a notificationservice resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_context_resource_release(RCPR_SYM(resource)* r);

/**
 * \brief Create a notificationservice instance.
 *
 * \param inst      Pointer to receive the instance on success.
 * \param ctx       The root context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_instance_create(
    notificationservice_instance** inst, notificationservice_context* ctx);

/**
 * \brief Release a notificationservice instance resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_instance_resource_release(RCPR_SYM(resource)* r);

/**
 * \brief Create and add a protocol fiber to the scheduler.
 *
 * \param alloc     The allocator to use for this operation.
 * \param inst      The instance for this fiber.
 * \param sock      The socket for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_add(
    RCPR_SYM(allocator)* alloc, notificationservice_instance* inst, int sock);

/**
 * \brief Create an outbound endpoint fiber for an instance.
 *
 * \param alloc     The allocator to use for this operation.
 * \param inst      The instance for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_add(
    RCPR_SYM(allocator)* alloc, notificationservice_instance* inst);

/**
 * \brief Release a notificationservice protocol fiber context resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_context_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Entry point for a notificationservice protocol fiber.
 *
 * This fiber manages a notificationservice protocol instance.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_fiber_entry(void* vctx);

/**
 * \brief Release a notificationservice protocol outbound endpoint fiber context
 * resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_fiber_context_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Entry point for a notificationservice protocol outbound endpoint
 * fiber.
 *
 * This fiber manages a notificationservice protocol outbound endpoint instance.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_fiber_entry(void* vctx);

/**
 * \brief Handle unexpected resume events in fibers relating to the notification
 * service.
 *
 * \param context                   Opaque reference to notificationservice
 *                                  context.
 * \param fib                       The fiber experiencing this event.
 * \param resume_disc_id            The unexpected resume discipline id.
 * \param resume_event              The unexpected resume event.
 * \param resume_param              The unexpected resume parameter.
 * \param expected_resume_disc_id   The expected discipline id.
 * \param expected_resume_event     The expected resume event.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* fib,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* resume_param, const RCPR_SYM(rcpr_uuid)* expected_resume_disc_id,
    int expected_resume_event);

/**
 * \brief Read, decode, and dispatch a request from the client socket.
 *
 * \param context                   Notificationservice protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_read_decode_and_dispatch_packet(
    notificationservice_protocol_fiber_context* context);

/**
 * \brief Dispatch a reduce caps request.
 *
 * \param context                   Notificationservice protocol fiber context.
 * \param offset                    The client-supplied request offset.
 * \param payload                   Payload data for this request.
 * \param payload_size              The size of the payload data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_dispatch_reduce_caps(
    notificationservice_protocol_fiber_context* context, uint64_t offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Create a message payload, taking ownership of the payload data.
 *
 * \note On success, the data passed to this function is owned by the created
 * resource and will be freed using the provided allocator when that resource is
 * released.
 *
 * \param payload       Pointer to receive the created payload.
 * \param alloc         The allocator to use for this operation.
 * \param data          Allocated payload data to be passed to this resource.
 * \param size          The allocated payload size.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_message_payload_create(
    notificationservice_protocol_outbound_endpoint_message_payload** payload,
    RCPR_SYM(allocator)* alloc, uint8_t* data, size_t size);

/**
 * \brief Release a message payload resource.
 *
 * \param r             The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status
notificationservice_protocol_outbound_endpoint_message_payload_resource_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Send a response payload to the outbound endpoint.
 *
 * \param ctx           The context for this operation.
 * \param method_id     The method id for the request.
 * \param offset        The offset for the response.
 * \param status_code   The status for the response.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_send_response(
    notificationservice_protocol_fiber_context* ctx, uint32_t method_id,
    uint64_t offset, uint32_t status_code);
