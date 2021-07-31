/**
 * \file agentd/status_codes/canonization.h
 *
 * \brief Status code definitions for the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_ATTESTATION_HEADER_GUARD
#define AGENTD_STATUS_CODES_ATTESTATION_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0001U)

/**
 * \brief Forking the canonization service process failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0002U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0003U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0004U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0005U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0006U)

/**
 * \brief Attempt to close other FDs failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_CLOSE_OTHER_FDS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0007U)

/**
 * \brief Attempting to execute the canonization service private command failed.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0008U)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_ATTESTATIONSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_ATTESTATION, 0x0009U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_ATTESTATION_HEADER_GUARD*/
