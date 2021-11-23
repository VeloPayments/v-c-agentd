/**
 * \file protocolservice/unauthorized_protocol_service_control_write.c
 *
 * \brief Write data to the control socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <errno.h>
#include <vpr/parameters.h>

#if !defined(AGENTD_NEW_PROTOCOL)

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Write data to the control socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_control_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    unauthorized_protocol_service_instance_t* instance =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_WRITE);
    MODEL_ASSERT(NULL != instance);

    /* write data if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* has the socket been closed? */
        if (0 == bytes_written)
        {
            goto exit_failure;
        }
        /* has there been a socket error? */
        else if (bytes_written < 0)
        {
            /* for any error except retrying, exit the loop. */
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                goto exit_failure;
            }
        }
        /* re-enable the callback if there is more data to write. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                ctx, &unauthorized_protocol_service_control_write,
                &instance->loop);
        }
    }
    else
    {
        /* disable the callback if there is no more data to write. */
        ipc_set_writecb_noblock(ctx, NULL, &instance->loop);
    }

    /* success. */
    return;

exit_failure:
    unauthorized_protocol_service_exit_event_loop(instance);
}

#endif /* !defined(AGENTD_NEW_PROTOCOL) */
