/**
 * \file attestationservice/attestationservice_event_loop.c
 *
 * \brief Perform attestation of transactions in the pending transaction queue.
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

RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

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
status attestationservice_event_loop(attestationservice_instance* inst)
{
    status retval, release_retval;

    for (;;)
    {
        /* TODO - replace with config value. */
        uint64_t sleep_micros = 5000 * 1000;

        /* sleep. */
        TRY_OR_FAIL(
            attestationservice_sleep(inst->sleep_sock, sleep_micros),
            cleanup_inst);

        /* TODO - fill out the rest of the service as per below. */

        /* Query the pending transaction table for new entries. */

        /* if an entry is found, but it has been attested, go back to sleep. */

        /* Otherwise, start the attestation loop. */

            /* Has this entry been signed by an authorized entity? */

            /* If this is a create transaction, is the artifact ID unique and
             * all fields valid for a create?
             */

            /* If this is any other transaction, does the previous transaction
             * match the latest transaction from that artifact (either from the
             * pending transactions or queried from the database?) and does the
             * previous transaction state match the last transaction's state?
             */

            /* Is the transaction id unique? */

            /* NOTE: unique means that it does not exist as an artifact, entity,
             * block, or transaction id anywhere else.
             */

        /* reset the rbtree instances for transactions and artifacts. */
    }

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

    return retval;
}
