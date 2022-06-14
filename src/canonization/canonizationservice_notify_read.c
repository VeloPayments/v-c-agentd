/**
 * \file canonization/canonizationservice_notify_read.c
 *
 * \brief Read data from the notification service socket from the canonization
 * service socket.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/notificationservice/api.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle read events on the notification service socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_notify_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    uint8_t* resp = NULL;
    uint32_t resp_size = 0U;
    uint32_t method_id, status_code;
    uint64_t offset;
    const uint8_t* payload = NULL;
    size_t payload_size = 0U;

    /* get the instance from the user context. */
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->force_exit)
        return;

    /* attempt to read a response packet. */
    int retval = ipc_read_data_noblock(ctx, (void**)&resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        goto done;
    }
    /* handle general failures from the notification service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* sanity check. We should be in the block update read state. */
    if (CANONIZATIONSERVICE_STATE_WAITRESP_NOTIFY_BLOCK_UPDATE !=
            instance->state)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* decode the notificationservice response. */
    retval =
        notificationservice_api_decode_response(
            resp, resp_size, &method_id, &status_code, &offset, &payload,
            &payload_size);
    if (STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* sanity check of the response from block update. */
    if (
            method_id != NOTIFICATIONSERVICE_API_CAP_BLOCK_UPDATE
         || status_code != AGENTD_STATUS_SUCCESS )
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* close the child context. */
    canonizationservice_child_context_close(instance);

    /* success. */

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);

done:;
}
