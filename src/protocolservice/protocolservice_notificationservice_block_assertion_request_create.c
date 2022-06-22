/**
 * \file
 * protocolservice/protocolservice_notificationservice_block_assertion_request_create.c
 *
 * \brief Create a block assertion request message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a block assertion request message for the notificationservice
 * endpoint.
 *
 * \param payload       The pointer to receive this created message payload.
 * \param alloc         The allocator for this operation.
 * \param block_id      The block id for this operation.
 * \param return_addr   The return address to send the invalidation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_request_create(
    protocolservice_notificationservice_block_assertion_request** payload,
    RCPR_SYM(allocator)* alloc, const RCPR_SYM(rcpr_uuid)* block_id,
    RCPR_SYM(mailbox_address) return_addr)
{
    status retval;
    protocolservice_notificationservice_block_assertion_request* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(return_addr > 0);

    /* allocate memory for the payload. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear the payload memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize the payload resource. */
    resource_init(
        &tmp->hdr,
        &protocolservice_notificationservice_block_assertion_request_release);

    /* set the fields. */
    tmp->alloc = alloc;
    tmp->reply_addr = return_addr;
    memcpy(&tmp->block_id, block_id, sizeof(*block_id));

    /* success. */
    *payload = tmp;
    retval = STATUS_SUCCESS;

done:
    return retval;
}
