/**
 * \file attestationservice/attestationservice_verify_txn_is_unique.c
 *
 * \brief Verify that the given transaction id is unique, and the artifact id is
 * unique.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/attestationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "attestationservice_internal.h"

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
    const void* txn_data, size_t txn_data_size)
{
    /* TODO - fill out. */
    (void)inst;
    (void)child_context;
    (void)txn_node;
    (void)txn_data;
    (void)txn_data_size;

    return -1;
}
