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
#include <vcblockchain/protocol/data.h>
#include <vccrypt/suite.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus*/

/** \brief The accept endpoint fiber stack size. */
#define ACCEPT_ENDPOINT_FIBER_STACK_SIZE 16384

/** \brief The random endpoint fiber stack size. */
#define RANDOM_ENDPOINT_STACK_SIZE 16384

/** \brief The dataservice endpoint fiber stack size. */
#define DATASERVICE_ENDPOINT_STACK_SIZE 16384

/** \brief The manager fiber stack size. */
#define MANAGER_FIBER_STACK_SIZE 16384

/** \brief The protocol fiber stack size. */
#define PROTOCOL_FIBER_STACK_SIZE 16384

/** \brief The control fiber stack size. */
#define CONTROL_FIBER_STACK_SIZE 16384

/** \brief The size of the notificationservice endpoint fiber. */
#define NOTIFICATION_ENDPOINT_FIBER_STACK_SIZE 16384

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
    RCPR_SYM(rbtree)* capabilities;
};

/**
 * \brief An authorized entity capability key.
 */
typedef struct protocolservice_authorized_entity_capability_key
protocolservice_authorized_entity_capability_key;

struct protocolservice_authorized_entity_capability_key
{
    RCPR_SYM(rcpr_uuid) subject_id;
    RCPR_SYM(rcpr_uuid) verb_id;
    RCPR_SYM(rcpr_uuid) object_id;
};

/**
 * \brief An authorized entity capability.
 */
typedef struct protocolservice_authorized_entity_capability
protocolservice_authorized_entity_capability;

struct protocolservice_authorized_entity_capability
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    protocolservice_authorized_entity_capability_key key;
};

/**
 * \brief A mailbox context entry.
 */
typedef struct protocolservice_dataservice_mailbox_context_entry
protocolservice_dataservice_mailbox_context_entry;

struct protocolservice_dataservice_mailbox_context_entry
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int reference_count;
    RCPR_SYM(mailbox_address) addr;
    uint32_t context;
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
    RCPR_SYM(mailbox_address) notificationservice_endpoint_addr;
    RCPR_SYM(fiber)* main_fiber;
    RCPR_SYM(rbtree)* authorized_entity_dict;
    RCPR_SYM(rbtree)* extended_api_dict;
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
 * \brief Context structure for the protocol service notification service
 * endpoint.
 */
typedef struct protocolservice_notificationservice_fiber_context
protocolservice_notificationservice_fiber_context;

struct protocolservice_notificationservice_fiber_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) notify_addr;
    RCPR_SYM(psock)* notifysock;
    RCPR_SYM(rbtree)* client_xlat_map;
    RCPR_SYM(rbtree)* server_xlat_map;
    protocolservice_context* ctx;
    int reference_count;
    uint64_t request_offset_counter;
};

/**
 * \brief Block assertion request message for the notification service endpoint.
 */
typedef struct protocolservice_notificationservice_block_assertion_request
protocolservice_notificationservice_block_assertion_request;

struct protocolservice_notificationservice_block_assertion_request
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    uint32_t req_offset;
    RCPR_SYM(rcpr_uuid) block_id;
    RCPR_SYM(mailbox_address) reply_addr;
    bool cancel;
};

/**
 * \brief Block assertion response message for the notification service
 * endpoint.
 */
typedef struct protocolservice_notificationservice_block_assertion_response
protocolservice_notificationservice_block_assertion_response;

struct protocolservice_notificationservice_block_assertion_response
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    bool success;
    uint64_t offset;
};

/**
 * \brief Entry in the notificationservice translation table.
 */
typedef struct protocolservice_notificationservice_xlat_entry
protocolservice_notificationservice_xlat_entry;

struct protocolservice_notificationservice_xlat_entry
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    int reference_count;
    RCPR_SYM(mailbox_address) client_addr;
    uint64_t server_offset;
    uint64_t client_offset;
};

/**
 * \brief Entry in the protocol extended api response translation table.
 */
typedef struct protocolservice_extended_api_response_xlat_entry
protocolservice_extended_api_response_xlat_entry;

struct protocolservice_extended_api_response_xlat_entry
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    uint64_t server_offset;
    uint32_t client_offset;
    RCPR_SYM(mailbox_address) client_return_address;
};

/**
 * \brief Context structure for the protocol service dataservice endpoint.
 */
typedef struct protocolservice_dataservice_endpoint_context
protocolservice_dataservice_endpoint_context;

struct protocolservice_dataservice_endpoint_context
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    allocator_options_t vpr_alloc;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(fiber_scheduler_discipline)* msgdisc;
    RCPR_SYM(mailbox_address) addr;
    RCPR_SYM(psock)* datasock;
    RCPR_SYM(rbtree)* mailbox_context_tree;
    RCPR_SYM(rbtree)* context_mailbox_tree;
    protocolservice_context* ctx;
};

/**
 * \brief A request message payload for the dataservice endpoint.
 */
typedef struct protocolservice_dataservice_request_message
protocolservice_dataservice_request_message;

struct protocolservice_dataservice_request_message
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    uint32_t protocol_request_id;
    uint32_t request_id;
    uint32_t offset;
    uint64_t data;
    vccrypt_buffer_t payload;
};

/**
 * \brief Request IDs supported by the dataservice endpoint.
 */
enum protocolservice_dataservice_endpoint_request_id
{
    PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_CONTEXT_OPEN,
    PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_CONTEXT_CLOSE,
    PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_DATASERVICE_REQ,
};

/**
 * \brief Protocol write endpoint message.
 */
typedef struct protocolservice_protocol_write_endpoint_message
protocolservice_protocol_write_endpoint_message;

struct protocolservice_protocol_write_endpoint_message
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    uint32_t message_type;
    uint32_t original_request_id;
    uint32_t offset;
    vccrypt_buffer_t payload;
};

/**
 * \brief Message types for the protocol write endpoint.
 */
enum protocolservice_protocol_write_endpoint_message_type
{
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_MESSAGE_SHUTDOWN,
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_DATASERVICE_CONTEXT_CREATE_MSG,
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_DATASERVICE_MSG,
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_NOTIFICATION_MSG,
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_PACKET,
    PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_ERROR_MESSAGE,
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
    int reference_count;
    bool req_shutdown;
    bool shutdown;
    bool extended_api_enabled;
    bool extended_api_can_respond;
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
    RCPR_SYM(mailbox_address) fiber_addr;
    const protocolservice_authorized_entity* entity;
    bool dataservice_context_opened;
    bool latest_block_id_assertion_set;
    uint32_t latest_block_id_assertion_client_offset;
    uint64_t latest_block_id_assertion_server_offset;
    uint64_t extended_api_offset;
    RCPR_SYM(rbtree)* extended_api_offset_dict;
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
 * \brief Entry in the extended api dictionary.
 */
typedef struct protocolservice_extended_api_dict_entry
protocolservice_extended_api_dict_entry;

struct protocolservice_extended_api_dict_entry
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(rcpr_uuid) entity_id;
    protocolservice_protocol_fiber_context* ctx;
};

/**
 * \brief Send a message to the dataservice endpoint.
 *
 * \note This function takes ownership of the contents of the request buffer on
 * success. These contents are moved to the internal message sent to the
 * endpoint and are no longer available to the caller when ownership is taken.
 *
 * \param ctx               The protocol fiber context.
 * \param protocol_req_id   The protocol request id.
 * \param request_offset    The protocol request offset of the message.
 * \param request_buffer    The buffer holding the encoded request message.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_send_request(
    protocolservice_protocol_fiber_context* ctx, uint32_t protocol_req_id,
    uint32_t request_offset, vccrypt_buffer_t* request_buffer);

/**
 * \brief Create and add the protocol service data service endpoint fiber.
 *
 * \param ctx           Pointer to the saved context. NOTE THAT THIS IS OWNED
 *                      BY THE DATASERVICE ENDPOINT.
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
    protocolservice_dataservice_endpoint_context** ctx,
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int datasock);

/**
 * \brief Release a protocolservice dataservice endpoint context resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_context_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the protocol service dataservice endpoint fiber.
 *
 * This fiber manages communication with the dataservice instance assigned to
 * the protocol service.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_fiber_entry(void* vctx);

/**
 * \brief Decode and dispatch a dataservice endpoint request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_endpoint_decode_and_dispatch(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload);

/**
 * \brief Decode and dispatch a dataservice context open request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status pde_decode_and_dispatch_req_context_open(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload);

/**
 * \brief Decode and dispatch a dataservice context close request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status pde_decode_and_dispatch_req_context_close(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload);

/**
 * \brief Decode and dispatch a generic dataservice request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status pde_decode_and_dispatch_req_dataservice_req(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload);

/**
 * \brief Report an error for an invalid dataservice endpoint request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status pde_decode_and_dispatch_invalid_req(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload);

/**
 * \brief Create a dataservice endpoint request message.
 *
 * \param req_payload       Pointer to the pointer to be updated on success.
 * \param ctx               The protocol fiber context.
 * \param protocol_req_id   The protocol request id.
 * \param request_id        The request id.
 * \param offset            The offset code.
 * \param data              Optional data for the request.
 * \param payload           The payload data.
 *
 * If \p payload is not NULL, then the data in \p payload is moved into an
 * internal structure that is part of the request message owned by the caller
 * on success. Either on success or failure, \p payload should be disposed
 * after this call.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_request_message_create(
    protocolservice_dataservice_request_message** req_payload,
    protocolservice_protocol_fiber_context* ctx, uint32_t protocol_req_id,
    uint32_t request_id, uint32_t offset, uint64_t data,
    vccrypt_buffer_t* payload);

/**
 * \brief Release a dataservice endpoint request message.
 *
 * \param r             The message to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_request_message_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Create a write endpoint message.
 *
 * \param reply_payload     Pointer to the pointer to be updated on success.
 * \param ctx               The endpoint context.
 * \param message_type      The message type.
 * \param original_req_id   The original protocol request id.
 * \param offset            The offset code.
 * \param payload           The payload data.
 *
 * If \p payload is not NULL, then the data in \p payload is moved into an
 * internal structure that is part of the response message owned by the caller
 * on success. Either on success or failure, \p payload should be disposed
 * after this call.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_message_create(
    protocolservice_protocol_write_endpoint_message** reply_payload,
    protocolservice_context* ctx, uint32_t message_type,
    uint32_t original_req_id, uint32_t offset, const void* payload,
    size_t payload_size);

/**
 * \brief Release a write endpoint message.
 *
 * \param r             The message to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_message_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Release a mailbox context resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_mailbox_context_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Compare two opaque \ref mailbox_address values.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_dataservice_endpoint_mailbox_context_tree_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given a mailbox_context resource handle, return its \ref
 * mailbox_address value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_dataservice_endpoint_mailbox_context_tree_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Compare two opaque context values.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_dataservice_endpoint_context_mailbox_tree_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given an mailbox_context resource handle, return its context value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_dataservice_endpoint_context_mailbox_tree_key(
    void* context, const RCPR_SYM(resource)* r);

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
 * \brief Create and add the protocol service notification endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param ctx           The protocol service context for this operation.
 * \param notifysock    The socket connection to the notification service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, protocolservice_context* ctx,
    int notifysock);

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
 * \param status_       The status code of the error.
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
 * \brief Send an error response to the protocol write endpoint.
 *
 * \param ctx           The protocol fiber context for this socket.
 * \param request_id    The id of the request that caused the error.
 * \param status_       The status code of the error.
 * \param offset        The request offset that caused the error.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_send_error_response_message(
    protocolservice_protocol_fiber_context* ctx, int request_id, int status,
    uint32_t offset);

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
 * \brief Dispatch an auth entity capability add control request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_auth_entity_capability_add(
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
 * \brief Compare two opaque \ref authorized_entity_capability_key values.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_authorized_entity_capabilities_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given an authorized entity capability resource handle, return its
 * \ref authorized_entity_capability_key.
 * value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_authorized_entity_capabilities_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Check to see if the given capabilities are set.
 *
 * \param entity        The entity to check.
 * \param subject_id    The subject id.
 * \param verb_id       The verb id.
 * \param object_id     The object id.
 *
 * \returns true if the capability is set and false otherwise.
 */
bool protocolservice_authorized_entity_capability_check(
    const protocolservice_authorized_entity* entity,
    const RCPR_SYM(rcpr_uuid)* subject_id, const RCPR_SYM(rcpr_uuid)* verb_id,
    const RCPR_SYM(rcpr_uuid)* object_id);

/**
 * \brief Create a protocolservice_authorized_entity_capability instance.
 *
 * \param cap           Pointer to receive this instance.
 * \param alloc         The allocator to use for this operation.
 * \param subject_id    The subject UUID.
 * \param verb_id       The verb UUID.
 * \param object_id     The object UUID.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_capability_create(
    protocolservice_authorized_entity_capability** cap,
    RCPR_SYM(allocator)* alloc, const RCPR_SYM(rcpr_uuid)* subject_id,
    const RCPR_SYM(rcpr_uuid)* verb_id, const RCPR_SYM(rcpr_uuid)* object_id);

/**
 * \brief Release a protocolserice authorized entity capability resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_authorized_entity_capability_resource_release(
    RCPR_SYM(resource)* r);

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

/**
 * \brief Request a data service context for this connection.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_request_data_service_context(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Close the data service context for this connection.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_close_data_service_context(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Map the user capabilities in a form that the data service open context
 * request can understand.
 *
 * \param payload           The buffer to receive the payload to the open
 *                          context request. This buffer must be uninitialized.
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_map_user_capabilities(
    vccrypt_buffer_t* payload, protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Create and add a protocol write endpoint instance to the fiber
 * manager.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_add(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Entry point for a protocol service protocol write endpoint fiber.
 *
 * This fiber writes messages from the messaging discipline to the client.
 *
 * \param vctx          The type erased protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_entry(void* vctx);

/**
 * \brief Instruct the write endpoint fiber to shut down.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_shutdown_write_endpoint(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Release a protocol write endpoint message.
 *
 * \param r             The payload resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_message_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Decode and dispatch a message sent to the protocol write endpoint.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The message to be decoded and dispatched.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_decode_and_dispatch(
    protocolservice_protocol_fiber_context* ctx, RCPR_SYM(message)* msg);

/**
 * \brief Decode and dispatch a response message from the data service.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_message(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a latest block id get response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_block_id_latest_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a block read response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_block_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a transaction read response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_transaction_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch an artifact read response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_artifact_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a transaction submit response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_transaction_submit(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a block id by height get response.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_dataservice_block_id_by_height_get(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Decode and dispatch a response message from the notificationservice.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param payload       The message payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_pwe_dnd_notificationservice_message(
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_protocol_write_endpoint_message* payload);

/**
 * \brief Write a packet to the peer.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The packet message to be written
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_write_packet(
    protocolservice_protocol_fiber_context* ctx,
    const protocolservice_protocol_write_endpoint_message* msg);

/**
 * \brief Write a packet to the peer.
 *
 * \param ctx           The protocol service protocol fiber context.
 * \param msg           The raw message buffer to write.
 * \param size          The size of the message buffer to write.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_write_raw_packet(
    protocolservice_protocol_fiber_context* ctx, const void* msg, size_t size);

/**
 * \brief Read a packet from the client socket, and decode / dispatch it.
 *
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_read_decode_and_dispatch_packet(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Decode and dispatch a packet from the client.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_id        The request id of the packet.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_decode_and_dispatch(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_id,
    uint32_t request_offset, const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a latest block id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_latest_block_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a transaction submit request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_transaction_submit(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block by id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_block_by_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block get next id request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_block_id_next_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block get prev id request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_block_id_prev_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block id by height get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_block_id_by_height_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a transaction by id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_transaction_by_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a next transaction id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_transaction_id_next_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a prev transaction id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_transaction_id_prev_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a transaction block id get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_transaction_block_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch an artifact get first transaction id request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_artifact_first_transaction_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch an artifact get last transaction id request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_artifact_last_transaction_id_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block assertion request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_assert_latest_block_id(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a block assertion cancellation request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_assert_latest_block_id_cancel(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch an extended API enable request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_extended_api_enable(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch an extended API send request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_extended_api_sendrecv(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch an extended API client response request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_extended_api_sendresp(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a status get request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_status_get(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode and dispatch a close request.
 *
 * \param ctx               The protocol service protocol fiber context.
 * \param request_offset    The request offset of the packet.
 * \param payload           The payload of the packet.
 * \param payload_size      The size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_dnd_close(
    protocolservice_protocol_fiber_context* ctx, uint32_t request_offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Release the protocol service notification service fiber context.
 *
 * \param r             The context to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_fiber_context_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Entry point for the protocol service notificationservice endpoint
 * fiber.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_fiber_entry(void* vctx);

/**
 * \brief Entry point for the protocol service notificationservice write
 * endpoint fiber.
 *
 * \param vctx          The type erased context for this endpoint fiber.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_write_endpoint_fiber_entry(
    void* vctx);

/**
 * \brief Handle an assert block request from the protocol.
 *
 * This method creates an assert block request for the notification service
 * endpoint, sends it, and receives a response with the notification service
 * offset.
 *
 * \param ctx           The protocolservice protocol context for this request.
 * \param req_offset    The request offset from the client request.
 * \param block_id      The block id for this request.
 * \param offset        Pointer to be populated with the notificationservice
 *                      offset for this request, which can be used to cancel it.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_handle_assert_block_request(
    protocolservice_protocol_fiber_context* ctx, uint32_t req_offset,
    const vpr_uuid* block_id, uint64_t* offset);

/**
 * \brief Handle an assert block cancellation request from the protocol.
 *
 * This method creates an assert block cancel request for the notification
 * service endpoint, sends it, and receives a response with the notification
 * service offset.
 *
 * \param ctx           The protocolservice protocol context for this request.
 * \param req_offset    The request offset from the client request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_handle_assert_block_cancel_request(
    protocolservice_protocol_fiber_context* ctx, uint32_t req_offset);

/**
 * \brief Create a block assertion request message for the notificationservice
 * endpoint.
 *
 * \param payload       The pointer to receive this created message payload.
 * \param alloc         The allocator for this operation.
 * \param req_offset    The client request offset.
 * \param block_id      The block id for this operation.
 * \param return_addr   The return address to send the invalidation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_request_create(
    protocolservice_notificationservice_block_assertion_request** payload,
    RCPR_SYM(allocator)* alloc, uint32_t req_offset,
    const RCPR_SYM(rcpr_uuid)* block_id, RCPR_SYM(mailbox_address) return_addr);

/**
 * \brief Release the block assertion request resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_request_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Compare two opaque \ref protocolservice_notificationservice_xlat_entry
 * values.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_notificationservice_client_xlat_map_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given a protocolservice_notificationservice_xlat_entry resource handle,
 * return its \ref client_offset value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_notificationservice_client_xlat_map_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Compare two opaque \ref protocolservice_notificationservice_xlat_entry
 * values.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_notificationservice_server_xlat_map_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given a protocolservice_notificationservice_xlat_entry resource handle,
 * return its \ref server_offset value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_notificationservice_server_xlat_map_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Add a request to the notificationservice translation maps.
 *
 * \param ctx           The endpoint context.
 * \param msg_offset    The server-side offset.
 * \param client_addr   The client_side mailbox address.
 * \param req_offset    The client-side offset.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_xlat_map_add(
    protocolservice_notificationservice_fiber_context* ctx,
    uint64_t msg_offset, RCPR_SYM(mailbox_address) client_addr,
    uint32_t req_offset);

/**
 * \brief Send a response for a request sent to the notificationservice.
 *
 * \param ctx           The endpoint context.
 * \param reply_addr    The reply address.
 * \param msg_offset    The server-side offset.
 * \param success       Flag indicating success or failure.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_send_request_response(
    protocolservice_notificationservice_fiber_context* ctx,
    RCPR_SYM(mailbox_address) reply_addr, uint64_t msg_offset, bool success);

/**
 * \brief Reduce the reference count and possibly release a translation
 * table entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_xlat_entry_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Create a notificationservice endpoint block assertion response message
 * payload.
 *
 * \param payload       Pointer to receive the payload on success.
 * \param alloc         The allocator to use for this operation.
 * \param offset        The offset value to send in the response.
 * \param success       Flag to indicate whether the request was successful.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_response_create(
    protocolservice_notificationservice_block_assertion_response** payload,
    RCPR_SYM(allocator)* alloc, uint64_t offset, bool success);

/**
 * \brief Release a notificationservice endpoint block assertion response
 * message payload resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_response_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Look up a return address from a notificationservice offset, and remove
 * the entries from the lookup tables.
 *
 * \param return_addr       Pointer to receive the return address on success.
 * \param req_offset        Pointer to receive the client-side request offset.
 * \param ctx               The endpoint fiber context.
 * \param offset            The notificationservice offset.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_lookup_return_address_from_offset(
    RCPR_SYM(mailbox_address)* return_address, uint32_t* req_offset,
    protocolservice_notificationservice_fiber_context* ctx, uint64_t offset);

/**
 * \brief Compare two opaque \ref protocolservice_extended_api_dict_entry
 * keys.
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
RCPR_SYM(rcpr_comparison_result) protocolservice_extended_api_dict_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given a \ref protocolservice_extended_api_dict_entry resource handle,
 * return its \ref rcpr_uuid key.
 *
 * \param context       Unused.
 * \param r             The resource handle of an extended api dict entry.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_extended_api_dict_entry_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Create an extended API dictionary entry.
 *
 * \param entry         Pointer to receive the entry on success.
 * \param alloc         The allocator to use for this operation.
 * \param entity_id     The entity id for this entry.
 * \param ctx           A weak reference to the protocolservice protocol fiber
 *                      context for this entry.
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_dict_entry_create(
    protocolservice_extended_api_dict_entry** entry, RCPR_SYM(allocator)* alloc,
    const RCPR_SYM(rcpr_uuid)* entity_id,
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Release an extended API dictionary entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_dict_entry_resource_release(
    RCPR_SYM(resource)* r);

/**
 * \brief Route the extended API for a given sentinel entity.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      entity connection.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_route_extended_api_for_entity(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Unroute the extended API for a given sentinel entity.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      entity connection.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_unroute_extended_api_for_entity(
    protocolservice_protocol_fiber_context* ctx);

/**
 * \brief Forward an extended API request to the appropriate sentinel.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      operation.
 * \param req           The request to forward.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_extended_api_send_req(
    protocolservice_protocol_fiber_context* ctx,
    const protocol_req_extended_api* req);

/**
 * \brief Create an extended API client request payload buffer.
 *
 * \param buffer        Uninitialized buffer to receive the payload on success.
 * \param offset        Pointer to receive the offset on success.
 * \param ctx           The sending client's context.
 * \param entry         The receiving sentinel's routing entry.
 * \param req           The extended API request to send to the sentinel.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_create_extended_api_clientreq_payload(
    vccrypt_buffer_t* buffer, uint64_t* offset,
    protocolservice_protocol_fiber_context* ctx,
    protocolservice_extended_api_dict_entry* entry,
    const protocol_req_extended_api* req);

/**
 * \brief Compare two opaque \ref
 * protocolservice_extended_api_response_xlat_entry keys.
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
RCPR_SYM(rcpr_comparison_result)
protocolservice_extended_api_response_xlat_entry_compare(
    void* context, const void* lhs, const void* rhs);

/**
 * \brief Given a \ref protocolservice_extended_api_response_xlat_entry resource
 * handle, return its \ref rcpr_uuid key.
 *
 * \param context       Unused.
 * \param r             The resource handle of an extended api xlat entry.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_extended_api_response_xlat_entry_key(
    void* context, const RCPR_SYM(resource)* r);

/**
 * \brief Add an extended API response xlat entry to the given sentinel context.
 *
 * \param ctx           The context to which this entry is added.
 * \param server_offset The server offset.
 * \param client_offset The client offset.
 * \param return_addr   The client return address.
 *
 * \returns an error code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_response_xlat_entry_add(
    protocolservice_protocol_fiber_context* ctx, uint64_t server_offset,
    uint32_t client_offset, RCPR_SYM(mailbox_address) return_addr);

/**
 * \brief Release an extended API xlat table entry resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_extended_api_response_xlat_entry_release(
    RCPR_SYM(resource)* r);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  /*__cplusplus*/
