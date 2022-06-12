/**
 * \file notificationservice/notificationservice_assertion_entry_add.c
 *
 * \brief Add an assertion entry to this context's assertion tree.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Add an assertion entry to this context's assertion tree.
 *
 * \param context       The context for this assertion tree.
 * \param offset        The offset for the assertion.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_assertion_entry_add(
    notificationservice_protocol_fiber_context* context, uint64_t offset)
{
    status retval, release_retval;
    notificationservice_assertion_entry* tmp = NULL;

    /* allocate memory for this entry. */
    retval =
        rcpr_allocator_allocate(context->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear this memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(&tmp->hdr, &notificationservice_assertion_entry_release);

    /* set values. */
    tmp->alloc = context->alloc;
    tmp->context = context;
    tmp->offset = offset;

    /* add this entry to the rbtree. */
    retval = rbtree_insert(context->inst->assertions, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* this entry is now owned by the rbtree. */
    tmp = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    if (NULL != tmp)
    {
        release_retval = resource_release(&tmp->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

done:
    return retval;
}
