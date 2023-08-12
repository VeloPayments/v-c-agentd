/**
 * \file test_random_service_isolation_helpers.cpp
 *
 * Helpers for the random service isolation test.
 *
 * \copyright 2020-2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <signal.h>
#include <sys/wait.h>
#include <vpr/allocator/malloc_allocator.h>

/* GTEST DISABLED */
#if 0

#include "test_random_service_isolation.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

void random_service_isolation_test::SetUp()
{
    /* log to standard error. */
    rlogsock = dup(STDERR_FILENO);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

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

    /* by default, we run in blocking mode. */
    nonblockrandomsock_configured = false;

    /* spawn the random service process. */
    random_proc_status =
        randomservice_proc(
            &bconf, &conf, &rlogsock, &rprotosock, &randompid, false);

    /* create the rcpr allocator. */
    ralloc_status = rcpr_malloc_allocator_create(&ralloc);

    /* create the psock instance. */
    if (STATUS_SUCCESS == ralloc_status && STATUS_SUCCESS == random_proc_status)
    {
        proto_status = psock_create_from_descriptor(&proto, ralloc, rprotosock);
    }
    else
    {
        proto_status = -1;
    }
}

void random_service_isolation_test::TearDown()
{
    /* if the random socket is in non-block mode, clean it up. */
    if (nonblockrandomsock_configured)
    {
        dispose((disposable_t*)&nonblockrandomsock);
        dispose((disposable_t*)&loop);
        nonblockrandomsock_configured = false;
    }

    /* close the psock. */
    if (STATUS_SUCCESS == proto_status)
    {
        resource_release(psock_resource_handle(proto));
    }
    else
    {
        close(rprotosock);
    }

    /* release the allocator. */
    if (STATUS_SUCCESS == ralloc_status)
    {
        resource_release(rcpr_allocator_resource_handle(ralloc));
    }

    /* terminate the random service. */
    if (0 == random_proc_status)
    {
        int status = 0;
        kill(randompid, SIGTERM);
        waitpid(randompid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    if (rlogsock >= 0)
        close(rlogsock);
}

void random_service_isolation_test::nonblockmode(
    function<void()> onRead, function<void()> onWrite)
{
    /* set the read / write callbacks. */
    this->onRead = onRead;
    this->onWrite = onWrite;

    /* create the event loop if necessary. */
    if (!nonblockrandomsock_configured)
    {
        ipc_make_noblock(rprotosock, &nonblockrandomsock, this);
        nonblockrandomsock_configured = true;
        ipc_event_loop_init(&loop);
    }
    else
    {
        ipc_event_loop_remove(&loop, &nonblockrandomsock);
    }

    ipc_set_readcb_noblock(&nonblockrandomsock, &nonblock_read, nullptr);
    ipc_set_writecb_noblock(&nonblockrandomsock, &nonblock_write, nullptr);
    ipc_event_loop_add(&loop, &nonblockrandomsock);
    ipc_event_loop_run(&loop);
}

void random_service_isolation_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    random_service_isolation_test* that = (random_service_isolation_test*)ctx;

    that->onRead();
}

void random_service_isolation_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    random_service_isolation_test* that = (random_service_isolation_test*)ctx;

    that->onWrite();
}
#endif
