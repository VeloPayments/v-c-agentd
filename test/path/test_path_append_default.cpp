/**
 * \file test_path_append_default.cpp
 *
 * Test the path_append_default method.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <agentd/command.h>
#include <agentd/path.h>
#include <cstring>
#include <minunit/minunit.h>
#include <paths.h>
#include <string>
#include <vpr/disposable.h>

using namespace std;

TEST_SUITE(path_append_default);

/**
 * \brief Append the default path onto an empty string.
 */
TEST(empty_string)
{
    char* outpath = nullptr;

    TEST_ASSERT(0 == path_append_default("", &outpath));

    TEST_EXPECT(!strcmp(_PATH_DEFPATH, outpath));

    free(outpath);
}

/**
 * \brief Append the default path onto an arbitrary path
 */
TEST(arbitrary_path_1)
{
    char* outpath = nullptr;
    string arbitrary_path = string("baz") + string(":") + _PATH_DEFPATH;

    TEST_ASSERT(0 == path_append_default("baz", &outpath));

    TEST_EXPECT(!strcmp(arbitrary_path.c_str(), outpath));

    free(outpath);
}

/**
 * \brief Append the default path onto an arbitrary path
 */
TEST(arbitrary_path_2)
{
    char* outpath = nullptr;
    string begin_path("/bin:/usr/bin:/home/foo/bin");
    string arbitrary_path =
        string("/bin:/usr/bin:/home/foo/bin") + string(":") + _PATH_DEFPATH;

    TEST_ASSERT(0 == path_append_default(begin_path.c_str(), &outpath));

    TEST_EXPECT(!strcmp(arbitrary_path.c_str(), outpath));

    free(outpath);
}
