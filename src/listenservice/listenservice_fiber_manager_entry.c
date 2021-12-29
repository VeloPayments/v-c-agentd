/**
 * \file listenservice/listenservice_fiber_manager_entry.c
 *
 * \brief Entry point for the fiber manager.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <rcpr/uuid.h>

#include "listenservice_internal.h"

RCPR_IMPORT_fiber;
RCPR_IMPORT_uuid;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the listen service fiber manager fiber.
 *
 * This fiber manages cleanup for fibers as they stop.
 *
 * \param vsched        The type erased scheduler.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_fiber_manager_entry(void* vsched)
{
    status retval;
    const rcpr_uuid* resume_id;
    int resume_event;
    void* resume_param;
    fiber* stopped_fiber;

    fiber_scheduler* sched = (fiber_scheduler*)vsched;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_fiber_scheduler_valid(sched));

    for (;;)
    {
        /* receive a management event. */
        retval =
            disciplined_fiber_scheduler_receive_management_event(
                sched, &resume_id, &resume_event, &resume_param);
        if (STATUS_SUCCESS != retval)
        {
            return retval;
        }

        /* decode the management event. */
        if (
            !memcmp(
                resume_id, &FIBER_SCHEDULER_MANAGEMENT_DISCIPLINE,
                sizeof(rcpr_uuid)))
        {
            /* decode the event code. */
            switch (resume_event)
            {
                /* a fiber has been stopped.  Clean it up. */
                case FIBER_SCHEDULER_MANAGEMENT_RESUME_EVENT_FIBER_STOP:
                    stopped_fiber = (fiber*)resume_param;

                    /* instruct the fiber scheduler to remove the fiber ref. */
                    retval =
                        disciplined_fiber_scheduler_remove_fiber(
                            sched, stopped_fiber);
                    if (STATUS_SUCCESS != retval)
                    {
                        /* TODO - log an error and exit. */
                        continue;
                    }
                    else
                    {
                        /* release the fiber. */
                        retval =
                            resource_release(
                                fiber_resource_handle(stopped_fiber));
                        if (STATUS_SUCCESS != retval)
                        {
                            /* TODO - log an error and exit. */
                            continue;
                        }
                    }
                    break;

                /* a quiesce request. */
                case FIBER_SCHEDULER_MANAGEMENT_RESUME_EVENT_QUIESCE_REQUEST:
                    break;

                /* a termination request. */
                case FIBER_SCHEDULER_MANAGEMENT_RESUME_EVENT_TERMINATION_REQUEST:
                    break;

                /* ignore any other management event. */
                default:
                    break;
            }
        }
        /* ignore any other event. */
        else
        {
            continue;
        }
    }
}
