/**
 * \file supervisor/supervisor_create_notification_service.c
 *
 * \brief Create the notification service as a process that can be started.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/notificationservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

/**
 * \brief Notification service process structure.
 */
typedef struct notification_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int log_socket;
    int consensus_socket;
    int protocol_socket;
} notification_process_t;

/* forward decls. */
static void supervisor_dispose_notification_service(void* disposable);
static int supervisor_start_notification_service(process_t* proc);

/**
 * \brief Create the notification service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the notification service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              protocol service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param log_socket            The log socket descriptor.
 * \param consensus_socket      The socket pointer to receive the socket for the
 *                              consensus service.
 * \param protocol_socket       The socket pointer to receive the socket for the
 *                              protocol service.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_notification_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* log_socket, int* consensus_socket,
    int* protocol_socket)
{
    int retval;

    /* allocate memory for the notification process. */
    notification_process_t* notification_proc =
        (notification_process_t*)malloc(sizeof(notification_process_t));
    if (NULL == notification_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up the structure. */
    memset(notification_proc, 0, sizeof(notification_process_t));
    notification_proc->hdr.hdr.dispose =
        &supervisor_dispose_notification_service;
    notification_proc->hdr.init_method = &supervisor_start_notification_service;
    notification_proc->bconf = bconf;
    notification_proc->conf = conf;
    notification_proc->log_socket = -1;
    notification_proc->consensus_socket = -1;
    notification_proc->protocol_socket = -1;

    /* create the socketpair for the consennus socket. */
    retval =
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0, consensus_socket,
            &notification_proc->consensus_socket);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_notification_proc;
    }

    /* create the socketpair for the protocol socket. */
    retval =
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0, protocol_socket,
            &notification_proc->protocol_socket);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_notification_proc;
    }

    /* take ownership of the log socket. */
    notification_proc->log_socket = *log_socket; *log_socket = -1;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)notification_proc;
    goto done;

cleanup_notification_proc:
    dispose((disposable_t*)notification_proc);
    free(notification_proc);

done:
    return retval;
}

/**
 * \brief Dispose of a notification process instance.
 *
 * \param disposable        The instance to dispose.
 */
static void supervisor_dispose_notification_service(void* disposable)
{
    notification_process_t* notification_proc =
        (notification_process_t*)disposable;

    /* close the log socket, if open. */
    if (-1 != notification_proc->log_socket)
    {
        close(notification_proc->log_socket);
    }

    /* close the consensus socket, if open. */
    if (-1 != notification_proc->consensus_socket)
    {
        close(notification_proc->consensus_socket);
    }

    /* close the protocol socket, if open. */
    if (-1 != notification_proc->protocol_socket)
    {
        close(notification_proc->protocol_socket);
    }

    /* clear out structure. */
    memset(notification_proc, 0, sizeof(*notification_proc));
}

/**
 * \brief Start the notification service.
 *
 * \param proc      The notification service to start.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int supervisor_start_notification_service(process_t* proc)
{
    notification_process_t* notification_proc = (notification_process_t*)proc;
    int retval;

    /* attempt to create the notification service. */
    TRY_OR_FAIL(
        notificationservice_proc(
            notification_proc->bconf, notification_proc->conf,
            notification_proc->log_socket, notification_proc->consensus_socket,
            notification_proc->protocol_socket,
            &notification_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    notification_proc->log_socket = -1;
    notification_proc->consensus_socket = -1;
    notification_proc->protocol_socket = -1;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
