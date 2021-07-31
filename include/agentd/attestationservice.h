/**
 * \file agentd/attestationservice.h
 *
 * \brief Service level API for the attestation service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_ATTESTATIONSERVICE_HEADER_GUARD
#define AGENTD_ATTESTATIONSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/protocolservice/api.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Entry point for the attestation service.
 *
 * \param datasock      The data service socket.  The attestation service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The attestation service
 *                      logs on this socket.
 * \param controlsock   The socket used to control the attestation service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - a non-zero error code on failure.
 */
int attestationservice_entry_point(
    int datasock, int logsock, int controlsock);

/**
 * \brief Spawn an attestation service process using the provided config
 * structure and logger socket.
 *
 * On success, the pointer to the pid for this process is set.  This can be used
 * to signal and wait when this process should be terminated.
 *
 * \param bconf             The bootstrap configuration for this service.
 * \param conf              The configuration for this service.
 * \param logsock           Pointer to the socket used to communicate with the
 *                          logger.
 * \param datasock          Pointer to the socket used to communicate with the
 *                          data service.
 * \param controlsock       Pointer to the socket used to control the
 *                          attestation service.
 * \param attestationpid    Pointer to the attestation service pid, to be
 *                          updated on the successful completion of this
 *                          function.
 * \param runsecure         Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the protocol service process failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if
 *        there was a failure looking up the configured user and group for the
 *        protocol service process.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_SETFDS_FAILURE if setting
 *        file descriptors failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if
 *        executing the private command failed.
 *      - AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if
 *        the process survived execution (weird!).
 */
int start_attestationservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int* logsock,
    int* datasock, int* controlsock, pid_t* attestationpid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_ATTESTATIONSERVICE_HEADER_GUARD*/
