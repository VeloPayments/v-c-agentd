/**
 * \file canonization/canonizationservice_notify_write.c
 *
 * \brief Write data to the notification service socket from the canonization
 * service socket.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/canonizationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Callback for writing data to the notification service socket from the
 * canonization service.
 *
 * \param ctx           The socket context on which this write request occurred.
 * \param event_flags   The event flags that triggered this callback.
 * \param user_context  Opaque pointer to the canonization service instance.
 */
void canonizationservice_notify_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    /* get the canonization service instance from the user context. */
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* first, see if we still need to write data. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (    bytes_written == 0
            || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            canonizationservice_exit_event_loop(instance);
            return;
        }
        /* re-enable callback if there is more data to write. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                instance->notify, &canonizationservice_notify_write,
                instance->loop_context);
        }
    }
    else
    {
        /* disable callback if there is no more data to write. */
        ipc_set_writecb_noblock(instance->notify, NULL, instance->loop_context);
    }
}
