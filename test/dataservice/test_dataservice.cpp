/**
 * \file test_dataservice.cpp
 *
 * Test the data service private API.
 *
 * \copyright 2018-2023 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <minunit/minunit.h>
#include <vccert/certificate_types.h>

#include "test_dataservice.h"

using namespace std;

static const uint64_t DEFAULT_DATABASE_SIZE = 1024 * 1024;

TEST_SUITE(dataservice_test);

#define BEGIN_TEST_F(name) \
TEST(name) \
{ \
    dataservice_test fixture; \
    fixture.setUp();

#define END_TEST_F() \
    fixture.tearDown(); \
}

/**
 * Test that the data service root context can be initialized.
 */
BEGIN_TEST_F(root_context_init)
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* We can't create a root context again. */
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));

    /* All other capabilities are set by default. */
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that without the root create capability, we cannot create a root
 * context.
 */
BEGIN_TEST_F(root_context_init_no_permission)
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly forbid the capability to create this root context. */
    BITCAP_SET_FALSE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should fail. */
    TEST_ASSERT(
        0
            != dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));
END_TEST_F()

/**
 * Test that we can reduce the capabilities in the root context -- in this case,
 * we reduce all capabilities except further reducing capabilities, and then we
 * eliminate that capability and demonstrate that it is no longer possible to
 * further reduce capabilities.
 */
BEGIN_TEST_F(root_context_reduce_capabilities)
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly set the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should succeed. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* We can't create a root context again. */
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));

    /* All other capabilities are set by default. */
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE));
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* reduce the capabilites to only allow the capabilities to be further
     * reduced. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* the call to reduce capabilities should succeed. */
    TEST_ASSERT(
        0
            == dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* We can further reduce capabilities. */
    TEST_EXPECT(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));

    /* All other capabilities are disabled. */
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* reduce the capabilites to nothing. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* the call to reduce capabilities should succeed. */
    TEST_ASSERT(
        0
            == dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* All capabilities are disabled. */
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE));
    TEST_EXPECT(!BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* the call to reduce capabilities will now fail. */
    TEST_ASSERT(
        0
            != dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that a child context can be created from a root context.
 */
BEGIN_TEST_F(child_context_create)
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* the child context cannot create other child contexts. */
    TEST_EXPECT(!
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));

    /* the child context can close itself. */
    TEST_EXPECT(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));

    /* verify that this child context can read transactions. */
    TEST_EXPECT(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_APP_TRANSACTION_READ));

    /* verify that other capabilities, like database backup, are disabled. */
    TEST_EXPECT(!
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_DATABASE_BACKUP));

    /* verify that trying to create the child context a second time fails. */
    TEST_ASSERT(
        0 != dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that a child context cannot be created from a root context if the root
 * context does not have the create child context capability.
 */
BEGIN_TEST_F(child_context_create_denied)
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* explicitly deny child context creation in the parent context. */
    BITCAP_SET_FALSE(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* creating a child fails because root cannot create child contexts. */
    TEST_ASSERT(
        0 != dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that a child context can be closed.
 */
BEGIN_TEST_F(child_context_close)
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* closing the child context succeeds. */
    TEST_ASSERT(0 == dataservice_child_context_close(&child));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that closing a child context fails if it lacks the close cap.
 */
BEGIN_TEST_F(child_context_close_denied)
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* make sure the child create context cap is set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* explicitly deny child close context cap. */
    BITCAP_SET_FALSE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* closing the child context fails. */
    TEST_ASSERT(0 != dataservice_child_context_close(&child));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can query a global setting that is already saved in the
 * database.
 */
BEGIN_TEST_F(global_settings_get)
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));
    TEST_ASSERT(0 == mdb_put(txn, details->global_db, &key, &val, 0));
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should succeed. */
    TEST_ASSERT(0
            == dataservice_global_settings_get(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    schema_buffer, &schema_buffer_sz));

    /* the buffer size should be the size of the schema UUID. */
    TEST_ASSERT(sizeof(SCHEMA_VERSION) == schema_buffer_sz);

    /* the schema buffer should match the schema UUID. */
    TEST_EXPECT(0 == memcmp(schema_buffer, SCHEMA_VERSION, schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that if we are not allowed to query a global setting, the API call
 * fails.
 */
BEGIN_TEST_F(global_settings_get_denied)
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    /* don't allow it to query global settings. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));
    TEST_ASSERT(0 == mdb_put(txn, details->global_db, &key, &val, 0));
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail. */
    TEST_ASSERT(
        0
            != dataservice_global_settings_get(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    schema_buffer, &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we get a truncation error if attempting to query a value with too
 * small of a buffer.
 */
BEGIN_TEST_F(global_settings_get_would_truncate)
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[10];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));
    TEST_ASSERT(0 == mdb_put(txn, details->global_db, &key, &val, 0));
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail due to truncation. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_WOULD_TRUNCATE
            == dataservice_global_settings_get(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    schema_buffer, &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we get a value not found error when querying for a value not in the
 * database.
 */
BEGIN_TEST_F(global_settings_get_not_found)
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail due to the value not being faund. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_global_settings_get(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    schema_buffer, &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can set a global setting and then get it.
 */
BEGIN_TEST_F(global_settings_set_get)
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings put / get. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* setting the global setting should succeed. */
    TEST_ASSERT(
        0
            == dataservice_global_settings_set(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    SCHEMA_VERSION, sizeof(SCHEMA_VERSION)));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should succeed. */
    TEST_ASSERT(
        0
            == dataservice_global_settings_get(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    schema_buffer, &schema_buffer_sz));

    /* the buffer size should be the size of the schema UUID. */
    TEST_ASSERT(sizeof(SCHEMA_VERSION) == schema_buffer_sz);

    /* the schema buffer should match the schema UUID from the set call. */
    TEST_EXPECT(0 == memcmp(schema_buffer, SCHEMA_VERSION, schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that global settings set respects the global settings write capability.
 */
BEGIN_TEST_F(global_settings_set_denied)
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* there should be a disposer set. */
    TEST_ASSERT(nullptr != ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* setting the global setting should fail. */
    TEST_ASSERT(
        0
            != dataservice_global_settings_set(
                    &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                    SCHEMA_VERSION, sizeof(SCHEMA_VERSION)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we transaction_get_first indicates that no transaction is found
 * when the transaction queue is empty.
 */
BEGIN_TEST_F(transaction_get_first_empty)
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction should return a "not found" result. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get_first(
                    &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    TEST_ASSERT(nullptr == txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we transaction_get_first indicates that no transaction is found
 * when the transaction queue exists and is empty.
 */
BEGIN_TEST_F(transaction_get_first_empty_with_start_end)
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memset(start.next, 0xFF, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memset(end.prev, 0, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* getting the first transaction should return a "not found" result. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get_first(
                    &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    TEST_ASSERT(nullptr == txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we transaction_get_first fails when called without the appropriate
 * capability being set.
 */
BEGIN_TEST_F(transaction_get_first_no_capability)
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* conspicuously, no transaction caps. */

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction should fail due to missing caps. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_get_first(
                    &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    TEST_ASSERT(nullptr == txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we transaction_get_first retrieves the first found transaction.
 */
BEGIN_TEST_F(transaction_get_first_happy_path)
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
    free(foo);
    free(bar);
END_TEST_F()

/**
 * Test that we transaction_get_first retrieves the first found transaction
 * while under a transaction.
 */
BEGIN_TEST_F(transaction_get_first_txn_happy_path)
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    TEST_ASSERT(
        0 == mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit the transaction. */
    mdb_txn_commit(txn);

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    TEST_ASSERT(
        0 == dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, &txn_ctx, nullptr, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(foo);
    free(bar);
END_TEST_F()

/**
 * Test that we transaction_get_first retrieves the first found transaction and
 * populates the provided transaction node.
 */
BEGIN_TEST_F(transaction_get_first_with_node_happy_path)
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    TEST_ASSERT(0 == mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
#if ATTESTATION == 1
    foo->net_txn_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED);
#else
    foo->net_txn_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED);
#endif
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));
#if ATTESTATION == 1
    bar->net_txn_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED);
#else
    bar->net_txn_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED);
#endif

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    TEST_ASSERT(0 == mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    TEST_ASSERT(0 == mdb_txn_commit(txn));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    TEST_EXPECT(0 == memcmp(node.key, foo_key, sizeof(node.key)));
    TEST_EXPECT(0 == memcmp(node.prev, start_key, sizeof(node.prev)));
    TEST_EXPECT(0 == memcmp(node.next, bar_key, sizeof(node.next)));
    TEST_EXPECT(txn_size == (size_t)ntohll(node.net_txn_cert_size));
#if ATTESTATION == 1
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
    free(foo);
    free(bar);
END_TEST_F()

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it.
 */
BEGIN_TEST_F(transaction_submit_get_first_with_node_happy_path)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    TEST_EXPECT(0 == memcmp(node.key, foo_key, sizeof(node.key)));
    TEST_EXPECT(0 == memcmp(node.prev, start_key, sizeof(node.prev)));
    TEST_EXPECT(0 == memcmp(node.next, end_key, sizeof(node.next)));
    TEST_EXPECT(sizeof(foo_data) == (size_t)ntohll(node.net_txn_cert_size));
#if ATTESTATION == 1
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
END_TEST_F()

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it, while under a transaction.
 */
BEGIN_TEST_F(transaction_submit_txn_get_first_with_node_happy_path)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    TEST_ASSERT(
        0 == dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, &txn_ctx, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, &txn_ctx, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    TEST_EXPECT(0 == memcmp(node.key, foo_key, sizeof(node.key)));
    TEST_EXPECT(0 == memcmp(node.prev, start_key, sizeof(node.prev)));
    TEST_EXPECT(0 == memcmp(node.next, end_key, sizeof(node.next)));
    TEST_EXPECT(sizeof(foo_data) == (size_t)ntohll(node.net_txn_cert_size));
#if ATTESTATION == 1
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it by id.
 */
BEGIN_TEST_F(transaction_submit_get_with_node_happy_path)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    TEST_EXPECT(0 == memcmp(node.key, foo_key, sizeof(node.key)));
    TEST_EXPECT(0 == memcmp(node.prev, start_key, sizeof(node.prev)));
    TEST_EXPECT(0 == memcmp(node.next, end_key, sizeof(node.next)));
    TEST_EXPECT(sizeof(foo_data) == (size_t)ntohll(node.net_txn_cert_size));
#if ATTESTATION == 1
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
END_TEST_F()

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it by id, while under a transaction.
 */
BEGIN_TEST_F(transaction_submit_txn_get_with_node_happy_path)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    TEST_ASSERT(
        0 == dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, &txn_ctx, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, &txn_ctx, foo_key, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    TEST_ASSERT(nullptr != txn_bytes);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    TEST_EXPECT(0 == memcmp(node.key, foo_key, sizeof(node.key)));
    TEST_EXPECT(0 == memcmp(node.prev, start_key, sizeof(node.prev)));
    TEST_EXPECT(0 == memcmp(node.next, end_key, sizeof(node.next)));
    TEST_EXPECT(sizeof(foo_data) == (size_t)ntohll(node.net_txn_cert_size));
#if ATTESTATION == 1
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_EXPECT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that an ettempt to drop the all zeroes or all FFs transactions results
 * in a "not found" error, even after a transaction has been submitted.
 */
BEGIN_TEST_F(transaction_drop_00_ff)
    uint8_t begin_key[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t end_key[16] = {
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff,
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff
    };
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* attempt to drop the begin transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_drop(
                    &child, nullptr, begin_key));

    /* attempt to drop the end transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_drop(&child, nullptr, end_key));

    /* submit foo transaction. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* attempt to drop the begin transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_drop(
                    &child, nullptr, begin_key));

    /* attempt to drop the end transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_drop(
                    &child, nullptr, end_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can drop an entry in the transaction queue after submitting
 * it.
 */
BEGIN_TEST_F(transaction_drop)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this transaction id should be ours. */
    TEST_ASSERT(0 == memcmp(node.key, foo_key, 16));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* attempt to drop foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_drop(
                    &child, nullptr, foo_key));

    /* getting the first transaction should fail. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* now if we try to get the transaction by id, this fails. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that other entries are preserved and updated when we drop an entry from
 * the queue.
 */
BEGIN_TEST_F(transaction_drop_ordering)
    uint8_t foo1_key[16] = {
        0x2a, 0x3d, 0xe3, 0x6f, 0x4f, 0x5f, 0x43, 0x75,
        0x8d, 0xaf, 0xb0, 0x74, 0x97, 0x8b, 0x51, 0x67
    };
    uint8_t foo1_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo1_data[16] = {
        0xfa, 0x99, 0xb1, 0x9d, 0x66, 0x7a, 0x4a, 0xe3,
        0x96, 0xf4, 0x50, 0xd6, 0x65, 0xda, 0x11, 0x5c
    };
    uint8_t foo2_key[16] = {
        0xb2, 0xea, 0x70, 0x5c, 0x42, 0xd4, 0x40, 0x21,
        0x96, 0xe1, 0x7e, 0x89, 0xfb, 0x04, 0x9a, 0x33
    };
    uint8_t foo2_artifact[16] = {
        0xeb, 0x18, 0xe9, 0x7b, 0x2e, 0x8a, 0x41, 0xf2,
        0xbf, 0xc5, 0xea, 0x7d, 0x65, 0x2a, 0x71, 0xce
    };
    uint8_t foo2_data[16] = {
        0x83, 0xf3, 0x6a, 0xa4, 0x71, 0xbe, 0x4f, 0xb6,
        0xa0, 0xcf, 0xe5, 0x69, 0x29, 0x23, 0x2b, 0xe0
    };
    uint8_t foo3_key[16] = {
        0x33, 0x48, 0xfd, 0x83, 0xa7, 0xc5, 0x4b, 0xf1,
        0x85, 0x2f, 0x27, 0x99, 0x90, 0x8a, 0xce, 0xbc
    };
    uint8_t foo3_artifact[16] = {
        0xf2, 0x90, 0xce, 0xe0, 0x44, 0x29, 0x49, 0x97,
        0xad, 0x8b, 0xb0, 0x77, 0x06, 0xe2, 0xc1, 0x97
    };
    uint8_t foo3_data[16] = {
        0x4f, 0x61, 0x98, 0x8e, 0x23, 0x84, 0x49, 0x29,
        0x92, 0x76, 0x84, 0x06, 0x42, 0x36, 0x3a, 0x6b
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo1 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo1_key, foo1_artifact, foo1_data,
                    sizeof(foo1_data)));

    /* submit foo2 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo2_key, foo2_artifact, foo2_data,
                    sizeof(foo2_data)));

    /* submit foo3 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo3_key, foo3_artifact, foo3_data,
                    sizeof(foo3_data)));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xff, sizeof(end_key));
    TEST_ASSERT(0 == memcmp(node.key, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo1_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo2_key, 16));
    TEST_ASSERT(sizeof(foo1_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    TEST_ASSERT(0 == memcmp(node.key, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo1_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo2_key, 16));
    TEST_ASSERT(sizeof(foo1_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    TEST_ASSERT(0 == memcmp(node.key, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo2_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo3_key, 16));
    TEST_ASSERT(sizeof(foo2_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    TEST_ASSERT(0 == memcmp(node.key, foo3_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo3_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, end_key, 16));
    TEST_ASSERT(sizeof(foo3_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* attempt to drop foo2 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_drop(
                    &child, nullptr, foo2_key));

    /* now if we try to get the transaction by id, this fails. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get(
                    &child, nullptr, foo2_key, &node, &txn_bytes, &txn_size));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    TEST_ASSERT(0 == memcmp(node.key, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo1_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo3_key, 16));
    TEST_ASSERT(sizeof(foo1_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    TEST_ASSERT(0 == memcmp(node.key, foo3_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo3_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, end_key, 16));
    TEST_ASSERT(sizeof(foo3_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that other entries are preserved and updated when we drop the first
 * entry from the queue.
 */
BEGIN_TEST_F(transaction_drop_first_ordering)
    uint8_t foo1_key[16] = {
        0x2a, 0x3d, 0xe3, 0x6f, 0x4f, 0x5f, 0x43, 0x75,
        0x8d, 0xaf, 0xb0, 0x74, 0x97, 0x8b, 0x51, 0x67
    };
    uint8_t foo1_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo1_data[16] = {
        0xfa, 0x99, 0xb1, 0x9d, 0x66, 0x7a, 0x4a, 0xe3,
        0x96, 0xf4, 0x50, 0xd6, 0x65, 0xda, 0x11, 0x5c
    };
    uint8_t foo2_key[16] = {
        0xb2, 0xea, 0x70, 0x5c, 0x42, 0xd4, 0x40, 0x21,
        0x96, 0xe1, 0x7e, 0x89, 0xfb, 0x04, 0x9a, 0x33
    };
    uint8_t foo2_artifact[16] = {
        0xeb, 0x18, 0xe9, 0x7b, 0x2e, 0x8a, 0x41, 0xf2,
        0xbf, 0xc5, 0xea, 0x7d, 0x65, 0x2a, 0x71, 0xce
    };
    uint8_t foo2_data[16] = {
        0x83, 0xf3, 0x6a, 0xa4, 0x71, 0xbe, 0x4f, 0xb6,
        0xa0, 0xcf, 0xe5, 0x69, 0x29, 0x23, 0x2b, 0xe0
    };
    uint8_t foo3_key[16] = {
        0x33, 0x48, 0xfd, 0x83, 0xa7, 0xc5, 0x4b, 0xf1,
        0x85, 0x2f, 0x27, 0x99, 0x90, 0x8a, 0xce, 0xbc
    };
    uint8_t foo3_artifact[16] = {
        0xf2, 0x90, 0xce, 0xe0, 0x44, 0x29, 0x49, 0x97,
        0xad, 0x8b, 0xb0, 0x77, 0x06, 0xe2, 0xc1, 0x97
    };
    uint8_t foo3_data[16] = {
        0x4f, 0x61, 0x98, 0x8e, 0x23, 0x84, 0x49, 0x29,
        0x92, 0x76, 0x84, 0x06, 0x42, 0x36, 0x3a, 0x6b
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo1 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo1_key, foo1_artifact, foo1_data,
                    sizeof(foo1_data)));

    /* submit foo2 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo2_key, foo2_artifact, foo2_data,
                    sizeof(foo2_data)));

    /* submit foo3 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo3_key, foo3_artifact, foo3_data,
                    sizeof(foo3_data)));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xff, sizeof(end_key));
    TEST_ASSERT(0 == memcmp(node.key, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo1_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo2_key, 16));
    TEST_ASSERT(sizeof(foo1_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    TEST_ASSERT(0 == memcmp(node.key, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo1_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo2_key, 16));
    TEST_ASSERT(sizeof(foo1_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    TEST_ASSERT(0 == memcmp(node.key, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo2_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo1_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo3_key, 16));
    TEST_ASSERT(sizeof(foo2_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    TEST_ASSERT(0 == memcmp(node.key, foo3_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo3_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, end_key, 16));
    TEST_ASSERT(sizeof(foo3_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* attempt to drop foo1 transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_drop(
                    &child, nullptr, foo1_key));

    /* now if we try to get the transaction by id, this fails. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get(
                    &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    TEST_ASSERT(0 == memcmp(node.key, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo2_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, begin_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, foo3_key, 16));
    TEST_ASSERT(sizeof(foo2_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* getting the next transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    TEST_ASSERT(0 == memcmp(node.key, foo3_key, 16));
    TEST_ASSERT(0 == memcmp(node.artifact_id, foo3_artifact, 16));
    TEST_ASSERT(0 == memcmp(node.prev, foo2_key, 16));
    TEST_ASSERT(0 == memcmp(node.next, end_key, 16));
    TEST_ASSERT(sizeof(foo3_data) == txn_size);
    TEST_ASSERT(0 == memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));
#if ATTESTATION == 1
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED
            == ntohl(node.net_txn_state));
#else
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));
#endif

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that an ettempt to promote the all zeroes or all FFs transactions
 * results in a "not found" error, even after a transaction has been submitted.
 */
BEGIN_TEST_F(transaction_promote_00_ff)
    uint8_t begin_key[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t end_key[16] = {
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff,
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff
    };
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read, and promote. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* attempt to promote the begin transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_promote(
                    &child, nullptr, begin_key));

    /* attempt to promote the end transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_promote(
                    &child, nullptr, end_key));

    /* submit foo transaction. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* attempt to promote the begin transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_promote(
                    &child, nullptr, begin_key));

    /* attempt to promote the end transaction. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_promote(
                    &child, nullptr, end_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can promote an entry in the transaction queue after submitting
 * it.
 */
BEGIN_TEST_F(transaction_promote)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and promote. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* getting the first transaction should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this transaction id should be ours. */
    TEST_ASSERT(0 == memcmp(node.key, foo_key, 16));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* attempt to promote foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_promote(
                    &child, nullptr, foo_key));

    /* getting the first transaction should succeed. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* the node state should be updated. */
    TEST_ASSERT(
        DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED
            == ntohl(node.net_txn_state));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that dataservice_transaction_submit respects the bitcap for this action.
 */
BEGIN_TEST_F(transaction_submit_bitcap)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submitting foo transaction fails. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_data,
                    sizeof(foo_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that dataservice_transaction_get_first respects the bitcap for this
 * action.
 */
BEGIN_TEST_F(transaction_get_first_bitcap)
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction fails due no capabilities. */
    data_transaction_node_t node;
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_get_first(
                    &child, nullptr, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that dataservice_transaction_get respects the bitcap for this action.
 */
BEGIN_TEST_F(transaction_get_bitcap)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction fails due no capabilities. */
    data_transaction_node_t node;
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that dataservice_transaction_drop respects the bitcap for this action.
 */
BEGIN_TEST_F(transaction_drop_bitcap)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dropping a transaction fails due to no capability. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_drop(
                    &child, nullptr, foo_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that dataservice_transaction_promote respects the bitcap for this
 * action.
 */
BEGIN_TEST_F(transaction_promote_bitcap)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* promoting a transaction fails due to no capability. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_transaction_promote(
                    &child, nullptr, foo_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
END_TEST_F()

/**
 * Test that we can add a transaction to the transaction queue, create a block
 * containing this transaction, and the dataservice_block_make API call
 * automatically drops this transaction.
 */
BEGIN_TEST_F(transaction_make_block_simple)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    data_transaction_node_t node;
    data_artifact_record_t foo_artifact_record;
    data_block_node_t block_node;
    uint8_t* txn_bytes;
    size_t txn_size;
    uint8_t* block_txn_bytes;
    size_t block_txn_size;
    uint8_t block_id_for_height_1[16];
    uint8_t latest_block_id[16];

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* verify that our block does not exist. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_get(
                    &child, nullptr, foo_block_id, &block_node,
                    &block_txn_bytes, &block_txn_size));

    /* verify that a block ID does not exist for block height 1. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_id_by_height_get(
                    &child, nullptr, 1, block_id_for_height_1));

    /* verify that the latest block id get call returns the root UUID. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_latest_block_id_get(
                    &child, nullptr, latest_block_id));
    TEST_ASSERT(
        0
            == memcmp(
                    latest_block_id, vccert_certificate_type_uuid_root_block,
                    16));

    /* verify that our artifact does not exist. */
    /* getting the artifact record by artifact id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_artifact_get(
                    &child, nullptr, foo_artifact, &foo_artifact_record));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* create foo block. */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts, foo_block_id,
                    vccert_certificate_type_uuid_root_block, 1, &foo_block_cert,
                    &foo_block_cert_length, foo_cert, foo_cert_length,
                    nullptr));

    /* getting the block transaction by id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* make block. */
    TEST_ASSERT(
        0
            == dataservice_block_make(
                    &child, nullptr, foo_block_id,
                    foo_block_cert, foo_block_cert_length));

    /* getting the transaction by id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* getting the block transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_block_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* getting the block record by block id should return success. */
    TEST_ASSERT(
        0
            == dataservice_block_get(
                    &child, nullptr, foo_block_id, &block_node,
                    &block_txn_bytes, &block_txn_size));
    /* the key should match our block id. */
    TEST_ASSERT(0 == memcmp(block_node.key, foo_block_id, 16));
    TEST_ASSERT(0 == memcmp(block_node.first_transaction_id, foo_key, 16));
    TEST_ASSERT(1U == ntohll(block_node.net_block_height));

    /* verify that a block ID exists for block height 1. */
    TEST_ASSERT(
        0
            == dataservice_block_id_by_height_get(
                    &child, nullptr, 1, block_id_for_height_1));
    /* this block ID matches our block ID. */
    TEST_EXPECT(0 == memcmp(foo_block_id, block_id_for_height_1, 16));

    /* verify that the latest block id matches our block id. */
    TEST_ASSERT(
        0
            == dataservice_latest_block_id_get(
                    &child, nullptr, latest_block_id));
    /* this block ID matches our block ID. */
    TEST_EXPECT(0 == memcmp(foo_block_id, latest_block_id, 16));

    /* getting the artifact record by artifact id should return success. */
    TEST_ASSERT(
        0
            == dataservice_artifact_get(
                    &child, nullptr, foo_artifact, &foo_artifact_record));
    /* the key should match the artifact ID. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.key, foo_artifact, 16));
    /* the first transaction should be the foo transaction. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.txn_first, foo_key, 16));
    /* the latest transaction should be the foo transaction. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.txn_latest, foo_key, 16));
    /* the first height for this artifact should be 1. */
    TEST_ASSERT(1U == ntohll(foo_artifact_record.net_height_first));
    /* the latest height for this artifact should be 1. */
    TEST_ASSERT(1U == ntohll(foo_artifact_record.net_height_latest));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
END_TEST_F()

/**
 * Test that the bitset is enforced for making blocks.
 */
BEGIN_TEST_F(transaction_make_block_bitset)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* DO NOT ALLOW BLOCK_WRITE. */
    /*BITCAP_SET_TRUE(reducedcaps,
                    DATASERVICE_API_CAP_APP_BLOCK_WRITE);*/
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* create foo block. */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts, foo_block_id,
                    vccert_certificate_type_uuid_root_block, 1, &foo_block_cert,
                    &foo_block_cert_length, foo_cert, foo_cert_length,
                    nullptr));

    /* make block should fail because of missing capability. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED
            == dataservice_block_make(
                    &child, nullptr, foo_block_id, foo_block_cert,
                    foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
END_TEST_F()

/**
 * Test that appending a block with an invalid height will fail.
 */
BEGIN_TEST_F(transaction_make_block_bad_height)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* create foo block with invalid 0 block height. */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts, foo_block_id,
                    vccert_certificate_type_uuid_root_block, 0, &foo_block_cert,
                    &foo_block_cert_length, foo_cert, foo_cert_length,
                    nullptr));

    /* make block fails due to invalid block height. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_INVALID_BLOCK_HEIGHT
            == dataservice_block_make(
                    &child, nullptr, foo_block_id, foo_block_cert,
                    foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
END_TEST_F()

/**
 * Test that appending a block with an invalid previous block ID will fail.
 */
BEGIN_TEST_F(transaction_make_block_bad_prev_block_id)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* create foo block with invalid previous block ID. */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts, foo_block_id, fixture.zero_uuid, 1,
                    &foo_block_cert, &foo_block_cert_length, foo_cert,
                    foo_cert_length, nullptr));

    /* make block fails due to invalid previous block ID. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_INVALID_PREVIOUS_BLOCK_UUID
            == dataservice_block_make(
                    &child, nullptr, foo_block_id, foo_block_cert,
                    foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
END_TEST_F()

/**
 * Test that appending a block with an invalid block ID will fail.
 */
BEGIN_TEST_F(transaction_make_block_bad_block_id)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* create foo block with invalid block ID (root block ID). */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts,
                    vccert_certificate_type_uuid_root_block,
                    vccert_certificate_type_uuid_root_block, 1, &foo_block_cert,
                    &foo_block_cert_length, foo_cert, foo_cert_length,
                    nullptr));

    /* make block fails due to invalid block ID. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_INVALID_BLOCK_UUID
            == dataservice_block_make(
                    &child, nullptr, foo_block_id, foo_block_cert,
                    foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
END_TEST_F()

/**
 * Test that dataservice_api_node_ref_is_beginning matches against a begin node.
 */
BEGIN_TEST_F(node_ref_is_beginning)
    const uint8_t BEGINNING[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t NOT_BEGINNING[16] = {
        0x8f, 0x8c, 0x87, 0xd0, 0xe7, 0x55, 0x43, 0xa2,
        0x95, 0x28, 0x3a, 0xb2, 0x55, 0x15, 0xbc, 0x05
    };

    TEST_EXPECT(dataservice_api_node_ref_is_beginning(BEGINNING));
    TEST_EXPECT(!dataservice_api_node_ref_is_beginning(NOT_BEGINNING));
END_TEST_F()

/**
 * Test that dataservice_api_node_ref_is_beginning matches against an end node.
 */
BEGIN_TEST_F(node_ref_is_end)
    const uint8_t END[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    const uint8_t NOT_END[16] = {
        0x8f, 0x8c, 0x87, 0xd0, 0xe7, 0x55, 0x43, 0xa2,
        0x95, 0x28, 0x3a, 0xb2, 0x55, 0x15, 0xbc, 0x05
    };

    TEST_EXPECT(dataservice_api_node_ref_is_end(END));
    TEST_EXPECT(!dataservice_api_node_ref_is_end(NOT_END));
END_TEST_F()

/**
 * Getting the root block's next block id succeeds once we make a block.
 */
BEGIN_TEST_F(transaction_empty_root_next_block_id)
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    data_transaction_node_t node;
    data_artifact_record_t foo_artifact_record;
    data_block_node_t block_node;
    uint8_t* txn_bytes;
    size_t txn_size;
    uint8_t* block_txn_bytes;
    uint8_t* root_block_txn_bytes;
    size_t block_txn_size;
    size_t root_block_txn_size;
    uint8_t block_id_for_height_1[16];
    uint8_t latest_block_id[16];

    /* create the directory for this test. */
    TEST_ASSERT(0 == fixture.createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    TEST_ASSERT(
        0
            == dataservice_root_context_init(
                    &ctx, DEFAULT_DATABASE_SIZE, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    TEST_ASSERT(
        0 == dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* verify that our block does not exist. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_get(
                    &child, nullptr, foo_block_id, &block_node,
                    &block_txn_bytes, &block_txn_size));

    /* verify that a block ID does not exist for block height 1. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_id_by_height_get(
                    &child, nullptr, 1, block_id_for_height_1));

    /* verify that the latest block id get call returns the root UUID. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_latest_block_id_get(
                    &child, nullptr, latest_block_id));
    TEST_ASSERT(
        0
            == memcmp(
                    latest_block_id, vccert_certificate_type_uuid_root_block,
                    16));

    /* verify that if we try to get the root block id, we get nothing. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_get(
                    &child, nullptr, vccert_certificate_type_uuid_root_block,
                    &block_node, &block_txn_bytes, &block_txn_size));

    /* verify that our artifact does not exist. */
    /* getting the artifact record by artifact id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_artifact_get(
                    &child, nullptr, foo_artifact, &foo_artifact_record));

    /* create foo transaction. */
    TEST_ASSERT(
        0
            == fixture.create_dummy_transaction(
                    foo_key, foo_prev, foo_artifact, &foo_cert,
                    &foo_cert_length));

    /* submit foo transaction. */
    TEST_ASSERT(
        0
            == dataservice_transaction_submit(
                    &child, nullptr, foo_key, foo_artifact, foo_cert,
                    foo_cert_length));

    /* getting the transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* create foo block. */
    TEST_ASSERT(
        0
            == create_dummy_block(
                    &fixture.builder_opts, foo_block_id,
                    vccert_certificate_type_uuid_root_block, 1, &foo_block_cert,
                    &foo_block_cert_length, foo_cert, foo_cert_length,
                    nullptr));

    /* getting the block transaction by id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_block_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* make block. */
    TEST_ASSERT(
        0
            == dataservice_block_make(
                    &child, nullptr, foo_block_id, foo_block_cert,
                    foo_block_cert_length));

    /* getting the transaction by id should return not found. */
    TEST_ASSERT(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND
            == dataservice_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* getting the block transaction by id should return success. */
    TEST_ASSERT(
        0
            == dataservice_block_transaction_get(
                    &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* getting the block record by block id should return success. */
    TEST_ASSERT(
        0
            == dataservice_block_get(
                    &child, nullptr, foo_block_id, &block_node,
                    &block_txn_bytes, &block_txn_size));
    /* the key should match our block id. */
    TEST_ASSERT(0 == memcmp(block_node.key, foo_block_id, 16));
    TEST_ASSERT(0 == memcmp(block_node.first_transaction_id, foo_key, 16));
    TEST_ASSERT(1U == ntohll(block_node.net_block_height));

    /* verify that a block ID exists for block height 1. */
    TEST_ASSERT(
        0
            == dataservice_block_id_by_height_get(
                    &child, nullptr, 1, block_id_for_height_1));
    /* this block ID matches our block ID. */
    TEST_EXPECT(0 == memcmp(foo_block_id, block_id_for_height_1, 16));

    /* verify that the latest block id matches our block id. */
    TEST_ASSERT(
        0
            == dataservice_latest_block_id_get(
                    &child, nullptr, latest_block_id));
    /* this block ID matches our block ID. */
    TEST_EXPECT(0 == memcmp(foo_block_id, latest_block_id, 16));

    /* getting the artifact record by artifact id should return success. */
    TEST_ASSERT(
        0
            == dataservice_artifact_get(
                    &child, nullptr, foo_artifact, &foo_artifact_record));
    /* the key should match the artifact ID. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.key, foo_artifact, 16));
    /* the first transaction should be the foo transaction. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.txn_first, foo_key, 16));
    /* the latest transaction should be the foo transaction. */
    TEST_ASSERT(0 == memcmp(foo_artifact_record.txn_latest, foo_key, 16));
    /* the first height for this artifact should be 1. */
    TEST_ASSERT(1U == ntohll(foo_artifact_record.net_height_first));
    /* the latest height for this artifact should be 1. */
    TEST_ASSERT(1U == ntohll(foo_artifact_record.net_height_latest));

    /* verify that if we try to get the root block id, we get nothing. */
    TEST_ASSERT(
        AGENTD_STATUS_SUCCESS
            == dataservice_block_get(
                    &child, nullptr, vccert_certificate_type_uuid_root_block,
                    &block_node, &root_block_txn_bytes, &root_block_txn_size));

    /* the next value should be our block. */
    TEST_EXPECT(0 == memcmp(foo_block_id, block_node.next, 16));
    TEST_ASSERT(nullptr != root_block_txn_bytes);
    TEST_ASSERT(0U == root_block_txn_size);

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
    free(block_txn_bytes);
END_TEST_F()
