/**
 * \file command/private_command_attestationservice.c
 *
 * \brief Run an attestation service instance.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/attestationservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run an attestation service instance.
 */
void private_command_attestationservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the entry point for the canonization service. */
    int retval =
        attestationservice_entry_point(
            AGENTD_FD_ATTESTATION_SVC_DATA,
            AGENTD_FD_ATTESTATION_SVC_LOG,
            AGENTD_FD_ATTESTATION_SVC_CONTROL);

    /* exit with the return code from the event loop. */
    exit(retval);
}
