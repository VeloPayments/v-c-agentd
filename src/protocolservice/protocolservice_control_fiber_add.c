/**
 * \file protocolservice/protocolservice_control_fiber_add.c
 *
 * \brief Add the control fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Create and add the protocol service control fiber.
 *
 * \param alloc         The allocator to use to create this fiber.
 * \param ctx           The protocol service context.
 * \param controlsock   The socket connection to the supervisor.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_fiber_add(
    RCPR_SYM(allocator)* alloc, protocolservice_context* ctx, int controlsock)
{
    /* TODO - replace stub. */
    (void)alloc;
    (void)ctx;
    (void)controlsock;

    return STATUS_SUCCESS;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
