/**
 * \file protocolservice/protocolservice_request_data_service_context.c
 *
 * \brief Send a request to the data service endpoint for a context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

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
    protocolservice_protocol_fiber_context* ctx)
{
    /* TODO - fill out stub. */
    (void)ctx;

    return STATUS_SUCCESS;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
