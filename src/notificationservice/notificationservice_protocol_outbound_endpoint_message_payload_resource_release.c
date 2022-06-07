/**
 * \file
 * notificationservice/notificationservice_protocol_outbound_endpoint_message_payload_resource_release.c
 *
 * \brief Release a message payload resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a message payload resource.
 *
 * \param r             The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status
notificationservice_protocol_outbound_endpoint_message_payload_resource_release(
    RCPR_SYM(resource)* r)
{
    status payload_reclaim_retval;
    status reclaim_retval;
    notificationservice_protocol_outbound_endpoint_message_payload* payload =
        (notificationservice_protocol_outbound_endpoint_message_payload*)r;

    /* cache allocator. */
    rcpr_allocator* alloc = payload->alloc;

    /* clear payload memory. */
    memset(payload->payload_data, 0, payload->payload_data_size);

    /* reclaim payload data memory. */
    payload_reclaim_retval =
        rcpr_allocator_reclaim(alloc, payload->payload_data);

    /* reclaim payload. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, payload);

    /* decode return value. */
    if (STATUS_SUCCESS != payload_reclaim_retval)
    {
        return payload_reclaim_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
