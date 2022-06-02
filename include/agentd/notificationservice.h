/**
 * \file agentd/notificationservice.h
 *
 * \brief Service level API for the notification service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_NOTIFICATIONSERVICE_HEADER_GUARD
#define AGENTD_NOTIFICATIONSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/protocolservice/api.h>
#include <config.h>
#include <rcpr/status.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Main entry point for the notification service.  It handles the details
 * of reacting to events sent over the protocol service socket.
 *
 * \param logsock       The socket to the logging service service.
 * \param consensussock Socket connection to the consensus service.
 * \param protocolsock  Socket connection to the protocol service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - a non-zero error code on failure.
 */
status notificationservice_run(
    int logsock, int consensussock, int protocolsock);

/**
 * \brief Spawn a notification service process using the provided config
 * structure and logger socket.
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
 * \param consensussock Socket connection to the consensus service.
 * \param protocolsock  Socket connection to the protocol service.
 * \param pid           Pointer to the notification service pid, to be updated
 *                      on the successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int notificationservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int logsock,
    int consensussock, int protocolsock, pid_t* pid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_NOTIFICATIONSERVICE_HEADER_GUARD*/
