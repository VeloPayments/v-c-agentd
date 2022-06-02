/**
 * \file notificationservice/notificationservice_instance_resource_release.c
 *
 * \brief Release the notificationservice instance resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Release a notificationservice instance resource.
 *
 * \param r         The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_instance_resource_release(RCPR_SYM(resource)* r)
{
    notificationservice_instance* inst = (notificationservice_instance*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = inst->alloc;

    /* clear the structure. */
    memset(inst, 0, sizeof(*inst));

    /* reclaim memory. */
    return
        rcpr_allocator_reclaim(alloc, inst);
}
