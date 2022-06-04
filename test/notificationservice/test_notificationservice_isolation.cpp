/**
 * \file test/notificationservice/test_notificationservice_isolation.cpp
 *
 * Isolation tests for the notificationservice.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_notificationservice_isolation.h"

using namespace std;

/**
 * Test that we can spawn the notificationservice.
 */
TEST_F(notificationservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, notify_proc_status);
}
