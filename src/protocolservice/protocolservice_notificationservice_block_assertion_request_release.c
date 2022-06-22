/**
 * \file
 * protocolservice/protocolservice_notificationservice_block_assertion_request_release.c
 *
 * \brief Release a block assertion request message resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Release the block assertion request resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_request_release(
    RCPR_SYM(resource)* r)
{
    protocolservice_notificationservice_block_assertion_request* req =
        (protocolservice_notificationservice_block_assertion_request*)r;

    /* sanity check. */
    MODEL_ASSERT(prop_notificationservice_block_assertion_request_valid(req));

    /* cache allocator. */
    rcpr_allocator* alloc = req->alloc;

    /* clear memory. */
    memset(req, 0, sizeof(*req));

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, req);
}
