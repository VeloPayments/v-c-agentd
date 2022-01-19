/**
 * \file protocolservice/protocolservice_internal.h
 *
 * \brief Internal header for the protocol service.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <config.h>
#include <agentd/protocolservice/api.h>
#include <rcpr/allocator.h>
#include <rcpr/fiber.h>
#include <rcpr/message.h>
#include <rcpr/psock.h>
#include <rcpr/rbtree.h>
#include <rcpr/resource/protected.h>
#include <rcpr/uuid.h>
#include <stdbool.h>
#include <vccrypt/suite.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus*/

/** \brief The accept endpoint fiber stack size. */
#define ACCEPT_ENDPOINT_FIBER_STACK_SIZE 16384

/** \brief The random endpoint fiber stack size. */
#define RANDOM_ENDPOINT_STACK_SIZE 16384

/** \brief The manager fiber stack size. */
#define MANAGER_FIBER_STACK_SIZE 16384

/** \brief The protocol fiber stack size. */
#define PROTOCOL_FIBER_STACK_SIZE 16384

/** \brief The control fiber stack size. */
#define CONTROL_FIBER_STACK_SIZE 16384

/**
 * \brief An authorized entity.
 */
typedef struct protocolservice_authorized_entity
protocolservice_authorized_entity;

struct protocolservice_authorized_entity
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(rcpr_uuid) entity_uuid;
    vccrypt_buffer_t encryption_pubkey;
    vccrypt_buffer_t signing_pubkey;
};

/**
 * \brief Context structure for the protocol service.
 */
typedef struct protocolservice_context
protocolservice_context;

struct protocolservice_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    allocator_options_t vpr_alloc;
    RCPR_SYM(fiber_scheduler)* sched;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) data_endpoint_addr;
    RCPR_SYM(mailbox_address) random_endpoint_addr;
    RCPR_SYM(fiber)* main_fiber;
    RCPR_SYM(rbtree)* authorized_entity_dict;
    vccrypt_suite_options_t suite;
    RCPR_SYM(rcpr_uuid) agentd_uuid;
    vccrypt_buffer_t agentd_enc_pubkey;
    vccrypt_buffer_t agentd_enc_privkey;
    vccrypt_buffer_t agentd_sign_pubkey;
    vccrypt_buffer_t agentd_sign_privkey;
    bool private_key_set;
    size_t protocol_fiber_count;
    bool quiesce;
    bool terminate;
};
 
/**
 * \brief Context structure for the protocol service accept endpoint.
 */
typedef struct protocolservice_accept_endpoint_context
protocolservice_accept_endpoint_context;

struct protocolservice_accept_endpoint_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    protocolservice_context* ctx;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(psock)* acceptsock;
};

/**
 * \brief Context structure for the protocol service random endpoint.
 */
typedef struct protocolservice_random_endpoint_context
protocolservice_random_endpoint_context;

struct protocolservice_random_endpoint_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) addr;
    RCPR_SYM(psock)* randomsock;
};

/**
 * \brief Request message for the random service endpoint.
 */
typedef struct protocolservice_random_request_message
protocolservice_random_request_message;

struct protocolservice_random_request_message
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    size_t size;
};

/**
 * \brief Response message for the random service endpoint.
 */
typedef struct protocolservice_random_response_message
protocolservice_random_response_message;

struct protocolservice_random_response_message
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    void* data;
    size_t size;
};

/**
 * \brief Context structure for a protocol fiber.
 */
typedef struct protocolservice_protocol_fiber_context
protocolservice_protocol_fiber_context;

struct protocolservice_protocol_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    protocolservice_context* ctx;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(psock)* protosock;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_key_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;
    uint64_t client_iv;
    uint64_t server_iv;
    RCPR_SYM(rcpr_uuid) entity_uuid;
    RCPR_SYM(mailbox_address) return_addr;
    const protocolservice_authorized_entity* entity;
};

/**
 * \brief Context structure for the control fiber.
 */
typedef struct protocolservice_control_fiber_context
protocolservice_control_fiber_context;

struct protocolservice_control_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    protocolservice_context* ctx;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(psock)* controlsock;
    bool should_exit;
};

/**
 * \brief Create and add the protocol service data service endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The fiber scheduler to which this endpoint fiber should
 *                      be assigned.
 * \param datasock      The socket connection to the data service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int datasock);

/**
 * \brief Create and add the protocol service random endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The fiber scheduler to which this endpoint fiber should
 *                      be assigned.
 * \param randomsock    The socket connection to the random service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_randomservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int randomsock);

/**
 * \brief Create the protocol service context.
 *
 * \param ctx           Pointer to receive the protocol service context pointer
 *                      on success.
 * \param alloc         The allocator to use to create this context.
 * \param sched         The fiber scheduler.
 * \param random_addr   The mailbox address of the random service endpoint.
 * \param data_addr     The mailbox address of the data service endpoint.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_context_create(
    protocolservice_context** ctx, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, RCPR_SYM(mailbox_address) random_addr,
    RCPR_SYM(mailbox_address) data_addr);

/**
 * \brief Create and add the protocol service management fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this management fiber should be
 *                      assigned.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_management_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched);

/**
 * \brief Create and add the protocol service control fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param controlsock   The socket connection to the supervisor.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int controlsock);

/**
 * \brief Create and add the protocol service accept fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param protosock     The socket connection to the listen service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int protosock);

/**
 * \brief Entry point for the protocol service fiber manager fiber.
 *
 * This fiber manages cleanup for fibers as they stop.
 *
 * \param vsched        The type erased scheduler.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_fiber_manager_entry(void* vsched);

/**
 * \brief Release the protocol service context.
 *
 * \param r             The protocol service context to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Release the protocol service accept endpoint fiber context.
 *
 * \param r             The protocol service accept endpoint fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_endpoint_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the protocol service accept endpoint fiber.
 *
 * This fiber accepts connections from the listen service and assigns protocol
 * fiber instances to manage them.
 *
 * \param vctx          The type erased accept endopint context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_accept_endpoint_fiber_entry(void* vctx);

/**
 * \brief Release the protocol service random endpoint context.
 *
 * \param r             The protocol service random endpoint context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_endpoint_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the protocol service random endpoint fiber.
 *
 * This fiber forwards requests to the random service and returns responses.
 *
 * \param vctx          The type erased random endopint context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_endpoint_fiber_entry(void* vctx);

/**
 * \brief Create a request message payload for the random service endpoint.
 *
 * \param payload       Pointer to hold the created payload structure on
 *                      success. This resource is owned by the caller.
 * \param alloc         The allocator to use to create this payload.
 * \param size          The number of bytes requested.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_request_message_create(
    protocolservice_random_request_message** payload,
    RCPR_SYM(allocator)* alloc, size_t size);

/**
 * \brief Release a protocol service random request payload resource.
 *
 * \param r             The payload resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_request_message_release(RCPR_SYM(resource)* r);

/**
 * \brief Create a response message payload for the random service endpoint.
 *
 * \param payload       Pointer to hold the created payload structure on
 *                      success. This resource is owned by the caller.
 * \param alloc         The allocator to use to create this payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_response_message_create(
    protocolservice_random_response_message** payload,
    RCPR_SYM(allocator)* alloc);

/**
 * \brief Release a protocol service random response payload resource.
 *
 * \param r             The payload resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_response_message_release(RCPR_SYM(resource)* r);

/**
 * \brief Handle unexpected resume events in fibers relating to the protocol
 * service.
 *
 * \param context                   Opaque reference to protocol service
 *                                  context.
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
status protocolservice_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* fib,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* resume_param, const RCPR_SYM(rcpr_uuid)* expected_resume_disc_id,
    int expected_resume_event);

/**
 * \brief Create and add a protocol service protocol fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param sock          The client socket for this fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int sock);

/**
 * \brief Entry point for a protocol service protocol fiber.
 *
 * This fiber manages the protocol for a single client connection.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_entry(void* vctx);

/**
 * \brief Release a protocol service protocol fiber context.
 *
 * \param r             The protocol service  protocol fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_fiber_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Write an error response to the socket.
 *
 * \param ctx           The protocol fiber context for this socket.
 * \param request_id    The id of the request that caused the error.
 * \param status        The status code of the error.
 * \param offset        The request offset that caused the error.
 * \param encrypted     Set to true if this packet should be encrypted.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_write_error_response(
    protocolservice_protocol_fiber_context* ctx, int request_id, int status,
    uint32_t offset, bool encrypted);

/**
 * \brief Perform the handshake for the protocol.
 *
 * \param ctx       The protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_handle_handshake(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Read the handshake request from the client.
 *
 * \param ctx       The protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_handshake_req(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Entry point for the protocol service control fiber.
 *
 * This fiber manages the control protocol for the protocol service.
 *
 * \param vctx          The type erased control fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_entry(void* vctx);

/**
 * \brief Release the protocol service control fiber context.
 *
 * \param r             The protocol service control fiber context to be
 *                      released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_context_release(RCPR_SYM(resource)* r);

/**
 * \brief Force an exit from the protocol service.
 *
 * \param ctx           The protocol service context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_force_exit(protocolservice_context* ctx);

/**
 * \brief Decode and dispatch a control packet from the supervisor.
 *
 * \param ctx           The protocol service control fiber context.
 * \param req           Pointer to the control packet.
 * \param size          The size of the control packet.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_decode_and_dispatch(
    protocolservice_control_fiber_context* ctx, const void* req, size_t size);

/**
 * \brief Dispatch an auth entity add control request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_auth_entity_add(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size);

/**
 * \brief Dispatch a private key set request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_private_key_set(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size);

/**
 * \brief Dispatch a finalize request
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_finalize(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size);

/**
 * \brief Write a response to the control socket.
 *
 * \param ctx           The control fiber context.
 * \param request_id    The id of the request.
 * \param status        The status code.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_write_response(
    protocolservice_control_fiber_context* ctx, int request_id, int status);

/**
 * \brief Compare two opaque \ref rcpr_uuid values.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side of the comparison.
 * \param rhs           The right-hand side of the comparison.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs.
 */
RCPR_SYM(rcpr_comparison_result) protocolservice_authorized_entity_uuid_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given an authorized entity resource handle, return its \ref rcpr_uuid
 * value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_authorized_entity_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Add an authorized entity to the protocol service context.
 *
 * \param ctx                   The context to which the entity should be added.
 * \param entity_uuid           The uuid of this entity.
 * \param encryption_pubkey     The encryption public key of this entity.
 * \param signing_pubkey        The signing public key of this entity.
 *
 * \note This method transfers ownership of the public keys on success; they do
 * not have to be disposed afterward.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_add(
    protocolservice_context* ctx, const RCPR_SYM(rcpr_uuid)* entity_uuid,
    vccrypt_buffer_t* encryption_pubkey, vccrypt_buffer_t* signing_pubkey);

/**
 * \brief Look up an authorized entity by entity id.
 *
 * \param entity                Pointer to the authorized entity pointer to
 *                              receive this entity on success.
 * \param ctx                   The context from which to look up this entity.
 * \param entity_uuid           The uuid of the entity to look up.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_lookup(
    const protocolservice_authorized_entity** entity,
    protocolservice_protocol_fiber_context* ctx,
    const RCPR_SYM(rcpr_uuid)* entity_uuid);

/**
 * \brief Release an authorized entity resource.
 *
 * \param r             The authorized entity resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_release(RCPR_SYM(resource)* r);

/**
 * \brief Read random bytes from the random service endpoint.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_read_random_bytes(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Compute a shared secret based on the nonce data gathered during the
 * handshake, the server private key, and the client public key.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_compute_shared_secret(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Write the response to the handshake request to the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_handshake_req_resp(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Read the handshake ack request from the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_handshake_ack_req(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Write the handshake ack response to the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_handshake_ack_resp(
    protocolservice_protocol_fiber_context* ctx);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  /*__cplusplus*/
