/**
 * \file command/private_command_protocolservice.c
 *
 * \brief Run an unauthorized protocol service instance.
 *
 * \copyright 2019-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/command.h>
#include <agentd/protocolservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run an unauthorized protocol service instance.
 */
void private_command_protocolservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

#if !defined(AGENTD_NEW_PROTOCOL)
    /* run the event loop for the protocol service. */
    int retval =
        old_unauthorized_protocol_service_event_loop(
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_RANDOM,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_ACCEPT,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_CONTROL,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_DATA,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG);
#else /* defined(AGENTD_NEW_PROTOCOL) */
    /* run the protocol service. */
    int retval =
        protocolservice_run(
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_RANDOM,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_ACCEPT,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_CONTROL,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_DATA,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG);
#endif /* defined(AGENTD_NEW_PROTOCOL) */

    /* exit with the return code from the event loop. */
    exit(retval);
}
