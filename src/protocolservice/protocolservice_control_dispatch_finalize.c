/**
 * \file protocolservice/protocolservice_control_dispatch_finalize.c
 *
 * \brief Dispatch a finalize control command.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Dispatch a finalize request
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_finalize(
    protocolservice_control_fiber_context* ctx, const void* /*payload*/,
    size_t /*size*/)
{
    /* instruct the control fiber to exit. */
    ctx->should_exit = true;

    /* write a successful response to the control socket. */
    return
        protocolservice_control_write_response(
            ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_FINALIZE, STATUS_SUCCESS);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
