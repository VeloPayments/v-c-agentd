/**
 * \file test_bootstrap_config.cpp
 *
 * Test the boostrap configuration functions.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cstdlib>
#include <cstring>
#include <minunit/minunit.h>
#include <vpr/disposable.h>

using namespace std;

TEST_SUITE(bootstrap_config_test);

/**
 * \brief Initializing the bootstrap config structure empties all values.
 */
TEST(bootstrap_config_init)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* by default, agentd runs as a daemon. */
    TEST_EXPECT(!bconf.foreground);
    /* by default, command is NULL. */
    TEST_EXPECT(nullptr == bconf.command);
    /* by default, private_command is NULL. */
    TEST_EXPECT(nullptr == bconf.private_command);
    /* by default, the config file is set to /etc/agentd.conf. */
    TEST_EXPECT(!strcmp("/etc/agentd.conf", bconf.config_file));
    /* by default, config_file_override is false. */
    TEST_EXPECT(!bconf.config_file_override);
    /* by default, init mode is set to false. */
    TEST_EXPECT(!bconf.init_mode);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_foreground sets the foreground field.
 */
TEST(bootstrap_config_set_foreground)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: foreground is false. */
    TEST_ASSERT(!bconf.foreground);

    /* run bootstrap_config_set_foreground. */
    bootstrap_config_set_foreground(&bconf, true);

    /* Postcondition: foreground is true. */
    TEST_EXPECT(bconf.foreground);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_init_mode sets the init_mode field.
 */
TEST(bootstrap_config_set_init_mode)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: foreground is false. */
    TEST_ASSERT(!bconf.init_mode);

    /* run bootstrap_config_set_init_mode. */
    bootstrap_config_set_init_mode(&bconf, true);

    /* Postcondition: foreground is true. */
    TEST_EXPECT(bconf.init_mode);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_command sets the command field.
 */
TEST(bootstrap_config_set_command)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: command is NULL. */
    TEST_ASSERT(nullptr == bconf.command);

    /* run bootstrap_config_set_command. */
    bootstrap_config_set_command(&bconf, (bootstrap_config_command_t)0x1234);

    /* Postcondition: command is set. */
    TEST_EXPECT((bootstrap_config_command_t)0x1234 == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_private_command sets the private_command field.
 */
TEST(bootstrap_config_set_private_command)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: private_command is NULL. */
    TEST_ASSERT(nullptr == bconf.private_command);

    /* run bootstrap_config_set_private_command. */
    bootstrap_config_set_private_command(
        &bconf, (bootstrap_config_private_command_t)0x1234);

    /* Postcondition: private_command is set. */
    TEST_EXPECT(
        (bootstrap_config_private_command_t)0x1234 == bconf.private_command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_config_file sets the config file.
 */
TEST(bootstrap_config_set_config_file)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: config file is set to the default name. */
    TEST_ASSERT(!strcmp("/etc/agentd.conf", bconf.config_file));
    /* Precondition: config file override is false. */
    TEST_ASSERT(!bconf.config_file_override);

    /* Change the config file location. */
    bootstrap_config_set_config_file(&bconf, "etc/awesome_agentd.conf");

    /* Postcondition: config file is updated. */
    TEST_EXPECT(!strcmp("etc/awesome_agentd.conf", bconf.config_file));
    /* Postcondition: config file override is true. */
    TEST_EXPECT(bconf.config_file_override);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_binary sets the absolute location of the binary
 * provided.
 */
TEST(bootstrap_config_set_binary)
{
    bootstrap_config_t bconf;
    const char* CATLOC = getenv("TEST_BIN");

    bootstrap_config_init(&bconf);

    /* Precondition: binary name is null by default. */
    TEST_ASSERT(NULL == bconf.binary);

    /* Set the binary name. */
    TEST_ASSERT(0 == bootstrap_config_set_binary(&bconf, "cat"));

    /* Postcondition: binary name is correct. */
    TEST_EXPECT(!strcmp(CATLOC, bconf.binary));

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_binary fails if the binary can't be found.
 */
TEST(bootstrap_config_set_binary_bad_binary)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: binary name is null by default. */
    TEST_ASSERT(NULL == bconf.binary);

    /* Set the binary name. */
    TEST_ASSERT(0 != bootstrap_config_set_binary(&bconf, "esathualceuhalrou"));

    dispose((disposable_t*)&bconf);
}
