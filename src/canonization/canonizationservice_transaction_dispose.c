/**
 * \file canonization/canonizationservice_transaction_dispose.c
 *
 * \brief Disposer for transaction instances.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Dispose of a canonizationservice_transaction_t instance.
 *
 * \param ptr           Opaque pointer to the transaction instance to be
 *                      disposed.
 */
void canonizationservice_transaction_dispose(void* ptr)
{
    canonizationservice_transaction_t* txn =
        (canonizationservice_transaction_t*)ptr;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != txn);

    /* clear this value. */
    memset(txn, 0, sizeof(canonizationservice_transaction_t) + txn->cert_size);
}
