/**
 * \file protocolservice/unauthorized_protocol_service_instance_init.c
 *
 * \brief Initialize the unauthorized protocol service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls */
static void unauthorized_protocol_service_instance_dispose(void* disposable);

/**
 * \brief Create the unauthorized protocol service instance.
 *
 * \param inst          The service instance to initialize.
 * \param random        The random socket to use for this instance.
 * \param control       The control socket to use for this instance.
 * \param data          The dataservice socket to use for this instance.
 * \param proto         The protocol socket to use for this instance.
 * \param max_socks     The maximum number of socket connections to accept.
 *
 * \returns a status code indicating success or failure.
 */
int unauthorized_protocol_service_instance_init(
    unauthorized_protocol_service_instance_t* inst, int random, int control,
    int data, int proto, size_t max_socks)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(proto >= 0);
    MODEL_ASSERT(random >= 0);
    MODEL_ASSERT(control >= 0);
    MODEL_ASSERT(data >= 0);
    MODEL_ASSERT(max_socks > 0);

    /* Set up the instance basics. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
    inst->hdr.dispose = &unauthorized_protocol_service_instance_dispose;

    /* create the allocator for this instance. */
    malloc_allocator_options_init(&inst->alloc_opts);

    /* create the crypto suite for this instance. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_options_init(
            &inst->suite, &inst->alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto done;
    }

    /* set the protocol socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(proto, &inst->proto, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_suite;
    }

    /* set the random socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(random, &inst->random, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_proto;
    }

    /* set the control socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(control, &inst->control, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_random;
    }

    /* set the data socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(data, &inst->data, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_control;
    }

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&inst->loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_data;
    }

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&inst->loop, SIGHUP);
    ipc_exit_loop_on_signal(&inst->loop, SIGTERM);
    ipc_exit_loop_on_signal(&inst->loop, SIGQUIT);

    /* create a single dynamic array and size for all connections so that
     * we can reference them by offset in constant time.
     */
    inst->num_connections = max_socks;
    inst->connections = (unauthorized_protocol_connection_t*)
        malloc(max_socks * sizeof(unauthorized_protocol_connection_t));
    if (NULL == inst->connections)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_loop;
    }

    /* clear all connections */
    memset(
        inst->connections, 0,
        max_socks * sizeof(unauthorized_protocol_connection_t));

    /* move connections to free list. */
    for (size_t i = 0; i < max_socks; ++i)
    {
        /* add this connection to the free list. */
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, inst->connections + i);
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

    /*cleanup_connections:
    free(inst->connections); */

cleanup_loop:
    dispose((disposable_t*)&inst->loop);

cleanup_data:
    dispose((disposable_t*)&inst->data);

cleanup_control:
    dispose((disposable_t*)&inst->control);

cleanup_random:
    dispose((disposable_t*)&inst->random);

cleanup_proto:
    dispose((disposable_t*)&inst->proto);

cleanup_suite:
    dispose((disposable_t*)&inst->suite);

done:
    return retval;
}

/**
 * \brief Dispose of an unauthorized protocol service instance.
 */
static void unauthorized_protocol_service_instance_dispose(void* disposable)
{
    unauthorized_protocol_service_instance_t* inst =
        (unauthorized_protocol_service_instance_t*)disposable;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);

    /* dispose of connections waiting for a free dataservice context. */
    for (unauthorized_protocol_connection_t* i =
             inst->dataservice_context_create_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        dispose((disposable_t*)i);
        i = next;
    }

    /* dispose of used conections. */
    for (unauthorized_protocol_connection_t* i = inst->used_connection_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        dispose((disposable_t*)i);
        i = next;
    }

    /* free connection array. */
    memset(
        inst->connections, 0,
        inst->num_connections * sizeof(unauthorized_protocol_connection_t));
    free(inst->connections);

    /* dispose of authorized entities. */
    while (NULL != inst->entity_head)
    {
        ups_authorized_entity_t* tmp = inst->entity_head->next;
        inst->entity_head->next = NULL;
        dispose((disposable_t*)inst->entity_head);
        free(inst->entity_head);
        inst->entity_head = tmp;
    }

    /* dispose of private key if set. */
    if (NULL != inst->private_key)
    {
        dispose((disposable_t*)inst->private_key);
        free(inst->private_key);
    }

    /* dispose of the proto socket. */
    dispose((disposable_t*)&inst->proto);

    /* dispose of the random socket. */
    dispose((disposable_t*)&inst->random);

    /* dispose of the control socket. */
    dispose((disposable_t*)&inst->control);

    /* dispose of the data socket. */
    dispose((disposable_t*)&inst->data);

    /* dispose of the loop. */
    dispose((disposable_t*)&inst->loop);

    /* dispose of the crypto suite. */
    dispose((disposable_t*)&inst->suite);

    /* dispose of the allocator. */
    dispose((disposable_t*)&inst->alloc_opts);

    /* clear this instance. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
}
