/**
 * \file listenservice/listenservice_listen_fiber_unexpected_handler.c
 *
 * \brief Unexpected resume event handler for the listen fiber.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <rcpr/uuid.h>
#include <unistd.h>

#include "listenservice_internal.h"

/**
 * \brief Handle unexpected resume events in the listen fiber.
 *
 * \param context                   Opaque reference to listenservice listen
 *                                  fiber context.
 * \param fib                       The fiber experiencing this event.
 * \param resume_disc_id            The unexpected resume discipline id.
 * \param resume_event              The unexpected resume event.
 * \param resume_param              The unexpected resume parameter.
 * \param expected_resume_disc_id   The expected discipline id.
 * \param expected_resume_event     The expected resume event.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS if the fiber should retry the yield.
 *      - a non-zero error code if the fiber should exit.
 */
status listenservice_listen_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* /*fib*/,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* /*resume_param*/,
    const RCPR_SYM(rcpr_uuid)* /*expected_resume_disc_id*/,
    int /*expected_resume_event*/)
{
    listenservice_listen_fiber_context* ctx =
        (listenservice_listen_fiber_context*)context;

    /* is this a management discipline event? */
    if (!memcmp(resume_disc_id, &FIBER_SCHEDULER_MANAGEMENT_DISCIPLINE, 16))
    {
        /* retry on quiesce. */
        if (
            FIBER_SCHEDULER_MANAGEMENT_RESUME_EVENT_QUIESCE_REQUEST
                == resume_event)
        {
            ctx->quiesce = true;
            return STATUS_SUCCESS;
        }
    }

    /* for any other resume event, terminate the listen fiber. */
    return ERROR_FIBER_INVALID_STATE;
}
