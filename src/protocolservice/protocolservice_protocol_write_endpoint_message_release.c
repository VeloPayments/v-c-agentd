/**
 * \file
 * protocolservice/protocolservice_protocol_write_endpoint_message_release.c
 *
 * \brief Release a write endpoint message payload.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <agentd/status_codes.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release a protocol write endpoint message.
 *
 * \param r             The payload resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_write_endpoint_message_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_protocol_write_endpoint_message* msg =
        (protocolservice_protocol_write_endpoint_message*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_valid_protocolservice_protocol_write_endpoint_message(msg));

    /* cache the allocator. */
    rcpr_allocator* alloc = msg->alloc;

    /* dispose the message data, if set. */
    if (NULL != msg->payload.data)
    {
        dispose((disposable_t*)&msg->payload);
    }

    /* reclaim memory. */
    return rcpr_allocator_reclaim(alloc, msg);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
