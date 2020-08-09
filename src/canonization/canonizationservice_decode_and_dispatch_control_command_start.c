/**
 * \file
 * canonization/canonizationservice_decode_and_dispatch_control_command_start.c
 *
 * \brief Decode and dispatch the start command.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Decode and dispatch a start request.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int canonizationservice_decode_and_dispatch_control_command_start(
    canonizationservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* UNUSED(req), size_t UNUSED(size))
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* if this instance has not been configured, then it can't be started. */
    if (!instance->configured)
    {
        retval =
            canonizationservice_decode_and_dispatch_write_status(
                sock, CANONIZATIONSERVICE_API_METHOD_START, 0U,
                AGENTD_ERROR_CANONIZATIONSERVICE_START_BEFORE_CONFIGURE, NULL,
                0);
        goto done;
    }

    /* if the private key has not been set, then it can't be started. */
    if (NULL == instance->private_key)
    {
        retval =
            canonizationservice_decode_and_dispatch_write_status(
                sock, CANONIZATIONSERVICE_API_METHOD_START, 0U,
                AGENTD_ERROR_CANONIZATIONSERVICE_START_BEFORE_PRIVATE_KEY_SET,
                NULL, 0);
        goto done;
    }

    /* if this instance is running, then it can't be started again. */
    if (instance->running)
    {
        retval =
            canonizationservice_decode_and_dispatch_write_status(
                sock, CANONIZATIONSERVICE_API_METHOD_START, 0U,
                AGENTD_ERROR_CANONIZATIONSERVICE_ALREADY_RUNNING, NULL, 0);
        goto done;
    }

    /* otherwise, start the service. */
    instance->running = true;

    /* create a timer event for running the canonization action. */
    retval =
        ipc_timer_init(
            &instance->timer, instance->block_max_milliseconds,
            &canonizationservice_timer_cb, instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* TODO - write error status. */
        goto done;
    }

    /* set the timer event. */
    retval = ipc_event_loop_add_timer(instance->loop_context, &instance->timer);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* TODO - write error status. */
        goto cleanup_timer;
    }

    /* write a success status. */
    retval =
        canonizationservice_decode_and_dispatch_write_status(
            sock, CANONIZATIONSERVICE_API_METHOD_START, 0U,
            AGENTD_STATUS_SUCCESS, NULL, 0);
    goto done;

cleanup_timer:
    dispose((disposable_t*)&instance->timer);

done:
    return retval;
}
