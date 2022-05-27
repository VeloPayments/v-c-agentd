/**
 * \file agentd/fds.h
 *
 * \brief File descriptors for agentd.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_FDS_CONFIG_HEADER_GUARD
#define AGENTD_FDS_CONFIG_HEADER_GUARD

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/******************************************************************************/
/* Config Reader                                                              */
/******************************************************************************/

/**
 * \brief File descriptor for the config input file.
 * Used by the readconfig private command.
 */
#define AGENTD_FD_CONFIG_IN ((int)0)

/**
 * \brief File descriptor for the config output stream.
 * Used by the readconfig private command.
 */
#define AGENTD_FD_CONFIG_OUT ((int)1)

/******************************************************************************/
/* Reader Processes                                                           */
/******************************************************************************/

/**
 * \brief Control socket for a reader process.
 */
#define AGENTD_FD_READER_CONTROL ((int)0)

/******************************************************************************/
/* Data Service                                                               */
/******************************************************************************/

/**
 * \brief File descriptor for the data service socket.
 * Used by the data service private command.
 */
#define AGENTD_FD_DATASERVICE_SOCK ((int)0)

/**
 * \brief File descriptor for the data service log socket.
 * Used by the data service private command.
 */
#define AGENTD_FD_DATASERVICE_LOG ((int)1)

/******************************************************************************/
/* Listen Service                                                             */
/******************************************************************************/

/**
 * \brief File descriptor for the listen service log socket.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_LOG ((int)0)

/**
 * \brief File descriptor for the listen service accept socket.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_ACCEPT ((int)1)

/**
 * \brief File descriptor for the first listen socket for the listen service.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_SOCK_START ((int)2)

/******************************************************************************/
/* Supervisor Service                                                         */
/******************************************************************************/

/**
 * \brief File descriptor for the pid flocked file.
 * Used by the supervisor private command.
 */
#define AGENTD_FD_PID ((int)2)

/******************************************************************************/
/* Authentication Service                                                     */
/******************************************************************************/

/**
 * \brief File descriptor for the auth service socket.
 * Used by the auth service private command.
 */
#define AGENTD_FD_AUTHSERVICE_SOCK ((int)0)

/**
 * \brief File descriptor for the auth service log socket.
 * Used by the auth service private command.
 */
#define AGENTD_FD_AUTHSERVICE_LOG ((int)1)


/******************************************************************************/
/* Unauthorized Protocol Service                                              */
/******************************************************************************/

/**
 * \brief File descriptor for the unauthorized protocol service accept socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_ACCEPT ((int)0)

/**
 * \brief File descriptor for the unauthorized protocol service log socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG ((int)1)

/**
 * \brief File descriptor for the unauthorized protocol service data socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_DATA ((int)2)

/**
 * \brief File descriptor for the unauthorized protocol service random socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_RANDOM ((int)3)

/**
 * \brief File descriptor for the unauthorized protocol service control socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_CONTROL ((int)4)

/******************************************************************************/
/* Random Service                                                             */
/******************************************************************************/

/**
 * \brief File descriptor for the random device.
 * Used by the random service private command.
 */
#define AGENTD_FD_RANDOM_SERVICE_RANDOM_DEVICE ((int)0)

/**
 * \brief File descriptor for the shared socket between the protocol service and
 * the random service.
 * Used by the random service private command.
 */
#define AGENTD_FD_RANDOM_SERVICE_PROTOCOL_SERVICE ((int)1)

/**
 * \brief File descriptor for the random service log socket.
 * Used by the random service private command.
 */
#define AGENTD_FD_RANDOM_SERVICE_LOG_SOCKET ((int)2)

/******************************************************************************/
/* Canonization Service                                                       */
/******************************************************************************/

/**
 * \brief File descriptor for the canonization service log socket.
 * Used by the canonization service private command.
 */
#define AGENTD_FD_CANONIZATION_SVC_LOG ((int)0)

/**
 * \brief File descriptor for the canonization service data socket.
 * Used by the canonization service private command.
 */
#define AGENTD_FD_CANONIZATION_SVC_DATA ((int)1)

/**
 * \brief File descriptor for the canonization service random socket.
 * Used by the canonization service private command.
 */
#define AGENTD_FD_CANONIZATION_SVC_RANDOM ((int)2)

/**
 * \brief File descriptor for the canonization service control socket.
 * Used by the canonization service private command.
 */
#define AGENTD_FD_CANONIZATION_SVC_CONTROL ((int)3)

/******************************************************************************/
/* Attestation Service                                                        */
/******************************************************************************/

/**
 * \brief File descriptor for the attestation service log socket.
 * Used by the attestation service private command.
 */
#define AGENTD_FD_ATTESTATION_SVC_LOG ((int)0)

/**
 * \brief File descriptor for the attestation service data socket.
 * Used by the canonization service private command.
 */
#define AGENTD_FD_ATTESTATION_SVC_DATA ((int)1)

/**
 * \brief File descriptor for the attestation service control socket.
 * Used by the attestation service private command.
 */
#define AGENTD_FD_ATTESTATION_SVC_CONTROL ((int)2)

/******************************************************************************/
/* Notification Service                                                       */
/******************************************************************************/

/**
 * \brief File descriptor for the notification service log socket.
 * Used by the notification service private command.
 */
#define AGENTD_FD_NOTIFICATION_SVC_LOG ((int)0)

/**
 * \brief File descriptor for the first notification service client socket.
 * Used by the canonization service.
 */
#define AGENTD_FD_NOTIFICATION_SVC_CLIENT1 ((int)1)

/**
 * \brief File descriptor for the second notification service client socket.
 * Used by the protocol service.
 */
#define AGENTD_FD_NOTIFICATION_SVC_CLIENT2 ((int)2)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_FDS_CONFIG_HEADER_GUARD*/
