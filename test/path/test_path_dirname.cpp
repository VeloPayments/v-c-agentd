/**
 * \file test_path_dirname.cpp
 *
 * Test the path_dirname method.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <agentd/command.h>
#include <agentd/path.h>
#include <cstring>
#include <limits.h>
#include <minunit/minunit.h>
#include <string>
#include <vpr/disposable.h>

using namespace std;

TEST_SUITE(path_dirname);

/**
 * When an empty string is encountered, return "." to represent the current
 * directory.
 */
TEST(empty_string)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname("", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp(".", dir));

    free(dir);
}

/**
 * When a null string is encountered, return "." to represent the current
 * directory.
 */
TEST(null_path)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname(NULL, &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp(".", dir));

    free(dir);
}

/**
 * When a simple filename is encountered, the directory is the current
 * directory.
 */
TEST(simple_filename)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname("foo.txt", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp(".", dir));

    free(dir);
}

/**
 * A filename with a single subdirectory is shortened to the subdir.
 */
TEST(single_subdir)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname("build/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp("build", dir));

    free(dir);
}

/**
 * A filename with multiple subdirs is properly extracted.
 */
TEST(multi_subdir)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname("build/host/checked/src/path/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp("build/host/checked/src/path", dir));

    free(dir);
}

/**
 * An absolute directory is properly extracted.
 */
TEST(multi_subdir_absolute)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(
        0 == path_dirname("/build/host/checked/src/path/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp("/build/host/checked/src/path", dir));

    free(dir);
}

/**
 * A filename relative to the current directory is properly handled.
 */
TEST(explicit_curdir)
{
    char* dir;

    /* path_dirname should succeed. */
    TEST_ASSERT(0 == path_dirname("./foo.txt", &dir));

    /* the dirname string matches what we expect. */
    TEST_EXPECT(!strcmp(".", dir));

    free(dir);
}
