/**
 * \file agentd/status_codes/reader.h
 *
 * \brief Status code definitions for reader.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_READER_HEADER_GUARD
#define AGENTD_STATUS_CODES_READER_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Error writing data to the socket.
 */
#define AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0000U)

/**
 * \brief Error reading data from the socket.
 */
#define AGENTD_ERROR_READER_IPC_READ_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0001U)

/**
 * \brief Error converting a network address to string form.
 */
#define AGENTD_ERROR_READER_INET_NTOP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0002U)

/**
 * \brief Error converting a string to a network address.
 */
#define AGENTD_ERROR_READER_INET_PTON_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0003U)

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_READER_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0004U)

/**
 * \brief A socket pair could not be created.
 */
#define AGENTD_ERROR_READER_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0005U)

/**
 * \brief Forking the config process failed.
 */
#define AGENTD_ERROR_READER_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0006U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0007U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0008U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0009U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000AU)

/**
 * \brief Attempting to execute the config private command failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000BU)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_READER_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000CU)

/**
 * \brief The config proc did not properly exit.
 */
#define AGENTD_ERROR_READER_PROC_EXIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000DU)

/**
 * \brief The control stream data was corrupt / invalid.
 */
#define AGENTD_ERROR_READER_INVALID_STREAM \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000EU)

/**
 * \brief Attempt to close other FDs failed.
 */
#define AGENTD_ERROR_READER_PRIVSEP_CLOSE_OTHER_FDS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x000FU)

/**
 * \brief Attempt to open a file failed.
 */
#define AGENTD_ERROR_READER_FILE_OPEN \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0010U)

/**
 * \brief Attempt to stat the file failed.
 */
#define AGENTD_ERROR_READER_FILE_STAT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0011U)

/**
 * \brief Attempt to read the file failed.
 */
#define AGENTD_ERROR_READER_FILE_READ \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_READER, 0x0012U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_READER_HEADER_GUARD*/
