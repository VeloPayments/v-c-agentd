/**
 * \file protocolservice/protocolservice_run.c
 *
 * \brief The main entry point for the protocol service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#if defined(AGENTD_NEW_PROTOCOL)

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
    int UNUSED(randomsock), int UNUSED(protosock), int UNUSED(controlsock),
    int UNUSED(datasock), int UNUSED(logsock))
{
    return -1;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
