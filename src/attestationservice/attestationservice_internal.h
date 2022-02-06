/**
 * \file attestationservice/attestationservice_internal.h
 *
 * \brief Internal header for the attestation service.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/dataservice/async_api.h>
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
    allocator_options_t vpr_alloc;
    RCPR_SYM(fiber)* fib;
    RCPR_SYM(psock)* sleep_sock;
    RCPR_SYM(psock)* data_sock;
    RCPR_SYM(psock)* log_sock;
    RCPR_SYM(rbtree)* transaction_tree;
    RCPR_SYM(rbtree)* artifact_tree;
};

/**
 * \brief The transaction record resource value.
 */
typedef struct transaction_record_value transaction_record_value;
struct transaction_record_value
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    data_transaction_node_t data;
};

/**
 * \brief The artifact record resource value.
 */
typedef struct artifact_record_value artifact_record_value;
struct artifact_record_value
{
    RCPR_SYM(resource) hdr;
    RCPR_SYM(allocator)* alloc;
    data_artifact_record_t data;
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

/**
 * \brief Verify that the given transaction has valid fields.
 *
 * \param inst              The attestation service instance.
 * \param txn_node          The transaction node.
 * \param txn_data          The transaction data.
 * \param txn_data_size     The size of the transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_verify_txn_has_valid_fields(
    attestationservice_instance* inst, const data_transaction_node_t* txn_node,
    const void* txn_data, size_t txn_data_size);

/**
 * \brief Verify that the given transaction is in the correct sequence.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The data service child context.
 * \param txn_node          The transaction node.
 * \param txn_data          The transaction data.
 * \param txn_data_size     The size of the transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_verify_txn_is_in_correct_sequence(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node,
    const void* txn_data, size_t txn_data_size);

/**
 * \brief Verify that the given transaction id / artifact id is unique.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The data service child context.
 * \param txn_node          The transaction node.
 * \param txn_data          The transaction data.
 * \param txn_data_size     The size of the transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_verify_txn_is_unique(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node,
    const void* txn_data, size_t txn_data_size);

/**
 * \brief Create a child context for communicating with the data service.
 *
 * \param inst              The attestation service instance.
 * \param child_context     Pointer to receive the child context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_dataservice_child_context_create(
    attestationservice_instance* inst, uint32_t* child_context);

/**
 * \brief Query the data service for either the first or the next pending
 * transaction.
 *
 * \param data_sock         Socket for the data service.
 * \param vpr_alloc         The VPR allocator to use for this operation.
 * \param alloc             The allocator to use for this operation.
 * \param child_context     The child context to use for this operation.
 * \param txn_id            The next transaction id, or NULL if the first
 *                          transaction ID should be queried.
 * \param txn_node          The transaction node to return.
 * \param txn_data          The transaction data to return.
 * \param txn_data_size     The size of the returned transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_dataservice_query_pending_transaction(
    RCPR_SYM(psock)* data_sock, allocator_options_t* vpr_alloc,
    RCPR_SYM(allocator)* alloc, uint32_t child_context,
    RCPR_SYM(rcpr_uuid)* txn_id, data_transaction_node_t* txn_node,
    void** txn_data, size_t* txn_data_size);

/**
 * \brief Query the data service for an artifact record by artifact id.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The child context to use for this operation.
 * \param artifact_id       The artifact id to query.
 * \param artifact          The artifact record to return.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_get_or_query_artifact(
    attestationservice_instance* inst,
    uint32_t child_context, RCPR_SYM(rcpr_uuid)* artifact_id,
    artifact_record_value** artifact);

/**
 * \brief Update the artifact record or insert a new one.
 *
 * \param inst              The attestation service instance.
 * \param artifact          The artifact record to use for update or insert.
 *
 * \note On success, this function takes ownership of this record value and will
 * release it if it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_update_or_insert_artifact(
    attestationservice_instance* inst,
    artifact_record_value* artifact);

/**
 * \brief Promote a transaction to attested.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The child context to use for this operation.
 * \param txn_node          The transaction node to promote.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_dataservice_transaction_promote(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node);

/**
 * \brief Add a transaction to the red-black tree.
 *
 * If this is a create transaction, add the artifact to the rbtree. Otherwise,
 * update the artifact record in the artifact rbtree.
 *
 * \param inst              The attestation service instance.
 * \param child_context     The dataservice child context.
 * \param txn_node          The transaction node to add.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_transaction_tree_insert(
    attestationservice_instance* inst, uint32_t child_context,
    const data_transaction_node_t* txn_node);

/**
 * \brief Create a transaction record to insert into the transaction tree.
 *
 * \param txn               Pointer to receive the pointer to the created
 *                          transaction record instance.
 * \param inst              The attestation service instance.
 * \param txn_node          The transaction node to create this record from.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_transaction_record_value_create(
    transaction_record_value** txn, attestationservice_instance* inst,
    const data_transaction_node_t* txn_node);

/**
 * \brief Create an artifact record to insert into the artifact tree.
 *
 * \param artifact          Pointer to receive the pointer to the created
 *                          artifact record instance.
 * \param inst              The attestation service instance.
 * \param txn_node          The transaction node to create this record from.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_artifact_record_value_create(
    artifact_record_value** artifact, attestationservice_instance* inst,
    const data_transaction_node_t* txn_node);

/**
 * \brief Create an artifact record to insert into the artifact tree, from an
 * artifact record.
 *
 * \param artifact          Pointer to receive the pointer to the created
 *                          artifact record instance.
 * \param inst              The attestation service instance.
 * \param artifact_node     The artifact record to create this record from.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_artifact_record_value_create_from_artifact(
    artifact_record_value** artifact, attestationservice_instance* inst,
    const data_artifact_record_t* artifact_node);

/**
 * \brief Release an artifact_record_value resource.
 *
 * \param r         The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_artifact_record_value_resource_release(
    RCPR_SYM(resource)* r);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_ATTESTATIONSERVICE_INTERNAL_HEADER_GUARD*/
