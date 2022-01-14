/**
 * \file protocolservice/protocolservice_random_request_message_create.c
 *
 * \brief Create a random service endpoint request message payload.
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
 * \brief Create a request message payload for the random service endpoint.
 *
 * \param payload       Pointer to hold the created payload structure on
 *                      success. This resource is owned by the caller.
 * \param alloc         The allocator to use to create this payload.
 * \param size          The number of bytes requested.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_random_request_message_create(
    protocolservice_random_request_message** payload,
    RCPR_SYM(allocator)* alloc, size_t size)
{
    status retval;
    protocolservice_random_request_message* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(size > 0);

    /* allocate memory for the payload. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear payload memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize payload resource. */
    resource_init(&tmp->hdr, &protocolservice_random_request_message_release);

    /* set the fields. */
    tmp->alloc = alloc;
    tmp->size = size;

    /* success. */
    *payload = tmp;
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
