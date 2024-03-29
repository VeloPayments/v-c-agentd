/**
 * \file agentd/services.h
 *
 * \brief Service enumeration.
 *
 * \copyright 2018-2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_SERVICES_HEADER_GUARD
#define AGENTD_SERVICES_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Services running in agentd.
 */
enum agentd_services
{
    /**
     * \brief General Service; used for code that does not belong to a
     * particular service.
     */
    AGENTD_SERVICE_GENERAL = 0x00U,

    /**
     * \brief Inter-process Communication pseudo-service.
     */
    AGENTD_SERVICE_IPC = 0x01U,

    /**
     * \brief Supervisor Service.
     */
    AGENTD_SERVICE_SUPERVISOR = 0x02U,

    /**
     * \brief Data Service.
     */
    AGENTD_SERVICE_DATASERVICE = 0x03U,

    /**
     * \brief Config Service.
     */
    AGENTD_SERVICE_CONFIG = 0x04U,

    /**
     * \brief Auth Service.
     */
    AGENTD_SERVICE_AUTHSERVICE = 0x05U,

    /**
     * \brief Log Service.
     */
    AGENTD_SERVICE_LOG = 0x06U,

    /**
     * \brief Canonization Service.
     */
    AGENTD_SERVICE_CANONIZATION = 0x07U,

    /**
     * \brief Application Service.
     */
    AGENTD_SERVICE_APPLICATION = 0x08U,

    /**
     * \brief Process pseudo-service.
     */
    AGENTD_SERVICE_PROCESS = 0x09U,
    /**
     * \brief Protocol Service.
     */
    AGENTD_SERVICE_PROTOCOL = 0x0AU,

    /**
     * \brief Listen Service.
     */
    AGENTD_SERVICE_LISTEN = 0x0BU,

    /**
     * \brief Random Service.
     */
    AGENTD_SERVICE_RANDOM = 0x0CU,

    /**
     * \brief Reader Service.
     */
    AGENTD_SERVICE_READER = 0x0DU,

    /**
     * \brief Attestation Service.
     */
    AGENTD_SERVICE_ATTESTATION = 0x0EU,

    /**
     * \brief Notification Service.
     */
    AGENTD_SERVICE_NOTIFICATION = 0x0FU,
};

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_SERVICES_HEADER_GUARD*/
