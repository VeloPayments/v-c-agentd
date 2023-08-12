/**
 * \file test_mock_notificationservice_helpers.cpp
 *
 * Helpers for the mock notificationservice unit test.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>

/* GTEST DISABLED */
#if 0

#include "test_mock_notificationservice.h"

using namespace std;

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

void mock_notificationservice_test::SetUp()
{
    status retval;

    test_suite_valid = false;
    alloc = nullptr;
    sock = nullptr;

    /* create a socketpair for the mock and notify sockets. */
    int mocksock;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &mocksock, &notifysock);

    /* create the mock notificationservice. */
    mock =
        make_unique<mock_notificationservice::mock_notificationservice>(
            mocksock);

    /* create a malloc allocator. */
    retval = rcpr_malloc_allocator_create(&alloc);
    if (STATUS_SUCCESS != retval)
    {
        return;
    }

    /* wrap this socket in a psock wrapper. */
    retval = psock_create_from_descriptor(&sock, alloc, notifysock);
    if (STATUS_SUCCESS != retval)
    {
        return;
    }

    /* if we've made it this far, the test suite is valid. */
    test_suite_valid = true;
}

void mock_notificationservice_test::TearDown()
{
    status release_retval;

    if (nullptr != sock)
    {
        release_retval = resource_release(psock_resource_handle(sock));
        (void)release_retval;
    }

    if (nullptr != alloc)
    {
        release_retval =
            resource_release(rcpr_allocator_resource_handle(alloc));
        (void)release_retval;
    }
}
#endif
