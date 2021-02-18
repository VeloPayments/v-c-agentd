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
#include <vcblockchain/version.h>
#include <vccert/version.h>
#include <vccrypt/version.h>
#include <vpr/parameters.h>
#include <vpr/version.h>

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

    printf("%-20s %20s\n", "agentd:", AGENTD_VERSION);
    printf("%-20s %20s\n", "v-portable-runtime:", vpr_version());
    printf("%-20s %20s\n", "v-c-crypto:", vccrypt_version());
    printf("%-20s %20s\n", "v-c-certificate:", vccert_version());
    printf("%-20s %20s\n", "v-c-bc:", vcblockchain_version());

    return AGENTD_STATUS_SUCCESS;
}
