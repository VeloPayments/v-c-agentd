/**
 * \file command/private_command_supervisor.c
 *
 * \brief Create, spawn, and introduce all services.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/control.h>
#include <agentd/config.h>
#include <agentd/ipc.h>
#include <agentd/process.h>
#include <agentd/status_codes.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <vpr/parameters.h>

/* forward decls. */
static int supervisor_run();

/**
 * \brief Run the the supervisor.
 */
void private_command_supervisor(bootstrap_config_t* bconf)
{
    /* install the signal handlers. */
    if (AGENTD_STATUS_SUCCESS != supervisor_sighandler_install())
    {
        perror("supervisor_sighandler_install");
        return;
    }

    /* we are in the running state. */
    keep_running = true;

    /* TODO - set the process name. */

    while (keep_running)
    {
        /* if supervisor_run fails, exit. */
        if (AGENTD_STATUS_SUCCESS != supervisor_run(bconf))
        {
            keep_running = false;
        }
    }

    /* uninstall the signal handlers on exit. */
    supervisor_sighandler_uninstall();
}

/**
 * \brief Helper macro for starting a process.
 */
#define START_PROCESS(svc, label) \
    do \
    { \
        retval = process_start(svc); \
        if (AGENTD_STATUS_SUCCESS != retval) \
        { \
            goto label; \
        } \
    } while (0)

/**
 * \brief Helper macro for closing a socket if valid.
 */
#define CLOSE_IF_VALID(sock) \
    if (sock >= 0) \
    { \
        close(sock); \
    }

/**
 * \brief Helper macro to clean up process.
 */
#define CLEANUP_PROCESS(svc) \
    dispose((disposable_t*)svc); \
    free(svc)

/**
 * \brief Run the supervisor.
 *
 * \param bconf         The bootstrap config for the supervisor.
 *
 * This function attempts to bootstrap all child services and then waits until
 * an appropriate signal is detected prior to exiting.
 *
 * The bootstrap process first reads the configuration file and then uses this
 * configuration file to start each child service.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - A non-zero status code on failure.
 */
static int supervisor_run(const bootstrap_config_t* bconf)
{
    int retval = AGENTD_STATUS_SUCCESS;
    agent_config_t conf;
    process_t* random_service;
    process_t* random_for_canonizationservice;
    process_t* listener_service;
    process_t* data_for_auth_protocol_service;
    process_t* data_for_canonizationservice;
    process_t* data_for_attestationservice;
    process_t* notification_service;
    process_t* protocol_service;
    process_t* canonizationservice;
    process_t* attestationservice;
    config_public_entity_node_t* endorser_entity;
    config_public_entity_node_t* public_entities;
    config_private_key_t private_key;

    int random_svc_log_sock = -1;
    int random_svc_log_dummy_sock = -1;
    int random_svc_for_canonization_log_sock = -1;
    int random_svc_for_canonization_log_dummy_sock = -1;
    int listen_svc_log_sock = -1;
    int listen_svc_log_dummy_sock = -1;
    int unauth_protocol_svc_log_sock = -1;
    int unauth_protocol_svc_log_dummy_sock = -1;
    int data_for_auth_protocol_svc_log_sock = -1;
    int data_for_auth_protocol_svc_log_dummy_sock = -1;
    int data_for_canonization_svc_log_sock = -1;
    int data_for_canonization_svc_log_dummy_sock = -1;
    int data_for_attestation_svc_log_sock = -1;
    int data_for_attestation_svc_log_dummy_sock = -1;
    int unauth_protocol_svc_random_sock = -1;
    int unauth_protocol_svc_accept_sock = -1;
    int unauth_protocol_svc_control_sock = -1;
    int auth_protocol_svc_data_sock = -1;
    int canonization_svc_data_sock = -1;
    int canonization_svc_random_sock = -1;
    int canonization_svc_log_sock = -1;
    int canonization_svc_log_dummy_sock = -1;
    int canonization_svc_control_sock = -1;
    int attestation_svc_data_sock = -1;
    int attestation_svc_log_sock = -1;
    int attestation_svc_log_dummy_sock = -1;
    int attestation_svc_control_sock = -1;
    int notification_svc_log_sock = -1;
    int notification_svc_log_dummy_sock = -1;
    int notification_svc_canonization_sock = -1;
    int notification_svc_protocol_sock = -1;

#if AUTHSERVICE
    process_t* auth_service;

    int auth_svc_sock = -1;
    int auth_svc_log_sock = -1;
    int auth_svc_log_dummy_sock = -1;
#endif /*AUTHSERVICE*/

    /* read config. */
    TRY_OR_FAIL(config_read_proc(bconf, &conf), done);

    /* Spawn a process to read the public entities. */
    TRY_OR_FAIL(
        config_read_public_entities_proc(
            bconf, &conf, &endorser_entity, &public_entities),
        cleanup_config);

    /* Spawn a process to read the private key. */
    TRY_OR_FAIL(
        config_read_private_key_proc(bconf, &conf, &private_key),
        cleanup_public_entities);

    /* TODO - replace with log service. */
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &random_svc_log_sock, &random_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &random_svc_for_canonization_log_sock,
            &random_svc_for_canonization_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &listen_svc_log_sock, &listen_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &unauth_protocol_svc_log_sock,
            &unauth_protocol_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &data_for_auth_protocol_svc_log_sock,
            &data_for_auth_protocol_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &data_for_canonization_svc_log_sock,
            &data_for_canonization_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &canonization_svc_log_sock,
            &canonization_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &data_for_attestation_svc_log_sock,
            &data_for_attestation_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &attestation_svc_log_sock,
            &attestation_svc_log_dummy_sock),
        cleanup_private_key);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &notification_svc_log_sock,
            &notification_svc_log_dummy_sock),
        cleanup_private_key);
#if AUTHSERVICE
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &auth_svc_log_sock,
            &auth_svc_log_dummy_sock),
        cleanup_private_key);
#endif /*AUTHSERVICE*/

    /* create random service. */
    TRY_OR_FAIL(
        supervisor_create_random_service(
            &random_service, bconf, &conf, &random_svc_log_sock,
            &unauth_protocol_svc_random_sock),
        cleanup_private_key);

    /* create random service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_random_service(
            &random_for_canonizationservice, bconf, &conf,
            &random_svc_for_canonization_log_sock,
            &canonization_svc_random_sock),
        cleanup_random_service);

    /* create listener service. */
    TRY_OR_FAIL(
        supervisor_create_listener_service(
            &listener_service, bconf, &conf, &unauth_protocol_svc_accept_sock,
            &listen_svc_log_sock),
        cleanup_random_for_canonizationservice);

    /* create data service for protocol service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_auth_protocol_service(
            &data_for_auth_protocol_service, bconf, &conf,
            &auth_protocol_svc_data_sock, &data_for_auth_protocol_svc_log_sock),
        cleanup_listener_service);

    /* create the notification service. */
    TRY_OR_FAIL(
        supervisor_create_notification_service(
            &notification_service, bconf, &conf, &notification_svc_log_sock,
            &notification_svc_canonization_sock,
            &notification_svc_protocol_sock),
        cleanup_data_for_auth_protocol_service);

    /* create protocol service. */
    TRY_OR_FAIL(
        supervisor_create_protocol_service(
            &protocol_service, bconf, &conf, &private_key, public_entities,
            &unauth_protocol_svc_random_sock, &unauth_protocol_svc_accept_sock,
            &unauth_protocol_svc_control_sock, &auth_protocol_svc_data_sock,
            &unauth_protocol_svc_log_sock),
        cleanup_notification_service);

#if AUTHSERVICE
    /* create auth service */
    TRY_OR_FAIL(
        supervisor_create_auth_service(
            &auth_service, bconf, &conf, &auth_svc_sock,
            &auth_svc_log_sock),
        cleanup_protocol_service);

    /* create data service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_canonizationservice(
            &data_for_canonizationservice, bconf, &conf,
            &canonization_svc_data_sock, &data_for_canonization_svc_log_sock),
        cleanup_auth_service);
#else
    /* create data service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_canonizationservice(
            &data_for_canonizationservice, bconf, &conf,
            &canonization_svc_data_sock, &data_for_canonization_svc_log_sock),
        cleanup_protocol_service);
#endif /*AUTHSERVICE*/

    /* create canonization service. */
    TRY_OR_FAIL(
        supervisor_create_canonizationservice(
            &canonizationservice, bconf, &conf, &private_key,
            &canonization_svc_data_sock, &canonization_svc_random_sock,
            &canonization_svc_log_sock, &canonization_svc_control_sock,
            notification_svc_canonization_sock),
        cleanup_data_service_for_canonizationservice);

    /* create data service for attestation service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_attestationservice(
            &data_for_attestationservice, bconf, &conf,
            &attestation_svc_data_sock, &data_for_attestation_svc_log_sock),
        cleanup_canonizationservice);

    /* create attestation service. */
    TRY_OR_FAIL(
        supervisor_create_attestationservice(
            &attestationservice, bconf, &conf, &private_key,
            &attestation_svc_data_sock, &attestation_svc_log_sock,
            &attestation_svc_control_sock),
        cleanup_data_service_for_attestationservice);

    /* if we've made it this far, attempt to start each service. */
    START_PROCESS(random_service, cleanup_attestationservice);
    START_PROCESS(random_for_canonizationservice, cleanup_attestationservice);
    START_PROCESS(data_for_canonizationservice, cleanup_attestationservice);
    START_PROCESS(data_for_attestationservice, quiesce_data_processes);
    START_PROCESS(data_for_auth_protocol_service, quiesce_data_processes);
    START_PROCESS(listener_service, quiesce_data_processes);
    START_PROCESS(notification_service, quiesce_data_processes);

#if AUTHSERVICE
    START_PROCESS(auth_service, quiesce_data_processes);
#endif /*AUTHSERVICE*/
    START_PROCESS(protocol_service, quiesce_data_processes);
    START_PROCESS(canonizationservice, quiesce_data_processes);
    START_PROCESS(attestationservice, quiesce_data_processes);

    /* wait until we get a signal, and then restart / terminate. */
    supervisor_sighandler_wait();

    /* wait before shutting everything down. */
    sleep(5);

    /* quiesce the higher-level processes first. */
#if AUTHSERVICE
    process_stop(auth_service);
#endif /*AUTHSERVICE*/
    process_stop(listener_service);
    process_stop(protocol_service);
    process_stop(canonizationservice);
    process_stop(attestationservice);
    process_stop(notification_service);

    /* wait an additional 2 seconds. */
    sleep(2);

    process_stop_ex(random_for_canonizationservice, 0);
    process_stop_ex(random_service, 0);

    /* kill these processes. */
#if AUTHSERVICE
    process_kill(auth_service);
#endif /*AUTHSERVICE*/
    process_kill(listener_service);
    process_kill(protocol_service);
    process_kill(canonizationservice);
    process_kill(attestationservice);
    process_kill(notification_service);

quiesce_data_processes:
    process_stop_ex(data_for_canonizationservice, 0);
    process_stop_ex(data_for_auth_protocol_service, 0);
    process_stop_ex(data_for_attestationservice, 0);

cleanup_attestationservice:
    CLEANUP_PROCESS(attestationservice);

cleanup_data_service_for_attestationservice:
    CLEANUP_PROCESS(data_for_attestationservice);

cleanup_canonizationservice:
    CLEANUP_PROCESS(canonizationservice);

cleanup_data_service_for_canonizationservice:
    CLEANUP_PROCESS(data_for_canonizationservice);

#if AUTHSERVICE
cleanup_auth_service:
    CLEANUP_PROCESS(auth_service);
#endif /*AUTHSERVICE*/

cleanup_protocol_service:
    CLEANUP_PROCESS(protocol_service);

cleanup_notification_service:
    CLEANUP_PROCESS(notification_service);

cleanup_data_for_auth_protocol_service:
    CLEANUP_PROCESS(data_for_auth_protocol_service);

cleanup_listener_service:
    CLEANUP_PROCESS(listener_service);

cleanup_random_for_canonizationservice:
    CLEANUP_PROCESS(random_for_canonizationservice);

cleanup_random_service:
    CLEANUP_PROCESS(random_service);

cleanup_private_key:
    dispose((disposable_t*)&private_key);

cleanup_public_entities:
    while (NULL != public_entities)
    {
        config_public_entity_node_t* tmp =
            (config_public_entity_node_t*)public_entities->hdr.next;

        dispose((disposable_t*)public_entities);
        free(public_entities);
        public_entities = tmp;
    }

cleanup_config:
    dispose((disposable_t*)&conf);

done:
    CLOSE_IF_VALID(random_svc_log_sock);
    CLOSE_IF_VALID(random_svc_log_dummy_sock);
    CLOSE_IF_VALID(random_svc_for_canonization_log_sock);
    CLOSE_IF_VALID(random_svc_for_canonization_log_dummy_sock);
    CLOSE_IF_VALID(listen_svc_log_sock);
    CLOSE_IF_VALID(listen_svc_log_dummy_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_log_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_log_dummy_sock);
    CLOSE_IF_VALID(data_for_auth_protocol_svc_log_sock);
    CLOSE_IF_VALID(data_for_auth_protocol_svc_log_dummy_sock);
    CLOSE_IF_VALID(data_for_canonization_svc_log_sock);
    CLOSE_IF_VALID(data_for_canonization_svc_log_dummy_sock);
    CLOSE_IF_VALID(data_for_attestation_svc_log_sock);
    CLOSE_IF_VALID(data_for_attestation_svc_log_dummy_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_random_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_accept_sock);
    CLOSE_IF_VALID(auth_protocol_svc_data_sock);
    CLOSE_IF_VALID(canonization_svc_data_sock);
    CLOSE_IF_VALID(canonization_svc_random_sock);
    CLOSE_IF_VALID(canonization_svc_log_sock);
    CLOSE_IF_VALID(canonization_svc_log_dummy_sock);
    CLOSE_IF_VALID(canonization_svc_control_sock);
    CLOSE_IF_VALID(attestation_svc_log_dummy_sock);
    CLOSE_IF_VALID(notification_svc_log_sock);
    CLOSE_IF_VALID(notification_svc_log_dummy_sock);
    CLOSE_IF_VALID(notification_svc_canonization_sock);
    CLOSE_IF_VALID(notification_svc_protocol_sock);
    CLOSE_IF_VALID(attestation_svc_control_sock);

#if AUTHSERVICE
    CLOSE_IF_VALID(auth_svc_log_sock);
    CLOSE_IF_VALID(auth_svc_log_dummy_sock);
    CLOSE_IF_VALID(auth_svc_sock);
#endif /*AUTHSERVICE*/

    return retval;
}
