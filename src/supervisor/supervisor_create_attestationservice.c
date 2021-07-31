/**
 * \file supervisor/supervisor_create_attestationservice.c
 *
 * \brief Create the attestion service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/attestationservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <vpr/allocator/malloc_allocator.h>

#include "supervisor_private.h"

/**
 * \brief attestion service process structure.
 */
typedef struct attestation_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    const config_private_key_t* private_key;
    int* data_socket;
    int* log_socket;
    int control_socket;
    /* do not close the srv socket. */
    int control_srv_socket;
} attestation_process_t;

/* forward decls. */
static void supervisor_dispose_attestationservice(void* disposable);
static int supervisor_start_attestationservice(process_t* proc);

/**
 * \brief Create the attestation service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the attestation service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              attestation service.  This configuration must
 *                              be valid for the lifetime of the service.
 * \param private_key           The private key for this service.
 * \param data_socket           The data socket descriptor.
 * \param log_socket            The log socket descriptor.
 * \param control_socket        The control socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_attestationservice(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, config_private_key_t* private_key,
    int* data_socket, int* log_socket, int* control_socket)
{
    int retval;

    /* allocate memory for the attestation process. */
    attestation_process_t* attestation_proc =
        (attestation_process_t*)malloc(sizeof(attestation_process_t));
    if (NULL == attestation_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up attestation_proc structure. */
    memset(attestation_proc, 0, sizeof(attestation_process_t));
    attestation_proc->hdr.hdr.dispose =
        &supervisor_dispose_attestationservice;
    attestation_proc->hdr.init_method = &supervisor_start_attestationservice;
    attestation_proc->bconf = bconf;
    attestation_proc->conf = conf;
    attestation_proc->private_key = private_key;
    attestation_proc->data_socket = data_socket;
    attestation_proc->log_socket = log_socket;

    /* create the socketpair for the control socket. */
    retval =
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0, control_socket,
            &attestation_proc->control_socket);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_attestation_proc;
    }

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    attestation_proc->control_srv_socket = *control_socket;
    *svc = (process_t*)attestation_proc;
    goto done;

cleanup_attestation_proc:
    memset(attestation_proc, 0, sizeof(attestation_process_t));
    free(attestation_proc);

done:
    return retval;
}

/**
 * \brief Start the attestation service.
 *
 * \param proc      The attestation service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_attestationservice(process_t* proc)
{
    attestation_process_t* attestation_proc = (attestation_process_t*)proc;
    int retval;
    allocator_options_t alloc_opts;

    /* create an allocator instance. */
    malloc_allocator_options_init(&alloc_opts);

    /* attempt to create the attestation service. */
    TRY_OR_FAIL(
        start_attestationservice_proc(
            attestation_proc->bconf, attestation_proc->conf,
            attestation_proc->log_socket, attestation_proc->data_socket,
            &attestation_proc->control_socket,
            &attestation_proc->hdr.process_id,
            true),
        done);

    /* TODO - send configuration data to the attestion process. */

    /* TODO - add supervisor configuration steps here. */

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

#if 0
terminate_proc:
    /* force the running status to true so we can terminate the process. */
    attestation_proc->hdr.running = true;
    process_stop((process_t*)attestation_proc);
    sleep(5);
    process_kill((process_t*)attestation_proc);
#endif

done:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}

/**
 * \brief Dispose of the attestation service by cleaning up.
 */
static void supervisor_dispose_attestationservice(void* disposable)
{
    attestation_process_t* attestation_proc =
        (attestation_process_t*)disposable;

    /* clean up the log socket if valid. */
    if (*attestation_proc->log_socket > 0)
    {
        close(*attestation_proc->log_socket);
        *attestation_proc->log_socket = -1;
    }

    /* clean up the data socket if valid. */
    if (*attestation_proc->data_socket > 0)
    {
        close(*attestation_proc->data_socket);
        *attestation_proc->data_socket = -1;
    }

    /* clean up the control socket if valid. */
    if (attestation_proc->control_socket > 0)
    {
        close(attestation_proc->control_socket);
        attestation_proc->control_socket = -1;
    }

    if (attestation_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)attestation_proc);

        sleep(5);

        /* kill the proc. */
        process_kill((process_t*)attestation_proc);
    }
}
