/**
 * \file notificationservice/notificationservice_protocol_dispatch_reduce_caps.c
 *
 * \brief Dispatch a reduce caps request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

/**
 * \brief Dispatch a reduce caps request.
 *
 * \param context                   Notificationservice protocol fiber context.
 * \param offset                    The client-supplied request offset.
 * \param payload                   Payload data for this request.
 * \param payload_size              The size of the payload data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS if the fiber should retry the yield.
 *      - a non-zero error code if the fiber should exit.
 */
status notificationservice_protocol_dispatch_reduce_caps(
    notificationservice_protocol_fiber_context* context, uint64_t offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval, status_retval;
    BITCAP(intersect, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);

    /* check to see if this call is permissible. */
    if (!BITCAP_ISSET(
            context->inst->caps, NOTIFICATIONSERVICE_API_CAP_REDUCE_CAPS))
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;
        goto report_status;
    }

    /* verify that the payload is set and the payload size is valid. */
    if (NULL == payload || sizeof(intersect) != payload_size)
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto report_status;
    }

    /* set the intersect. */
    memcpy(intersect, payload, payload_size);

    /* intersect both values. */
    BITCAP_INTERSECT(context->inst->caps, context->inst->caps, intersect);

    /* success. */
    retval = STATUS_SUCCESS;
    goto report_status;

report_status:
    status_retval =
        notificationservice_protocol_send_response(
            context, AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS,
            offset, retval);
    if (STATUS_SUCCESS != status_retval)
    {
        retval = status_retval;
    }

    return retval;
}
