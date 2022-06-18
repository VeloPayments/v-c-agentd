/**
 * \file test_mock_notificationservice.cpp
 *
 * Test the mock notificationservice.
 *
 * \copyright 2022 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <ostream>

#include "test_mock_notificationservice.h"
#include "../mocks/notificationservice.h"

using namespace std;

/**
 * Test that we can spawn the mock notificationservice.
 */
TEST_F(mock_notificationservice_test, basic_spawn)
{
    EXPECT_TRUE(test_suite_valid);
}
