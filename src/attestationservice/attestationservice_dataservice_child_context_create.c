/**
 * \file
 * attestationservice/attestationservice_dataservice_child_context_create.c
 *
 * \brief Create a child context for the data service connection.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Create a child context for communicating with the data service.
 *
 * \param data_sock         Socket for the data service.
 * \param alloc             The allocator to use for this operation.
 * \param child_context     Pointer to receive the child context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_dataservice_child_context_create(
    psock* data_sock, rcpr_allocator* alloc, uint32_t* child_context)
{
    status retval;
    uint32_t offset, status;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    /* use all capabilities. The supervisor has already capped us. */
    BITCAP_INIT_TRUE(caps);

    /* send a request to create the child context. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_child_context_create(
            data_sock, caps, sizeof(caps)),
        done);

    /* read the response. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_child_context_create(
            data_sock, alloc, &offset, &status, child_context),
        done);
    TRY_OR_FAIL(status, done);

done:
    return retval;
}
