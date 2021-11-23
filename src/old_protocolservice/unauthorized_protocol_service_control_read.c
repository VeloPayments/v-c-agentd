/**
 * \file protocolservice/unauthorized_protocol_service_control_read.c
 *
 * \brief Read commands over the control socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#if !defined(AGENTD_NEW_PROTOCOL)

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Read data from the control socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_control_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    bool read_loop = true;
    int retval = 0;
    void* req;
    uint32_t size = 0;
    unauthorized_protocol_service_instance_t* instance =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->force_exit)
        return;

    while (read_loop)
    {
        /* attempt to read a request. */
        retval = ipc_read_data_noblock(ctx, &req, &size);
        switch (retval)
        {
            /* on success, decode and dispatch. */
            case AGENTD_STATUS_SUCCESS:
                if (AGENTD_STATUS_SUCCESS !=
                    unauthorized_protocol_service_control_decode_and_dispatch(
                        instance, ctx, req, size))
                {
                    /* a bad control message means we should shut down. */
                    unauthorized_protocol_service_exit_event_loop(instance);
                    read_loop = false;
                }

                /* clear and free the request data. */
                memset(req, 0, size);
                free(req);
                break;

            /* wait for more data on the socket. */
            case AGENTD_ERROR_IPC_WOULD_BLOCK:
                read_loop = false;
                break;

            /* any other error code indicates that we should no longer trust the
             * control socket. */
            default:
                unauthorized_protocol_service_exit_event_loop(instance);
                read_loop = false;
                break;
        }
    }

    /* fire up the write callback if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        ipc_set_writecb_noblock(
            ctx, &unauthorized_protocol_service_control_write, &instance->loop);
    }
}

#endif /* !defined(AGENTD_NEW_PROTOCOL) */
