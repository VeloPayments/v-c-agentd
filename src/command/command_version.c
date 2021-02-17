/**
 * \file command/command_version.c
 *
 * \brief Print version information to standard output.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <config.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Print version information to standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_version(struct bootstrap_config* UNUSED(bconf))
{
    MODEL_ASSERT(NULL != bconf);

    printf("agentd version: %s\n", AGENTD_VERSION);

    return AGENTD_STATUS_SUCCESS;
}
