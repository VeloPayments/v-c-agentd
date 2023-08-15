/**
 * \file test_parse_commandline_options.cpp
 *
 * Test parsing command-line options.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <agentd/command.h>
#include <cstring>
#include <minunit/minunit.h>
#include <vpr/disposable.h>

using namespace std;

TEST_SUITE(parse_commandline_options_test);

/**
 * \brief Parsing an empty set of command-line options should result in a
 * default bootstrap config.
 */
TEST(empty_arguments)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* empty_args[] = { exename, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(empty_args) / sizeof(char*), empty_args);

    /* by default, agentd runs as a daemon. */
    TEST_EXPECT(!bconf.foreground);
    /* the help command is set. */
    TEST_EXPECT(&command_help == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -F option should set foreground to true.
 */
TEST(foreground_option)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'F', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has been set to run in the foreground. */
    TEST_EXPECT(bconf.foreground);
    /* the help command is set. */
    TEST_EXPECT(&command_help == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -I option should set init_mode to true.
 */
TEST(init_mode_option)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'I', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* init_mode is false by default. */
    TEST_ASSERT(!bconf.init_mode);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has been set to run in init mode. */
    TEST_EXPECT(bconf.init_mode);
    /* the help command is set. */
    TEST_EXPECT(&command_help == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -c config should set the config file name.
 */
TEST(config_option_space)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'c', 0 };
    char config[] = { 'o', 't', 'h', 'e', 'r', '.', 'c', 'o', 'n', 'f', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, config, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has its config file overridden. */
    TEST_EXPECT(!strcmp("other.conf", bconf.config_file));
    /* the help command is set. */
    TEST_EXPECT(&command_help == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -c config should set the config file name (no space).
 */
TEST(config_option_no_space)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'c', 'o', 't', 'h', 'e', 'r', '.', 'c', 'o', 'n',
        'f', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has its config file overridden. */
    TEST_EXPECT(!strcmp("other.conf", bconf.config_file));
    /* the help command is set. */
    TEST_EXPECT(&command_help == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -v should request version information.
 */
TEST(version_request)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'v', 0 };
    char* args[] = { exename, flags };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has the version request set. */
    TEST_EXPECT(bconf.version_request);
    /* the version command is set. */
    TEST_EXPECT(&command_version == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing an invalid options raises an error and prints usage.
 */
TEST(invalid_option)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'x', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    TEST_EXPECT(&command_error_usage == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing an invalid command returns an error.
 */
TEST(invalid_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char badcmd[] = { 'f', 'o', 'o', 0 };
    char* args[] = { exename, badcmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    TEST_EXPECT(&command_error_usage == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief A command is required.
 */
TEST(no_command_fails)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char* args[] = { exename };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    TEST_EXPECT(&command_error_usage == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief The readconfig command is a valid command.
 */
TEST(readconfig_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char cmd[] = { 'r', 'e', 'a', 'd', 'c', 'o', 'n', 'f', 'i', 'g', 0 };
    char* args[] = { exename, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    TEST_ASSERT(nullptr == bconf.command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to command_readconfig. */
    TEST_ASSERT(&command_readconfig == bconf.command);

    dispose((disposable_t*)&bconf);
}


/**
 * \brief The readconfig private command is a valid private command.
 */
TEST(readconfig_private_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'P', 0 };
    char cmd[] = { 'r', 'e', 'a', 'd', 'c', 'o', 'n', 'f', 'i', 'g', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    TEST_ASSERT(nullptr == bconf.command);
    /* precondition: private_command should be NULL. */
    TEST_ASSERT(nullptr == bconf.private_command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to NULL. */
    TEST_ASSERT(nullptr == bconf.command);
    /* postcondition: private command is set to private_command_readconfig. */
    TEST_ASSERT(&private_command_readconfig == bconf.private_command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief An invalid private command calls error_usage.
 */
TEST(readconfig_invalid_private_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'P', 0 };
    char cmd[] = { 'f', 'o', 'o', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    TEST_ASSERT(nullptr == bconf.command);
    /* precondition: private_command should be NULL. */
    TEST_ASSERT(nullptr == bconf.private_command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to command_error_usage. */
    TEST_ASSERT(&command_error_usage == bconf.command);
    /* postcondition: private command is NULL. */
    TEST_ASSERT(nullptr == bconf.private_command);

    dispose((disposable_t*)&bconf);
}


/**
 * \brief The start command is a valid command.
 */
TEST(start_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char cmd[] = { 's', 't', 'a', 'r', 't', 0 };
    char* args[] = { exename, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    TEST_ASSERT(nullptr == bconf.command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to command_readconfig. */
    TEST_ASSERT(&command_start == bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief The supervisor private command is a valid private command.
 */
TEST(supervisor_private_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'P', 0 };
    char cmd[] = { 's', 'u', 'p', 'e', 'r', 'v', 'i', 's', 'o', 'r', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    TEST_ASSERT(nullptr == bconf.command);
    /* precondition: private_command should be NULL. */
    TEST_ASSERT(nullptr == bconf.private_command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to NULL. */
    TEST_ASSERT(nullptr == bconf.command);
    /* postcondition: private command is set to private_command_readconfig. */
    TEST_ASSERT(&private_command_supervisor == bconf.private_command);

    dispose((disposable_t*)&bconf);
}
