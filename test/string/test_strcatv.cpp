/**
 * \file test_strcatv.cpp
 *
 * Test variable strcat.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/string.h>
#include <cstdlib>
#include <cstring>
#include <minunit/minunit.h>

using namespace std;

TEST_SUITE(strcatv);

/**
 * \brief Passing NULL as the first argument of strcatv returns an empty string.
 */
TEST(param_null)
{
    char* str = strcatv(nullptr);
    TEST_ASSERT(nullptr != str);
    TEST_EXPECT(!strcmp("", str));
    free(str);
}

/**
 * \brief We can concatenate one string and null.
 */
TEST(one_param)
{
    char* str = strcatv("foo", nullptr);
    TEST_ASSERT(nullptr != str);
    TEST_EXPECT(!strcmp("foo", str));
    free(str);
}

/**
 * \brief We can concatenate two strings and null.
 */
TEST(two_params)
{
    char* str = strcatv("foo", "bar", nullptr);
    TEST_ASSERT(nullptr != str);
    TEST_EXPECT(!strcmp("foobar", str));
    free(str);
}

/**
 * \brief We can concatenate many strings.
 */
TEST(many_params)
{
    char* str = strcatv("f", "o", "o", "", "b", "a", "r", "!", nullptr);
    TEST_ASSERT(nullptr != str);
    TEST_EXPECT(!strcmp("foobar!", str));
    free(str);
}
