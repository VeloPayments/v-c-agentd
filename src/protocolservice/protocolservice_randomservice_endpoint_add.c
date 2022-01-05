/**
 * \file protocolservice/protocolservice_randomservice_endpoint_add.c
 *
 * \brief Add the random service endpoint fiber.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Create and add the protocol service random endpoint fiber.
 *
 * \param addr          Pointer to receive the mailbox address for this
 *                      endpoint on success.
 * \param alloc         The allocator to use to create this fiber.
 * \param sched         The fiber scheduler to which this endpoint fiber should
 *                      be assigned.
 * \param randomsock    The socket connection to the random service.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_randomservice_endpoint_add(
    RCPR_SYM(mailbox_address)* addr, RCPR_SYM(allocator)* alloc,
    RCPR_SYM(fiber_scheduler)* sched, int randomsock)
{
    /* TODO - replace this stub. */

    (void)addr;
    (void)alloc;
    (void)sched;
    (void)randomsock;

    return STATUS_SUCCESS;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */