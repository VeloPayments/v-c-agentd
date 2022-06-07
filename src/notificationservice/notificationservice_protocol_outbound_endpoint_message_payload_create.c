/**
 * \file
 * notificationservice/notificationservice_protocol_outbound_endpoint_message_payload_create.c
 *
 * \brief Create the message payload for an outbound endpoint message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a message payload, taking ownership of the payload data.
 *
 * \note On success, the data passed to this function is owned by the created
 * resource and will be freed using the provided allocator when that resource is
 * released.
 *
 * \param payload       Pointer to receive the created payload.
 * \param alloc         The allocator to use for this operation.
 * \param data          Allocated payload data to be passed to this resource.
 * \param size          The allocated payload size.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_outbound_endpoint_message_payload_create(
    notificationservice_protocol_outbound_endpoint_message_payload** payload,
    RCPR_SYM(allocator)* alloc, uint8_t* data, size_t size)
{
    status retval;
    notificationservice_protocol_outbound_endpoint_message_payload* tmp;

    /* allocate memory for the payload. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear this memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize it as a resource. */
    resource_init(
        &tmp->hdr,
        &notificationservice_protocol_outbound_endpoint_message_payload_resource_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->payload_data = data;
    tmp->payload_data_size = size;

    /* success. */
    retval = STATUS_SUCCESS;
    *payload = tmp;
    goto done;

done:
    return retval;
}
