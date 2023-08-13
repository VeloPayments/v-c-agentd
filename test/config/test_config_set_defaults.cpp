/**
 * \file test_config_set_defaults.cpp
 *
 * Test that we can set reasonable defaults for config data.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <iostream>
#include <minunit/minunit.h>
#include <string>
#include <vector>
#include <vpr/disposable.h>

extern "C" {
#include "agentd.tab.h"
#include "agentd.yy.h"
}

using namespace std;

TEST_SUITE(config_set_defaults_test);

/**
 * \brief Simple user context structure for testing.
 */
struct test_context
{
    disposable_t hdr;
    vector<string> errors;
    agent_config_t* config;
};

/**
 * \brief Dispose a test context.
 */
static void test_context_dispose(void* disp)
{
    test_context* ctx = (test_context*)disp;

    if (nullptr != ctx->config)
        dispose((disposable_t*)ctx->config);
}

/**
 * \brief Initialize a test_context structure.
 */
static void test_context_init(test_context* ctx)
{
    ctx->hdr.dispose = &test_context_dispose;
    ctx->config = nullptr;
}

/**
 * \brief Simple error setting override.
 */
static void set_error(config_context_t* context, const char* msg)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->errors.push_back(msg);
}

/**
 * \brief Simple value setting override.
 */
static void config_callback(config_context_t* context, agent_config_t* config)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->config = config;
}

/**
 * Test that all defaults are set.
 */
TEST(empty_config)
{
    int64_t DEFAULT_MAX_DATABASE_SIZE = 16L * 1024L * 1024L * 1024L * 1024L;
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    bootstrap_config_t bconf;

    /* set up parse of empty config. */
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    TEST_ASSERT(0U == user_context.errors.size());

    /* initialize bootstrap config. */
    bootstrap_config_init(&bconf);
    bconf.prefix_dir = strdup("build/isolation");
    TEST_ASSERT(0 == system("mkdir -p build/isolation"));

    /* PRECONDITIONS: all config values are unset. */
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(!user_context.config->block_max_milliseconds_set);
    TEST_ASSERT(!user_context.config->block_max_transactions_set);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);

    /* set the defaults for this config. */
    TEST_ASSERT(0 == config_set_defaults(user_context.config, &bconf));

    /* POSTCONDITIONS: all config values have their defaults. */
    TEST_ASSERT(!strcmp("log", user_context.config->logdir));
    TEST_ASSERT(user_context.config->loglevel_set);
    TEST_ASSERT(4 == user_context.config->loglevel);
    TEST_ASSERT(
        DEFAULT_MAX_DATABASE_SIZE == user_context.config->database_max_size);
    TEST_ASSERT(user_context.config->block_max_milliseconds_set);
    TEST_ASSERT(5000 == user_context.config->block_max_milliseconds);
    TEST_ASSERT(user_context.config->block_max_transactions_set);
    TEST_ASSERT(500 == user_context.config->block_max_transactions);
    TEST_ASSERT(!strcmp("root/secret.cert", user_context.config->secret));
    TEST_ASSERT(!strcmp("root/root.cert", user_context.config->rootblock));
    TEST_ASSERT(!strcmp("data", user_context.config->datastore));
    TEST_ASSERT(nullptr != user_context.config->listen_head);
    TEST_ASSERT(!strcmp(bconf.prefix_dir, user_context.config->chroot));
    TEST_ASSERT(nullptr != user_context.config->usergroup);
    TEST_ASSERT(!strcmp("veloagent", user_context.config->usergroup->user));
    TEST_ASSERT(!strcmp("veloagent", user_context.config->usergroup->group));
    TEST_ASSERT(nullptr == user_context.config->view_head);

    /* clean up. */
    dispose((disposable_t*)&user_context);
    dispose((disposable_t*)&bconf);
}
