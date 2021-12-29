/**
 * \file listenservice/listenservice_event_loop.c
 *
 * \brief The event loop for the listen service.
 *
 * \copyright 2019-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/signalthread.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "listenservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_thread;

/**
 * \brief Event loop for the unauthorized listen service.  This is the entry
 * point for the listen service.  It handles the details of reacting to events
 * sent over the listen service socket.
 *
 * \param logsock       The logging service socket.  The listen service logs
 *                      on this socket.
 * \param acceptsock    The socket to which newly accepted sockets are sent.
 * \param listenstart   The first socket to which this service will listen.  The
 *                      listen service will iterate from this socket until it
 *                      encounters a closed descriptor and use each as a listen
 *                      socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the listen service socket to the event loop failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the listen service event loop failed.
 */
int listenservice_event_loop(
    int UNUSED(logsock), int acceptsock, int listenstart)
{
    int retval, release_retval;
    rcpr_allocator* alloc;
    thread* signalthread;
    psock* signal_sock;
    fiber_scheduler* sched;
    fiber* main_fiber;
    mailbox_address endpoint_addr;
    bool should_exit = false;

    /* parameter sanity checking. */
    MODEL_ASSERT(logsock >= 0);
    MODEL_ASSERT(listenstart >= 0);

    /* count the number of listen sockets. */
    int listensocket_count = inet_count_sockets(listenstart);

    /* create the allocator instance. */
    retval = rcpr_malloc_allocator_create(&alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create a fiber scheduler instance. */
    retval = fiber_scheduler_create_with_disciplines(&sched, alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_alloc;
    }

    /* add the management fiber. */
    retval = listenservice_management_fiber_add(alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* create the accept endpoint fiber. */
    retval =
        listenservice_accept_endpoint_fiber_add(
            alloc, sched, &endpoint_addr, acceptsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* create each listener fiber. */
    for (int i = 0; i < listensocket_count; ++i)
    {
        retval =
            listenservice_listen_fiber_add(
                alloc, sched, endpoint_addr, listenstart + i);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_scheduler;
        }
    }

    /* get the main fiber. */
    retval = disciplined_fiber_scheduler_main_fiber_get(&main_fiber, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* create the signal thread. */
    retval =
        signalthread_create(&signalthread, &signal_sock, alloc, main_fiber, 2);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* termination loop. */
    do
    {
        /* read a signal state from the signal thread. */
        int64_t payload_state;
        retval = psock_read_boxed_int64(signal_sock, &payload_state);
        if (STATUS_SUCCESS != retval)
        {
            goto terminate_process;
        }

        /* signal dispatch. */
        switch (payload_state)
        {
            /* quiesce all fibers. */
            case SIGNAL_STATE_QUIESCE:
                retval =
                    disciplined_fiber_scheduler_send_quiesce_request_to_all(
                        sched);
                if (STATUS_SUCCESS != retval)
                {
                    goto terminate_process;
                }
                break;

            /* terminate all fibers. */
            case SIGNAL_STATE_TERMINATE:
                should_exit = true;
                retval =
                    disciplined_fiber_scheduler_send_terminate_request_to_all(
                        sched);
                if (STATUS_SUCCESS != retval)
                {
                    goto terminate_process;
                }
                break;
        }
    } while (!should_exit);

    /* success. */
    retval = STATUS_SUCCESS;
    goto join_signal_thread;

terminate_process:
    exit(retval);

join_signal_thread:
    release_retval = resource_release(thread_resource_handle(signalthread));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

/*cleanup_signal_sock:*/
    release_retval = resource_release(psock_resource_handle(signal_sock));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_scheduler:
    release_retval = resource_release(fiber_scheduler_resource_handle(sched));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_alloc:
    release_retval = resource_release(rcpr_allocator_resource_handle(alloc));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
