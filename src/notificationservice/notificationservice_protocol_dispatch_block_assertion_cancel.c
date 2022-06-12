/**
 * \file
 * notificationservice/notificationservice_protocol_dispatch_block_assertion_cancel.c
 *
 * \brief Dispatch a block assertion cancellation request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_rbtree;

/**
 * \brief Dispatch a block assertion cancellation request.
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
status notificationservice_protocol_dispatch_block_assertion_cancel(
    notificationservice_protocol_fiber_context* context, uint64_t offset,
    const uint8_t* /*payload*/, size_t payload_size)
{
    status retval, status_retval;

    /* check to see if this call is permissible. */
    if (!BITCAP_ISSET(
            context->inst->caps, NOTIFICATIONSERVICE_API_CAP_BLOCK_ASSERTION))
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;
        goto report_status;
    }

    /* verify that the payload size is zero. */
    if (0U != payload_size)
    {
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto report_status;
    }

    /* attempt to delete the assertion entry. */
    retval = rbtree_delete(NULL, context->inst->assertions, &offset);
    if (STATUS_SUCCESS != retval && ERROR_RBTREE_NOT_FOUND != retval)
    {
        /* report any fatal error. */
        goto report_status;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto report_status;

report_status:
    status_retval =
        notificationservice_protocol_send_response(
            context,
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL,
            offset, retval);
    if (STATUS_SUCCESS != status_retval)
    {
        retval = status_retval;
    }

    return retval;
}
