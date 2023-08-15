/**
 * \file test/notificationservice/test_notificationservice_isolation.h
 *
 * Private header for the notificationservice isolation tests.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#pragma once

#include <agentd/config.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/string.h>
#include <functional>
#include <string>
#include <vector>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

extern "C" {
#include "agentd.tab.h"
#include "agentd.yy.h"
}

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

/**
 * The notificationservice isolation test class deals with the drudgery of
 * communicating with the notificationservice.  It provides a registration
 * mechanism so that data can be sent to the service and received from the
 * service.
 */
class notificationservice_isolation_test
{
public:
    void setUp();
    void tearDown();

    bootstrap_config_t bconf;
    agent_config_t conf;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    int client1sock;
    int rclient1sock;
    int client2sock;
    int rclient2sock;
    int logsock;
    int rlogsock;
    pid_t notifypid;
    int notify_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;

    bool suite_instance_initialized;
    RCPR_SYM(psock)* client1;
    RCPR_SYM(psock)* client2;
    RCPR_SYM(allocator)* alloc;
};
