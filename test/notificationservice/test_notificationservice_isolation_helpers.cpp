/**
 * \file test/notificationservice/test_notificationservice_isolation_helpers.cpp
 *
 * Helpers for the notificationservice isolation test.
 *
 * \copyright 2021-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>
#include <signal.h>
#include <sys/wait.h>

#include "test_notificationservice_isolation.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

void notificationservice_isolation_test::setUp()
{
    status retval;

    vccrypt_suite_register_velo_v1();

    /* initialize allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_suite_options_init(
            &suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        suite_instance_initialized = true;
    }
    else
    {
        suite_instance_initialized = false;
    }

    /* set the path for running agentd. */
    const char* agentd_path = getenv("AGENTD_PATH");
    if (NULL != agentd_path)
    {
        strcpy(wd, agentd_path);
        oldpath = getenv("PATH");
        if (NULL != oldpath)
        {
            path =
                strcatv(wd, ":", oldpath, NULL);
        }
        else
        {
            path = strcatv(wd, NULL);
        }
    }

    setenv("PATH", path, 1);

    /* log to standard error. */
    logsock = dup(STDERR_FILENO);
    rlogsock = dup(STDERR_FILENO);

    /* create the socket pair for client sock 1. */
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &client1sock, &rclient1sock);

    /* create the socket pair for client sock 2. */
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &client2sock, &rclient2sock);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* create the allocator. */
    retval = rcpr_malloc_allocator_create(&alloc);
    (void)retval;

    /* create the client 1 psock instance. */
    retval = psock_create_from_descriptor(&client1, alloc, client1sock);
    (void)retval;

    /* create the client 2 psock instance. */
    retval = psock_create_from_descriptor(&client2, alloc, client2sock);
    (void)retval;

    /* spawn the notificationservice process. */
    notify_proc_status =
        notificationservice_proc(
            &bconf, &conf, logsock, rclient1sock, rclient2sock, &notifypid,
            false);
}

void notificationservice_isolation_test::tearDown()
{
    /* terminate the notificationservice process. */
    if (0 == notify_proc_status)
    {
        int status = 0;
        kill(notifypid, SIGTERM);
        waitpid(notifypid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    close(logsock);
    if (rlogsock >= 0)
        close(rlogsock);
    close(rclient1sock);
    close(rclient2sock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    dispose((disposable_t*)&alloc_opts);

    resource_release(psock_resource_handle(client1));
    resource_release(psock_resource_handle(client2));
    resource_release(rcpr_allocator_resource_handle(alloc));
}
