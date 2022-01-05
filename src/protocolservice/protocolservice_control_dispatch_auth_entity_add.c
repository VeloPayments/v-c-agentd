/**
 * \file protocolservice/protocolservice_control_dispatch_auth_entity_add.c
 *
 * \brief Dispatch an auth entity add control command.
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
 * \brief Dispatch an auth entity add control request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_auth_entity_add(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size)
{
    /* TODO - fill out. */
    (void)payload;
    (void)size;
    return 
        protocolservice_control_write_response(
            ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
