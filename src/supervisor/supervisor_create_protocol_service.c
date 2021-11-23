/**
 * \file supervisor/supervisor_create_protocol_service.c
 *
 * \brief Create the protocol service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/protocolservice.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <vpr/allocator/malloc_allocator.h>

/**
 * \brief Protocol service process structure.
 */
typedef struct protocol_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    config_private_key_t* private_key;
    config_public_entity_node_t* public_entities;
    int* random_socket;
    int* accept_socket;
    int control_socket;
    int* data_socket;
    int* log_socket;
    int control;
} protocol_process_t;

/* forward decls. */
static void supervisor_dispose_protocol_service(void* disposable);
static int supervisor_start_protocol_service(process_t* proc);

/**
 * \brief Create the protocol service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the protocol service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              protocol service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param private_key           The private key for this service.
 * \param public_entities       The public entities authorized to use this
 *                              service.
 * \param random_socket         The random socket descriptor.
 * \param accept_socket         The accept socket descriptor.
 * \param control_socket        The control socket descriptor.
 * \param data_socket           The data socket descriptor.
 * \param log_socket            The log socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_protocol_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, config_private_key_t* private_key,
    config_public_entity_node_t* public_entities, int* random_socket,
    int* accept_socket, int* control_socket, int* data_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the protocol process. */
    protocol_process_t* protocol_proc =
        (protocol_process_t*)malloc(sizeof(protocol_process_t));
    if (NULL == protocol_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up protocol_proc structure. */
    memset(protocol_proc, 0, sizeof(protocol_process_t));
    protocol_proc->hdr.hdr.dispose = &supervisor_dispose_protocol_service;
    protocol_proc->hdr.init_method = &supervisor_start_protocol_service;
    protocol_proc->bconf = bconf;
    protocol_proc->conf = conf;
    protocol_proc->private_key = private_key;
    protocol_proc->public_entities = public_entities;
    protocol_proc->random_socket = random_socket;
    protocol_proc->accept_socket = accept_socket;
    protocol_proc->data_socket = data_socket;
    protocol_proc->log_socket = log_socket;

    /* create the socketpair for the control socket. */
    retval =
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0, control_socket,
            &protocol_proc->control_socket);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_protocol_proc;
    }

    /* save the server side of the control socket to the protocol structure. */
    protocol_proc->control = *control_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)protocol_proc;
    goto done;

cleanup_protocol_proc:
    memset(protocol_proc, 0, sizeof(protocol_process_t));
    free(protocol_proc);

done:
    return retval;
}

/**
 * \brief Start the protocol service.
 *
 * \param proc      The protocol service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_protocol_service(process_t* proc)
{
    allocator_options_t alloc_opts;
    protocol_process_t* protocol_proc = (protocol_process_t*)proc;
    int retval;
    uint32_t offset, status;

    /* create an allocator instance. */
    malloc_allocator_options_init(&alloc_opts);

    /* attempt to create the protocol service. */
    TRY_OR_FAIL(
        protocolservice_proc(
            protocol_proc->bconf, protocol_proc->conf,
            *protocol_proc->random_socket, *protocol_proc->log_socket,
            *protocol_proc->accept_socket, protocol_proc->control_socket,
            *protocol_proc->data_socket, &protocol_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    *protocol_proc->random_socket = -1;
    *protocol_proc->log_socket = -1;
    *protocol_proc->accept_socket = -1;
    protocol_proc->control_socket = -1;
    *protocol_proc->data_socket = -1;

    /* write the private key request to the protocol service control socket. */
    TRY_OR_FAIL(
        protocolservice_control_api_sendreq_private_key_set(
            protocol_proc->control, &alloc_opts, protocol_proc->private_key->id,
            &protocol_proc->private_key->enc_pubkey,
            &protocol_proc->private_key->enc_privkey,
            &protocol_proc->private_key->sign_pubkey,
            &protocol_proc->private_key->sign_privkey),
        done);

    /* receive the private key response from the control sock. */
    TRY_OR_FAIL(
        protocolservice_control_api_recvresp_private_key_set(
            protocol_proc->control, &offset, &status),
        done);

    /* verify the status. */
    TRY_OR_FAIL(status, done);

    /* send the public entities. */
    config_public_entity_node_t* tmp = protocol_proc->public_entities;
    while (NULL != tmp)
    {
        /* send the public entity add request. */
        TRY_OR_FAIL(
            protocolservice_control_api_sendreq_authorized_entity_add(
                protocol_proc->control, &alloc_opts, tmp->id,
                &tmp->enc_pubkey, &tmp->sign_pubkey),
            done);

        /* receive the public entity add response. */
        TRY_OR_FAIL(
            protocolservice_control_api_recvresp_authorized_entity_add(
                protocol_proc->control, &offset, &status),
            done);

        /* verify the status. */
        TRY_OR_FAIL(status, done);

        /* move on to the next public entity. */
        tmp = (config_public_entity_node_t*)tmp->hdr.next;
    }

    /* we're done with the control socket. It's owned by the supervisor. */
    protocol_proc->control = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}

/**
 * \brief Dispose of the data service by cleaning up.
 */
static void supervisor_dispose_protocol_service(void* disposable)
{
    protocol_process_t* protocol_proc = (protocol_process_t*)disposable;

    /* clean up the random socket if valid. */
    if (*protocol_proc->random_socket > 0)
    {
        close(*protocol_proc->random_socket);
        *protocol_proc->random_socket = -1;
    }

    /* clean up the accept socket if valid. */
    if (*protocol_proc->accept_socket > 0)
    {
        close(*protocol_proc->accept_socket);
        *protocol_proc->accept_socket = -1;
    }

    /* clean up the log socket if valid. */
    if (*protocol_proc->log_socket > 0)
    {
        close(*protocol_proc->log_socket);
        *protocol_proc->log_socket = -1;
    }

    /* clean up the control socket if valid. */
    if (protocol_proc->control_socket > 0)
    {
        close(protocol_proc->control_socket);
        protocol_proc->control_socket = -1;
    }

    /* clean up the data socket if valid. */
    if (*protocol_proc->data_socket > 0)
    {
        close(*protocol_proc->data_socket);
        *protocol_proc->data_socket = -1;
    }

    if (protocol_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)protocol_proc);

        sleep(5);

        /* kill the process. */
        process_kill((process_t*)protocol_proc);
    }
}
