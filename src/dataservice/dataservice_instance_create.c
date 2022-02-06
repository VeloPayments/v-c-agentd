/**
 * \file dataservice/dataservice_instance_create.c
 *
 * \brief Create a dataservice instance.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* forward decls */
static void dataservice_instance_dispose(void* disposable);

/**
 * \brief Create the dataservice instance.
 *
 * \returns a properly created dataservice instance, or NULL on failure.
 */
dataservice_instance_t* dataservice_instance_create()
{
    /* allocate memory for the instance. */
    dataservice_instance_t* instance =
        (dataservice_instance_t*)malloc(sizeof(dataservice_instance_t));
    if (NULL == instance)
        return NULL;

    /* clear the instance. */
    memset(instance, 0, sizeof(dataservice_instance_t));

    /* create a malloc allocator for this instance. */
    malloc_allocator_options_init(&instance->alloc_opts);

    /* explicitly allow the root context to be created. */
    BITCAP_SET_TRUE(
        instance->ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* set the dispose method. */
    instance->hdr.dispose = &dataservice_instance_dispose;

    /* for each child, add the child to the child_head. */
    for (size_t i = 0; i < DATASERVICE_MAX_CHILD_CONTEXTS; ++i)
    {
        instance->children[i].next = instance->child_head;
        instance->child_head = &instance->children[i];
    }

    /* success. */
    return instance;
}

/**
 * \brief Dispose of a dataservice instance.
 *
 * \param disposable        The instance to dispose.
 */
static void dataservice_instance_dispose(void* disposable)
{
    dataservice_instance_t* instance = (dataservice_instance_t*)disposable;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* dispose any children that need to be disposed. */
    for (size_t i = 0; i < DATASERVICE_MAX_CHILD_CONTEXTS; ++i)
    {
        if (instance->children[i].hdr.dispose)
            dispose((disposable_t*)&instance->children[i]);
    }

    /* if the root context hasn't been disposed, dispose it. */
    if (instance->ctx.hdr.dispose)
        dispose((disposable_t*)&instance->ctx);

    /* dispose the allocator options instance. */
    dispose((disposable_t*)&instance->alloc_opts);

    /* clear the data structure. */
    memset(instance, 0, sizeof(dataservice_instance_t));
}
