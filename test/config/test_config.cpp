/**
 * \file test_config.cpp
 *
 * Test the config parser.
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

TEST_SUITE(config_test);

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
 * Test that an empty config file produces a blank config.
 */
TEST(empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a logdir setting adds this data to the config.
 */
TEST(logdir_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("logdir log", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(!strcmp("log", user_context.config->logdir));
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a dot path logdir setting adds this data to the config.
 */
TEST(logdir_dotpath_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("logdir ./log", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(!strcmp("./log", user_context.config->logdir));
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that an absolute path for log is not accepted.
 */
TEST(logdir_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("logdir /log", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a relative path starting with .. for log is not accepted.
 */
TEST(logdir_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("logdir ../log", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a loglevel setting adds this data to the config.
 */
TEST(loglevel_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("loglevel 7", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(user_context.config->loglevel_set);
    TEST_ASSERT(7L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that bad loglevel ranges raise an error.
 */
TEST(loglevel_bad_range)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("loglevel 15", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter adds data to the config.
 */
TEST(secret_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("secret dir", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(!strcmp("dir", user_context.config->secret));
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can be a dot path.
 */
TEST(secret_dotpath_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("secret ./dir", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(!strcmp("./dir", user_context.config->secret));
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can't be absolute.
 */
TEST(secret_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("secret /dir", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can't be a dotdot relative path.
 */
TEST(secret_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("secret ../dir", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the rootblock parameter adds data to the config.
 */
TEST(rootblock_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state = yy_scan_string("rootblock root", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(!strcmp("root", user_context.config->rootblock));
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a rootblock path can be parsed.
 */
TEST(rootblock_path_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state = yy_scan_string("rootblock root/root.cert", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(!strcmp("root/root.cert", user_context.config->rootblock));
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a rootblock dot path can be parsed.
 */
TEST(rootblock_dot_path_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state = yy_scan_string("rootblock ./root/root.cert", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(!strcmp("./root/root.cert", user_context.config->rootblock));
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that relative paths starting with .. are not allowed.
 */
TEST(rootblock_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string("rootblock ../root/root.cert", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that no absolute paths are allowed in rootblock.
 */
TEST(rootblock_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state = yy_scan_string("rootblock /root/root.cert", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter adds data to the config.
 */
TEST(datastore_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("datastore data", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(!strcmp("data", user_context.config->datastore));
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can be a dot path.
 */
TEST(datastore_dotpath)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("datastore ./data", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(!strcmp("./data", user_context.config->datastore));
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can't be absolute.
 */
TEST(datastore_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("datastore /data", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can't be a dotdot relative path.
 */
TEST(datastore_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("datastore ../data", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a single listen parameter is added to the config.
 */
TEST(listen_single)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr != (state = yy_scan_string("listen 0.0.0.0:1234", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* check listeners. */
    TEST_ASSERT(nullptr != user_context.config->listen_head);
    TEST_EXPECT(0UL == user_context.config->listen_head->addr->s_addr);
    TEST_EXPECT(1234 == user_context.config->listen_head->port);
    TEST_ASSERT(nullptr == user_context.config->listen_head->hdr.next);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that multiple config parameters are added to the config.
 */
TEST(listen_double)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string(
                        "listen 0.0.0.0:1234\n"
                        "listen 1.2.3.4:4321\n",
                        scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* check listeners. */
    config_listen_address_t* listen = user_context.config->listen_head;
    TEST_ASSERT(nullptr != listen);
    TEST_EXPECT(0x04030201UL == listen->addr->s_addr);
    TEST_EXPECT(4321 == listen->port);
    listen = (config_listen_address_t*)listen->hdr.next;
    TEST_ASSERT(nullptr != listen);
    TEST_EXPECT(0UL == listen->addr->s_addr);
    TEST_EXPECT(1234 == listen->port);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter is added to the config.
 */
TEST(chroot_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("chroot root", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(!strcmp("root", user_context.config->chroot));
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can be a dot relative path.
 */
TEST(chroot_dot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("chroot ./root", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(!strcmp("./root", user_context.config->chroot));
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can't be an absolute path.
 */
TEST(chroot_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("chroot /root", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can't be a dotdot relative path.
 */
TEST(chroot_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("chroot ../root", scanner)));
    TEST_ASSERT(1 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a usergroup parameter is added to the config.
 */
TEST(usergroup_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("usergroup foo:bar", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);
    TEST_ASSERT(nullptr != user_context.config->usergroup);
    TEST_ASSERT(!strcmp("foo", user_context.config->usergroup->user));
    TEST_ASSERT(!strcmp("bar", user_context.config->usergroup->group));

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a canonization block parameter is accepted.
 */
TEST(empty_canonization_block)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("canonization { }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(!user_context.config->block_max_milliseconds_set);
    TEST_ASSERT(!user_context.config->block_max_transactions_set);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the block max milliseconds can be overridden.
 */
TEST(block_max_milliseconds)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string("canonization { max milliseconds 995 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(user_context.config->block_max_milliseconds_set);
    TEST_ASSERT(995 == user_context.config->block_max_milliseconds);
    TEST_ASSERT(!user_context.config->block_max_transactions_set);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a negative block max milliseconds is invalid.
 */
TEST(block_max_milliseconds_negative)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string("canonization { max milliseconds -7 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that too large of a block max milliseconds is invalid.
 */
TEST(block_max_milliseconds_large)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string(
                "canonization { max milliseconds 9999999999 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the block max transactions can be overridden.
 */
TEST(block_max_transactions)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string("canonization { max transactions 17 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(!user_context.config->block_max_milliseconds_set);
    TEST_ASSERT(user_context.config->block_max_transactions_set);
    TEST_ASSERT(17 == user_context.config->block_max_transactions);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a negative block max transactions is invalid.
 */
TEST(block_max_milliseconds_transactions)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string("canonization { max transactions -19 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that too large of a block max transactions is invalid.
 */
TEST(block_max_transactions_large)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string(
                "canonization { max transactions 9999999 }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a materialized view section.
 */
TEST(empty_materialized_view)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string("materialized view auth { }", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));
    /* there should be no artifact types set. */
    TEST_EXPECT(nullptr == user_context.config->view_head->artifact_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate view names causes an error.
 */
TEST(duplicate_materialized_view)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state =
            yy_scan_string(
                "materialized view auth { } "
                "materialized view auth { }",
                scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact type section.
 */
TEST(empty_artifact_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));
    /* there should be no transaction types. */
    TEST_ASSERT(nullptr == artifact->transaction_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate artifact types cause an error.
 */
TEST(duplicate_artifact_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { } "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { } "
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a transaction type section.
 */
TEST(empty_transaction_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate transaction types cause an error.
 */
TEST(duplicate_transaction_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { } "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { } "
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there should be 1 error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact create crud flag.
 */
TEST(artifact_create_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { create }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the CREATE crud flag should be set. */
    TEST_EXPECT(
        MATERIALIZED_VIEW_CRUD_CREATE == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact update crud flag.
 */
TEST(artifact_update_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { update }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the UPDATE crud flag should be set. */
    TEST_EXPECT(
        MATERIALIZED_VIEW_CRUD_UPDATE == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact append crud flag.
 */
TEST(artifact_append_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { append }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the APPEND crud flag should be set. */
    TEST_EXPECT(
        MATERIALIZED_VIEW_CRUD_APPEND == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact delete crud flag.
 */
TEST(artifact_delete_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { delete }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the DELETE crud flag should be set. */
    TEST_EXPECT(
        MATERIALIZED_VIEW_CRUD_DELETE == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can mix artifact crud flags.
 */
TEST(artifact_mix_crud_flags)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { create update append delete }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the CREATE UPDATE APPEND and DELETE crud flags should be set. */
    TEST_EXPECT(
        (  MATERIALIZED_VIEW_CRUD_CREATE | MATERIALIZED_VIEW_CRUD_UPDATE
           | MATERIALIZED_VIEW_CRUD_APPEND | MATERIALIZED_VIEW_CRUD_DELETE)
            == transaction->artifact_crud_flags);
    /* it should have no fields. */
    TEST_EXPECT(nullptr == transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field type section.
 */
TEST(empty_field_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * A duplicate field type should cause an error.
 */
TEST(duplicate_field_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { } "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { } "
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there should be one error. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field create crud flag.
 */
TEST(field_create_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "create"
                        " }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the CREATE crud flag should be set. */
    TEST_EXPECT(MATERIALIZED_VIEW_CRUD_CREATE == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field update crud flag.
 */
TEST(field_update_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "update"
                        " }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the UPDATE crud flag should be set. */
    TEST_EXPECT(MATERIALIZED_VIEW_CRUD_UPDATE == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field append crud flag.
 */
TEST(field_append_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "append"
                        " }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the APPEND crud flag should be set. */
    TEST_EXPECT(MATERIALIZED_VIEW_CRUD_APPEND == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field delete crud flag.
 */
TEST(field_delete_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "delete"
                        " }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the DELETE crud flag should be set. */
    TEST_EXPECT(MATERIALIZED_VIEW_CRUD_DELETE == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can mix field crud flags.
 */
TEST(field_mix_crud_flags)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    TEST_ASSERT(
        0
            == vpr_uuid_from_string(
                    &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "create update append delete"
                        " }"
                    " }"
                " }"
            " }",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* a view entry should be populated. */
    TEST_ASSERT(nullptr != user_context.config->view_head);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == user_context.config->view_head->hdr.next);
    /* the name should be set. */
    TEST_EXPECT(!strcmp("auth", user_context.config->view_head->name));

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    TEST_ASSERT(nullptr != artifact);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == artifact->hdr.next);
    /* the type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &artifact->artifact_type, &ARTIFACT_TYPE,
                    sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    TEST_ASSERT(nullptr != transaction);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == transaction->hdr.next);
    /* the transaction type should be set. */
    TEST_EXPECT(
        0
            == memcmp(
                    &transaction->transaction_type, &TRANSACTION_TYPE,
                    sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    TEST_EXPECT(0U == transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    TEST_ASSERT(nullptr != field);
    /* it should be the only entry. */
    TEST_EXPECT(nullptr == field->hdr.next);
    /* the field code should be set. */
    TEST_EXPECT(
        0
            == memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    TEST_EXPECT(0U == field->short_code);
    /* the CREATE UPDATE APPEND and DELETE crud flags should be set. */
    TEST_EXPECT(
        (  MATERIALIZED_VIEW_CRUD_CREATE | MATERIALIZED_VIEW_CRUD_UPDATE
          | MATERIALIZED_VIEW_CRUD_APPEND | MATERIALIZED_VIEW_CRUD_DELETE)
            == field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that, by default, the private key is NOT set.
 */
TEST(private_key_empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* the private key is NULL. */
    TEST_ASSERT(nullptr == user_context.config->private_key);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can set a private key.
 */
TEST(private_key_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string("private key private/123.cert", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* the private key is NOT NULL. */
    TEST_ASSERT(nullptr != user_context.config->private_key);

    /* the private key file is set. */
    TEST_ASSERT(nullptr != user_context.config->private_key->filename);
    /* the filename is what we set above. */
    TEST_EXPECT(
        !strcmp(
            "private/123.cert", user_context.config->private_key->filename));

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate private key entries fail.
 */
TEST(private_key_duplicates)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "private key private/123.cert "
            "private key private/456.cert ",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are errors. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that an empty authorized entity block has no effect on the config.
 */
TEST(empty_authorized_entities)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state 
                    = yy_scan_string(
                        "authorized entities { }",
                        scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an authorized entity.
 */
TEST(authorized_entity_single)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string(
                        "authorized entities { "
                            "public/foo.cert }",
                        scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);

    /* the public key list is NOT NULL. */
    TEST_ASSERT(nullptr != user_context.config->public_key_head);

    /* the public key file is set. */
    TEST_ASSERT(nullptr != user_context.config->public_key_head->filename);
    /* the filename is what we set above. */
    TEST_EXPECT(
        !strcmp(
            "public/foo.cert", user_context.config->public_key_head->filename));
    /* this is the only entry. */
    TEST_EXPECT(nullptr == user_context.config->public_key_head->hdr.next);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add multiple authorized entities.
 */
TEST(authorized_entities)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string(
                        "authorized entities { "
                            "public/foo.cert "
                            "public/bar.cert "
                            "public/baz.cert }",
                        scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);

    /* the public key list is NOT NULL. */
    TEST_ASSERT(nullptr != user_context.config->public_key_head);
    config_public_key_entry_t* pub = user_context.config->public_key_head;

    /* the public key file is set. */
    TEST_ASSERT(nullptr != pub->filename);
    /* the last filename appears first. */
    TEST_EXPECT(!strcmp( "public/baz.cert", pub->filename));
    /* this is the first entry. */
    TEST_ASSERT(nullptr != pub->hdr.next);
    pub = (config_public_key_entry_t*)pub->hdr.next;

    /* it's the second filename. */
    TEST_EXPECT(!strcmp( "public/bar.cert", pub->filename));
    /* there is one more entry. */
    TEST_ASSERT(nullptr != pub->hdr.next);
    pub = (config_public_key_entry_t*)pub->hdr.next;

    /* it's the first filename. */
    TEST_EXPECT(!strcmp( "public/foo.cert", pub->filename));
    /* there are no more entries. */
    TEST_EXPECT(nullptr == pub->hdr.next);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a max database size setting adds this setting to the config.
 */
TEST(max_database_size)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string("max datastore size 1024", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(user_context.config->database_max_size_set);
    TEST_ASSERT(1024L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->private_key);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that, by default, the endorser key is NOT set.
 */
TEST(endorser_key_empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr != (state = yy_scan_string("", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* the endorser key is NULL. */
    TEST_ASSERT(nullptr == user_context.config->endorser_key);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can set the endorser key.
 */
TEST(endorser_key_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(
        nullptr
            != (state
                    = yy_scan_string("endorser key public/123.pub", scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    TEST_ASSERT(0U == user_context.errors.size());

    /* verify user config. */
    TEST_ASSERT(nullptr != user_context.config);
    TEST_ASSERT(nullptr == user_context.config->logdir);
    TEST_ASSERT(!user_context.config->loglevel_set);
    TEST_ASSERT(0L == user_context.config->loglevel);
    TEST_ASSERT(!user_context.config->database_max_size_set);
    TEST_ASSERT(0L == user_context.config->database_max_size);
    TEST_ASSERT(nullptr == user_context.config->secret);
    TEST_ASSERT(nullptr == user_context.config->rootblock);
    TEST_ASSERT(nullptr == user_context.config->datastore);
    TEST_ASSERT(nullptr == user_context.config->listen_head);
    TEST_ASSERT(nullptr == user_context.config->chroot);
    TEST_ASSERT(nullptr == user_context.config->usergroup);
    TEST_ASSERT(nullptr == user_context.config->view_head);
    TEST_ASSERT(nullptr == user_context.config->public_key_head);

    /* the endorser key is NOT NULL. */
    TEST_ASSERT(nullptr != user_context.config->endorser_key);

    /* the endorser key file is set. */
    TEST_ASSERT(nullptr != user_context.config->endorser_key->filename);
    /* the filename is what we set above. */
    TEST_EXPECT(
        !strcmp("public/123.pub", user_context.config->endorser_key->filename));

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate endorser key entries fail.
 */
TEST(endorser_key_duplicates)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    TEST_ASSERT(0 == yylex_init(&scanner));
    TEST_ASSERT(nullptr !=
        (state = yy_scan_string(
            "endorser key public/123.pub "
            "endorser key public/456.pub ",
            scanner)));
    TEST_ASSERT(0 == yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are errors. */
    TEST_ASSERT(1U == user_context.errors.size());

    dispose((disposable_t*)&user_context);
}
