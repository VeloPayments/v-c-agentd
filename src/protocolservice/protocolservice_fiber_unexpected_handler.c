/**
 * \file protocolservice/protocolservice_fiber_unexpected_handler.c
 *
 * \brief Manage unexpected events for a given protocol service fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Handle unexpected resume events in fibers relating to the protocol
 * service.
 *
 * \param context                   Opaque reference to protocol service
 *                                  context.
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
status protocolservice_fiber_unexpected_handler(
    void* context, RCPR_SYM(fiber)* /*fib*/,
    const RCPR_SYM(rcpr_uuid)* resume_disc_id, int resume_event,
    void* /*resume_param*/,
    const RCPR_SYM(rcpr_uuid)* /*expected_resume_disc_id*/,
    int /*expected_resume_event*/)
{
    protocolservice_context* ctx = (protocolservice_context*)context;

    /* is this a management discipline event? */
    if (!memcmp(resume_disc_id, &FIBER_SCHEDULER_MANAGEMENT_DISCIPLINE, 16))
    {
        /* retry on quiesce. */
        if (
            FIBER_SCHEDULER_MANAGEMENT_RESUME_EVENT_QUIESCE_REQUEST
                == resume_event)
        {
            /* set the quiesce flag if context is set. */
            if (NULL != ctx)
            {
                ctx->quiesce = true;
            }

            return STATUS_SUCCESS;
        }
    }

    /* for any other resume event, terminate the fiber. */
    return ERROR_FIBER_INVALID_STATE;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
