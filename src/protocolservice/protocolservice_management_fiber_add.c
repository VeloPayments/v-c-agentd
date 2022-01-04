/**
 * \file protocolservice/protocolservice_management_fiber_add.c
 *
 * \brief Create and add the management fiber to the fiber scheduler.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_fiber;
RCPR_IMPORT_resource;

/**
 * \brief Create and add the protocol service management fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The scheduler to which this management fiber should be
 *                      assigned.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_management_fiber_add(
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber_scheduler)* sched)
{
    status retval, release_retval;
    fiber* manager;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_allocator_valid(alloc));
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));

    /* create the management fiber. */
    retval =
        fiber_create(
            &manager, alloc, sched, MANAGER_FIBER_STACK_SIZE, sched,
            &protocolservice_fiber_manager_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* add the management fiber to the scheduler. */
    retval = fiber_scheduler_add(sched, manager);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_manager;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_manager:
    release_retval = resource_release(fiber_resource_handle(manager));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
