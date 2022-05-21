/**
 * \file protocolservice/protocolservice_random_response_message_release.c
 *
 * \brief Release a random service endpoint response message payload.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release a protocol service random response payload resource.
 *
 * \param r             The payload resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_response_message_release(RCPR_SYM(resource)* r)
{
    status payload_release_retval = STATUS_SUCCESS;
    status data_release_retval = STATUS_SUCCESS;

    protocolservice_random_response_message* payload =
        (protocolservice_random_response_message*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_random_response_message_valid(payload));

    /* cache allocator. */
    rcpr_allocator* alloc = payload->alloc;

    /* reclaim the data memory if set. */
    if (NULL != payload->data)
    {
        memset(payload->data, 0, payload->size);
        data_release_retval = rcpr_allocator_reclaim(alloc, payload->data);
    }

    /* reclaim the payload memory. */
    payload_release_retval = rcpr_allocator_reclaim(alloc, payload);

    /* decode the response code to send to the caller. */
    if (STATUS_SUCCESS != data_release_retval)
    {
        return data_release_retval;
    }
    else
    {
        return payload_release_retval;
    }
}
