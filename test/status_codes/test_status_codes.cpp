/**
 * \file test_status_codes.cpp
 *
 * Test that status codes work as expected.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <minunit/minunit.h>

using namespace std;

TEST_SUITE(status_codes_test);

/**
 * Test that status codes are constructed properly.
 */
TEST(basic_test)
{
    TEST_EXPECT(0x8000001 == AGENTD_ERROR_GENERAL_OUT_OF_MEMORY);
}
