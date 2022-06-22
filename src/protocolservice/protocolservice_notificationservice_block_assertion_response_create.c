/**
 * \file
 * protocolservice/protocolservice_notificationservice_block_assertion_response_create.c
 *
 * \brief Create a block assertion response message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a notificationservice endpoint block assertion response message
 * payload.
 *
 * \param payload       Pointer to receive the payload on success.
 * \param alloc         The allocator to use for this operation.
 * \param offset        The offset value to send in the response.
 * \param success       Flag to indicate whether the request was successful.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_block_assertion_response_create(
    protocolservice_notificationservice_block_assertion_response** payload,
    RCPR_SYM(allocator)* alloc, uint64_t offset, bool success)
{
    status retval;
    protocolservice_notificationservice_block_assertion_response* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(offset > 0);

    /* allocate memory for the response. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr,
        &protocolservice_notificationservice_block_assertion_response_release);

    /* set values. */
    tmp->alloc = alloc;
    tmp->success = success;
    tmp->offset = offset;

    /* success. */
    *payload = tmp;
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
