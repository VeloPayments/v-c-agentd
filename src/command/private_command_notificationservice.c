/**
 * \file command/private_command_notificationservice.c
 *
 * \brief Run the notification service instance.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/command.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run the notification service instance.
 */
void private_command_notificationservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* TODO - implement service here. */
    for (;;)
    {
        sleep(10);
    }

    exit(0);
}
