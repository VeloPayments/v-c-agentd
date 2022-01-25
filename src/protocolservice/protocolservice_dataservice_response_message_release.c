/**
 * \file protocolservice/protocolservice_dataservice_response_message_release.c
 *
 * \brief Release a dataservice response message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a dataservice endpoint response message.
 *
 * \param r             The message to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_response_message_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_dataservice_response_message* msg =
        (protocolservice_dataservice_response_message*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_response_message_valid(msg));

    /* cache allocator. */
    rcpr_allocator* alloc = msg->alloc;

    /* if the payload is set, release it. */
    if (NULL != msg->payload.data)
    {
        dispose((disposable_t*)&msg->payload);
    }

    /* reclaim the memory. */
    return
        rcpr_allocator_reclaim(alloc, msg);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
