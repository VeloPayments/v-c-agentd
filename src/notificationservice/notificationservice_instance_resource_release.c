/**
 * \file notificationservice/notificationservice_instance_resource_release.c
 *
 * \brief Release the notificationservice instance resource.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_slist;

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
    status reclaim_retval = STATUS_SUCCESS;
    status protosock_release_retval = STATUS_SUCCESS;
    status outbound_addr_release_retval = STATUS_SUCCESS;
    status assertions_release_retval = STATUS_SUCCESS;
    notificationservice_instance* inst = (notificationservice_instance*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = inst->alloc;

    /* if the protosock is set, release it. */
    if (NULL != inst->protosock)
    {
        protosock_release_retval =
            resource_release(psock_resource_handle(inst->protosock));
    }

    /* if the outbound address is set, close it. */
    if (0 != inst->outbound_addr)
    {
        outbound_addr_release_retval =
            mailbox_close(inst->outbound_addr, inst->ctx->msgdisc);
    }

    /* if the assertions list is set, release it. */
    if (NULL != inst->assertions)
    {
        assertions_release_retval =
            resource_release(slist_resource_handle(inst->assertions));
    }

    /* clear the structure. */
    memset(inst, 0, sizeof(*inst));

    /* reclaim memory. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, inst);

    /* decode return value. */
    if (STATUS_SUCCESS != protosock_release_retval)
    {
        return protosock_release_retval;
    }
    else if (STATUS_SUCCESS != outbound_addr_release_retval)
    {
        return outbound_addr_release_retval;
    }
    else if (STATUS_SUCCESS != assertions_release_retval)
    {
        return assertions_release_retval;
    }
    else
    {
        return reclaim_retval;
    }
}
