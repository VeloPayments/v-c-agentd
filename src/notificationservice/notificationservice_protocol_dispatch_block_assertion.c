/**
 * \file
 * notificationservice/notificationservice_protocol_dispatch_block_assertion.c
 *
 * \brief Dispatch a block assertion request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vccrypt/compare.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Dispatch a block assertion request.
 *
 * \param context                   Notificationservice protocol fiber context.
 * \param offset                    The client-supplied request offset.
 * \param payload                   Payload data for this request.
 * \param payload_size              The size of the payload data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_dispatch_block_assertion(
    notificationservice_protocol_fiber_context* context, uint64_t offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval, status_retval;
    rcpr_uuid block_id;
    bool should_report_status = true;

    /* check to see if this call is permissible. */
    if (!BITCAP_ISSET(
            context->inst->caps, NOTIFICATIONSERVICE_API_CAP_BLOCK_ASSERTION))
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;
        goto report_status;
    }

    /* verify the payload size. */
    if (sizeof(block_id) != payload_size)
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto report_status;
    }

    /* copy the block id. */
    memcpy(&block_id, payload, payload_size);

    /* compare this block id to the latest block id. */
    /* if it does not match, then invalidate the assertion. */
    if (crypto_memcmp(&block_id, &context->inst->ctx->latest_block_id, 16))
    {
        /* a successful response is an invalidation. */
        retval = STATUS_SUCCESS;
        goto clear_block_id;
    }

    /* if it does not match, then add this to our assertion tree. */
    /* when a new block id is registered, all assertions will be invalidated. */
    retval = notificationservice_assertion_entry_add(context, offset);
    if (STATUS_SUCCESS != retval)
    {
        goto clear_block_id;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    should_report_status = false;
    goto clear_block_id;

clear_block_id:
    memset(&block_id, 0, sizeof(block_id));

report_status:
    if (should_report_status)
    {
        status_retval =
            notificationservice_protocol_send_response(
                context,
                AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
                offset, retval);
        if (STATUS_SUCCESS != status_retval)
        {
            retval = status_retval;
        }
    }

    return retval;
}
