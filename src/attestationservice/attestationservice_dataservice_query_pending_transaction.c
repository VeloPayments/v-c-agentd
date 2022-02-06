/**
 * \file
 * attestationservice/attestationservice_dataservice_query_pending_transaction.c
 *
 * \brief Query a pending transaction from the pending transaction queue.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_uuid;

/* forward decls. */
static status attestationservice_dataservice_query_first_pending_txn(
    psock* data_sock, rcpr_allocator* alloc, uint32_t child_context,
    data_transaction_node_t* txn_node, void** txn_data, size_t* txn_data_size);
static status attestationservice_dataservice_query_pending_txn(
    psock* data_sock, allocator_options_t* vpr_alloc, rcpr_allocator* alloc,
    uint32_t child_context, rcpr_uuid* txn_id,
    data_transaction_node_t* txn_node, void** txn_data, size_t* txn_data_size);

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
    psock* data_sock, allocator_options_t* vpr_alloc, rcpr_allocator* alloc,
    uint32_t child_context, rcpr_uuid* txn_id,
    data_transaction_node_t* txn_node, void** txn_data, size_t* txn_data_size)
{
    /* should we query the first pending transaction? */
    if (NULL == txn_id)
    {
        return
            attestationservice_dataservice_query_first_pending_txn(
                data_sock, alloc, child_context, txn_node,
                txn_data, txn_data_size);
    }
    /* no, query by transaction id. */
    else
    {
        return
            attestationservice_dataservice_query_pending_txn(
                data_sock, vpr_alloc, alloc, child_context, txn_id, txn_node,
                txn_data, txn_data_size);
    }
}

/**
 * \brief Query the data service for the first pending transaction.
 *
 * \param data_sock         Socket for the data service.
 * \param alloc             The allocator to use for this operation.
 * \param child_context     The child context to use for this operation.
 * \param txn_node          The transaction node to return.
 * \param txn_data          The transaction data to return.
 * \param txn_data_size     The size of the returned transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status attestationservice_dataservice_query_first_pending_txn(
    psock* data_sock, rcpr_allocator* alloc, uint32_t child_context,
    data_transaction_node_t* txn_node, void** txn_data, size_t* txn_data_size)
{
    status retval;
    uint32_t status, offset;

    /* send a request to the data service. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_transaction_get_first(
            data_sock, child_context),
        done);

    /* read the response. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_transaction_get_first(
            data_sock, alloc, &offset, &status, txn_node,
            txn_data, txn_data_size),
        done);

    /* set the return value to the status from the data service. */
    retval = status;

done:
    return retval;
}

/**
 * \brief Query the data service for the next pending transaction.
 *
 * \param data_sock         Socket for the data service.
 * \param alloc             The allocator to use for this operation.
 * \param child_context     The child context to use for this operation.
 * \param txn_node          The transaction node to return.
 * \param txn_data          The transaction data to return.
 * \param txn_data_size     The size of the returned transaction data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status attestationservice_dataservice_query_pending_txn(
    psock* data_sock, allocator_options_t* vpr_alloc, rcpr_allocator* alloc,
    uint32_t child_context, rcpr_uuid* txn_id,
    data_transaction_node_t* txn_node, void** txn_data, size_t* txn_data_size)
{
    status retval;
    uint32_t status, offset;

    /* send a request to the data service. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_transaction_get(
            data_sock, vpr_alloc, child_context, txn_id->data),
        done);

    /* read the response. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_transaction_get(
            data_sock, alloc, &offset, &status, txn_node,
            txn_data, txn_data_size),
        done);

    /* set the return value to the status from the data service. */
    retval = status;

done:
    return retval;
}
