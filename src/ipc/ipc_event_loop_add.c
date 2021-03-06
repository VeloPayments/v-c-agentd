/**
 * \file ipc/ipc_event_loop_add.c
 *
 * \brief Add a non-blocking socket descriptor to an event loop.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Add a non-blocking socket to the event loop.
 *
 * On success, the event loop will manage events on this non-blocking socket.
 * Note that the ownership for this socket context remains with the caller.  It
 * is the caller's responsibility to remove this socket from the event loop and
 * to dispose the socket.
 *
 * If either the read or write callbacks are set when this method is called,
 * they will be added as persistent callbacks.  If this is not desired behavior,
 * wait to add the read or write callbacks until *AFTER* adding the socket to
 * the event loop.  The persistent callback behavior is backwards compatible to
 * other code in agentd expecting this behavior.
 *
 * \param loop          The event loop context to which this socket is added.
 * \param sock          The socket context to add to the event loop.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_INVALID_ARGUMENT if the socket context has already
 *        been added to an event loop.
 *      - AGENTD_ERROR_IPC_EVBUFFER_NEW_FAILURE if a new event buffer could not
 *        be created.
 *      - AGENTD_ERROR_IPC_EVENT_NEW_FAILURE if a new event could not be
 *        created.
 *      - AGENTD_ERROR_IPC_EVENT_ADD_FAILURE if the event cannot be added to the
 *        event loop.
 */
int ipc_event_loop_add(
    ipc_event_loop_context_t* loop, ipc_socket_context_t* sock)
{
    ssize_t retval = 0;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);
    MODEL_ASSERT(NULL != sock);

    /* get the impls. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* create the read buffer. */
    if (NULL == sock_impl->readbuf)
    {
        sock_impl->readbuf = evbuffer_new();
        if (NULL == sock_impl->readbuf)
        {
            retval = AGENTD_ERROR_IPC_EVBUFFER_NEW_FAILURE;
            goto done;
        }
    }

    /* create the write buffer. */
    if (NULL == sock_impl->writebuf)
    {
        sock_impl->writebuf = evbuffer_new();
        if (NULL == sock_impl->writebuf)
        {
            retval = AGENTD_ERROR_IPC_EVBUFFER_NEW_FAILURE;
            goto cleanup_readbuf;
        }
    }

    /* maybe create a read event. */
    if (sock->read)
    {
        /* clean up the read event if already set. */
        if (NULL != sock_impl->read_ev)
        {
            event_free(sock_impl->read_ev);
            sock_impl->read_ev = NULL;
        }

        /* create the read event. */
        sock_impl->read_ev =
            event_new(
                loop_impl->evb, sock->fd, EV_READ | EV_PERSIST,
                &ipc_event_loop_cb, sock);
        if (NULL == sock_impl->read_ev)
        {
            retval = AGENTD_ERROR_IPC_EVENT_NEW_FAILURE;
            goto cleanup_writebuf;
        }

        /* add the event to the event base. */
        if (0 != event_add(sock_impl->read_ev, NULL))
        {
            retval = AGENTD_ERROR_IPC_EVENT_ADD_FAILURE;
            goto cleanup_read_ev;
        }
    }

    /* maybe create a write event. */
    if (sock->write)
    {
        /* clean up the write event if already set. */
        if (NULL != sock_impl->write_ev)
        {
            event_free(sock_impl->write_ev);
            sock_impl->write_ev = NULL;
        }

        /* create the write event. */
        sock_impl->write_ev =
            event_new(
                loop_impl->evb, sock->fd, EV_WRITE | EV_PERSIST,
                &ipc_event_loop_cb, sock);
        if (NULL == sock_impl->write_ev)
        {
            retval = AGENTD_ERROR_IPC_EVENT_NEW_FAILURE;
            goto cleanup_read_ev;
        }

        /* add the event to the event base. */
        if (0 != event_add(sock_impl->write_ev, NULL))
        {
            retval = AGENTD_ERROR_IPC_EVENT_ADD_FAILURE;
            goto cleanup_write_ev;
        }
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_write_ev:
    if (NULL != sock_impl->write_ev)
    {
        event_free(sock_impl->write_ev);
        sock_impl->write_ev = NULL;
    }

cleanup_read_ev:
    if (NULL != sock_impl->read_ev)
    {
        event_free(sock_impl->read_ev);
        sock_impl->read_ev = NULL;
    }

cleanup_writebuf:
    evbuffer_free(sock_impl->writebuf);
    sock_impl->writebuf = NULL;

cleanup_readbuf:
    evbuffer_free(sock_impl->readbuf);
    sock_impl->readbuf = NULL;

done:
    return retval;
}
