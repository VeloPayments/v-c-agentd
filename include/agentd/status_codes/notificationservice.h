/**
 * \file agentd/status_codes/notificationservice.h
 *
 * \brief Status code definitions for the notification service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_NOTIFICATIONSERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_NOTIFICATIONSERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0001U)

/**
 * \brief Forking the notification service process failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0002U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0003U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0004U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0005U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0006U)

/**
 * \brief Attempting to execute the notification service private command failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0007U)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0008U)

/**
 * \brief Attempt to close other fds failed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_PRIVSEP_CLOSE_OTHER_FDS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x0009U)

/**
 * \brief A bad argument was given to an API method.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x000AU)

/**
 * \brief An invalid API request was attempted.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_INVALID_REQUEST_ID \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x000BU)

/**
 * \brief The request is not authorized.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x000CU)

/**
 * \brief The request is malformed.
 */
#define AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_NOTIFICATION, 0x000DU)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_NOTIFICATIONSERVICE_HEADER_GUARD*/
