/**
 * \file test_path_resolve.cpp
 *
 * Test the path_resolve method.
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
#include <unistd.h>
#include <vpr/disposable.h>

using namespace std;

TEST_SUITE(path_resolve);

/**
 * \brief It is not possible to resolve a non-existent binary from an empty
 * path.
 */
TEST(empty_path_no_local)
{
    char* resolved = nullptr;

    TEST_ASSERT(0 != path_resolve("foosh", "", &resolved));

    TEST_EXPECT(nullptr == resolved);
}

/**
 * \brief It is possible to resolve a binary from a simple path.
 */
TEST(simple_path)
{
    char* resolved = nullptr;
    const char* CATLOC = getenv("TEST_BIN");

    TEST_ASSERT(0 == path_resolve("cat", "/bin", &resolved));

    TEST_EXPECT(!strcmp(CATLOC, resolved));

    free(resolved);
}

/**
 * \brief A non-existent binary and a simple path do not resolve.
 */
TEST(simple_path_non_existent_binary)
{
    char* resolved = nullptr;

    TEST_ASSERT(0 != path_resolve("foosh", "/bin", &resolved));

    TEST_EXPECT(nullptr == resolved);
}

/**
 * \brief It is possible to resolve a binary from a multi path.
 */
TEST(multi_path)
{
    char* resolved = nullptr;
    const char* CATLOC = getenv("TEST_BIN");

    TEST_ASSERT(
        0
            == path_resolve(
                    "cat", "/etasuetheoasu:/teasuthoseu:/bin", &resolved));

    TEST_EXPECT(!strcmp(CATLOC, resolved));

    free(resolved);
}

/**
 * \brief A non-existent binary and a multi path do not resolve.
 */
TEST(multi_path_non_existent_binary)
{
    char* resolved = nullptr;

    TEST_ASSERT(
        0
            != path_resolve(
                    "foosh", "/etasuetheoasu:/teasuthoseu:/bin", &resolved));

    TEST_EXPECT(nullptr == resolved);
}

/**
 * \brief If a binary is an absolute path but it does not exist, then
 * path_resolve fails.
 */
TEST(nonexistent_absolute_path)
{
    char* resolved = nullptr;

    TEST_ASSERT(0 != path_resolve("/bin/fooshsthsthsth", "", &resolved));

    TEST_EXPECT(nullptr == resolved);
}

/**
 * \brief If a binary is an absolute path and it exists, then resolved is
 * updated to the canonical path for this value and path_resolve succeeds.
 */
TEST(canonical_absolute_path)
{
    char* resolved = nullptr;
    const char* CATLOC = getenv("TEST_BIN");

    TEST_ASSERT(0 == path_resolve("/bin//cat", "", &resolved));

    TEST_EXPECT(!strcmp(CATLOC, resolved));

    free(resolved);
}

/**
 * \brief If a relative path starting with "." is encountered, attempt to
 * canonicalize it.  If it cannot be resolved, fail.
 */
TEST(canonical_relative_path_fail)
{
    char* resolved = nullptr;

    TEST_ASSERT(0 != path_resolve("./bin//cat", "", &resolved));

    TEST_EXPECT(nullptr == resolved);
}

/**
 * \brief If a relative path starting with "." is encountered, attempt to
 * canonicalize it.  If it can be resolved and is executable, succeed.
 */
TEST(canonical_relative_path)
{
    char buf[PATH_MAX];
    char* resolved = nullptr;
    char* pwd = getcwd(buf, sizeof(buf));
    char* agentd_path = getenv("AGENTD_PATH");
    string expected_resolved = string(agentd_path) + "/agentd";

    TEST_ASSERT(0 == chdir(agentd_path));
    TEST_ASSERT(0 == path_resolve("./agentd", "", &resolved));
    TEST_ASSERT(0 == chdir(pwd));

    TEST_EXPECT(!strcmp(expected_resolved.c_str(), resolved));

    free(resolved);
}

/**
 * \brief If a relative path NOT starting with "." is encountered, attempt to
 * canonicalize it.  If it can be resolved and is executable, succeed.
 */
TEST(canonical_relative_path2)
{
    char buf[PATH_MAX];
    char* resolved = nullptr;
    char* pwd = getcwd(buf, sizeof(buf));
    char* agentd_path = getenv("AGENTD_PATH");
    string expected_resolved = string(agentd_path) + "/agentd";

    TEST_ASSERT(0 == chdir(agentd_path));
    TEST_ASSERT(0 == path_resolve("agentd", "", &resolved));
    TEST_ASSERT(0 == chdir(pwd));

    TEST_EXPECT(!strcmp(expected_resolved.c_str(), resolved));

    free(resolved);
}
