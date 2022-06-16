/**
 * \file agentd/protocolservice.h
 *
 * \brief Service level API for the protocol service.
 *
 * \copyright 2019-2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PROTOCOLSERVICE_HEADER_GUARD
#define AGENTD_PROTOCOLSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/protocolservice/api.h>
#include <config.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Protocol service API methods.
 */
enum protocolservice_api_method_enum
{
    /**
     * \brief Lower bound of the API methods.  Must be the first value in this
     * enumeration.
     */
    PROTOCOLSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Get current connection status.
     */
    PROTOCOLSERVICE_AUTH_API_METHOD_CONNECTION_STATUS_GET,

    /**
     * \brief The number of methods in this API.
     *
     * Must be immediately after the last enumerated bit value.
     */
    PROTOCOLSERVICE_API_METHOD_UPPER_BOUND
};

/**
 * \brief Main entry point for the protocol service.  It handles the details of
 * reacting to events sent over the protocol service socket.
 *
 * \param randomsock    The socket to the RNG service.
 * \param protosock     The protocol service socket.  The protocol service
 *                      listens for connections on this socket.
 * \param controlsock   The control socket. The supervisor sends commands to the
 *                      protocol service over this socket.
 * \param datasock      The data service socket.  The protocol service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 * \param notifysock    The notification service socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *          attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the protocol service event loop failed.
 */
int protocolservice_run(
    int randomsock, int protosock, int controlsock, int datasock, int logsock,
    int notifysock);

/**
 * \brief Spawn an unauthorized protocol service process using the provided
 * config structure and logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptor for the protocl service socket.  This can be used by the caller to
 * send requests to the protocol service and to receive responses from this
 * service. Also, the pointer to the pid for this process is set.  This can be
 * used to signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param randomsock    Socket used to communicate with the random service.
 * \param logsock       Socket used to communicate with the logger.
 * \param acceptsock    Socket used to receive accepted peers.
 * \param controlsock   Socket used to send commands to this service.
 * \param datasock      Socket used to communicate with the data service.
 * \param notifysock    Socket used to communicate with the notification
 *                      service.
 * \param protopid      Pointer to the protocol service pid, to be updated on
 *                      the successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the protocol service process failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there
 *        was a failure looking up the configured user and group for the
 *        protocol service process.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_SETFDS_FAILURE if setting file
 *        descriptors failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if executing
 *        the private command failed.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the
 *        process survived execution (weird!).      
 */
int protocolservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int randomsock,
    int logsock, int acceptsock, int controlsock, int datasock, int notifysock,
    pid_t* protopid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PROTOCOLSERVICE_HEADER_GUARD*/
