/**
 * \file
 * protocolservice/protocolservice_notificationservice_block_assertion_response_release.c
 *
 * \brief Release a response resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release a notificationservice endpoint block assertion response
 * message payload resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_response_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_notificationservice_block_assertion_response* payload =
        (protocolservice_notificationservice_block_assertion_response*)r;

    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_block_assertion_response_valid(
            payload));

    /* cache allocator. */
    rcpr_allocator* alloc = payload->alloc;

    /* clear memory. */
    memset(payload, 0, sizeof(*payload));

    /* reclaim memory. */
    return rcpr_allocator_reclaim(alloc, payload);
}
