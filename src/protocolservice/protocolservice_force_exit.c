/**
 * \file protocolservice/protocolservice_force_exit.c
 *
 * \brief Force an exit from the protocol service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <rcpr/fiber.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_fiber;

/**
 * \brief Force an exit from the protocol service.
 *
 * \param ctx           The protocol service context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_force_exit(protocolservice_context* ctx)
{
    return
        disciplined_fiber_scheduler_send_terminate_request_to_fiber(
            ctx->sched, ctx->main_fiber);
}
