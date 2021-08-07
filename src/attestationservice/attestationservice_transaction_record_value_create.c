/**
 * \file attestationservice/attestationservice_transaction_record_value_create.c
 *
 * \brief Create a transaction record value.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/* forward decls. */
static status transaction_record_value_resource_release(resource* r);

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
    const data_transaction_node_t* txn_node)
{
    status retval;
    transaction_record_value* tmp;

    /* allocate memory for this record. */
    TRY_OR_FAIL(
        rcpr_allocator_allocate(
            inst->alloc, (void**)&tmp, sizeof(transaction_record_value)),
        done);

    /* clear the structure. */
    memset(tmp, 0, sizeof(transaction_record_value));

    /* initialize the resource. */
    resource_init(&tmp->hdr, &transaction_record_value_resource_release);

    /* copy init values. */
    memcpy(&tmp->data, txn_node, sizeof(data_transaction_node_t));
    tmp->alloc = inst->alloc;

    /* success. */
    retval = STATUS_SUCCESS;
    *txn = tmp;

done:
    return retval;
}

/**
 * \brief Release a transaction_record_value resource.
 *
 * \param r         The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status transaction_record_value_resource_release(resource* r)
{
    transaction_record_value* txn = (transaction_record_value*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = txn->alloc;

    /* clear the structure. */
    memset(txn, 0, sizeof(transaction_record_value));

    return rcpr_allocator_reclaim(alloc, txn);
}
