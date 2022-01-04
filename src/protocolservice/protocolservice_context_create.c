/**
 * \file protocolservice/protocolservice_context_create.c
 *
 * \brief Create the protocol service context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Create the protocol service context.
 *
 * \param ctx           Pointer to receive the protocol service context pointer
 *                      on success.
 * \param alloc         The allocator to use to create this context.
 * \param sched         The fiber scheduler.
 * \param random_addr   The mailbox address of the random service endpoint.
 * \param data_addr     The mailbox address of the data service endpoint.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_context_create(
    protocolservice_context** ctx, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, RCPR_SYM(mailbox_address) random_addr,
    RCPR_SYM(mailbox_address) data_addr)
{
    status retval, release_retval;
    protocolservice_context* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(prop_allocator_valid(alloc));
    MODEL_ASSERT(random_addr >= 0);
    MODEL_ASSERT(data_addr >= 0);

    /* allocate memory for the context. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear context memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(&tmp->hdr, &protocolservice_context_release);

    /* set init values. */
    tmp->alloc = alloc;
    tmp->sched = sched;
    tmp->data_endpoint_addr = data_addr;
    tmp->random_endpoint_addr = random_addr;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&tmp->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* success. */
    *ctx = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_context:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
