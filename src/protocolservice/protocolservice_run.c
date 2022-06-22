/**
 * \file protocolservice/protocolservice_run.c
 *
 * \brief The main entry point for the protocol service.
 *
 * \copyright 2021-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <config.h>
#include <agentd/signalthread.h>
#include <agentd/status_codes.h>
#include <stdlib.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_thread;

/* the number of microseconds to sleep for quiescing fibers. */
#define PROTOCOLSERVICE_QUIESCE_SLEEP_USECS 100000 /* 100 milliseconds. */

/**
 * \brief Main entry point for the protocol service.  It handles the details of
 * reacting to events sent over the protocol service socket.
 *
 * \param randomsock    The socket to the RNG service.
 * \param protosock     The protocol service socket.  The protocol service
 *                      listens for connections on this socket.
 * \param controlsock   The control socket. The supervisor sends commands to the
 *                      protocol service over this socket.
 * \param datasock      The data service socket.  The protocol service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 * \param notifysock    The notification service socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *          attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the protocol service event loop failed.
 */
int protocolservice_run(
    int randomsock, int protosock, int controlsock, int datasock,
    int UNUSED(logsock), int notifysock)
{
    status retval, release_retval;
    rcpr_allocator* alloc;
    thread* signalthread;
    psock* signal_sock;
    fiber_scheduler* sched;
    fiber* main_fiber;
    mailbox_address data_endpoint_addr;
    mailbox_address random_endpoint_addr;
    mailbox_address notify_endpoint_addr;
    protocolservice_context* ctx;

    /* parameter sanity checking. */
    MODEL_ASSERT(randomsock >= 0);
    MODEL_ASSERT(protosock >= 0);
    MODEL_ASSERT(controlsock >= 0);
    MODEL_ASSERT(datasock >= 0);
    MODEL_ASSERT(logsock >= 0);

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

    /* add the data service endpoint fiber. */
    retval =
        protocolservice_dataservice_endpoint_add(
            &data_endpoint_addr, alloc, sched, datasock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* add the random service endpoint fiber. */
    retval =
        protocolservice_randomservice_endpoint_add(
            &random_endpoint_addr, alloc, sched, randomsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* create the protocol service instance. */
    retval =
        protocolservice_context_create(
            &ctx, alloc, sched, random_endpoint_addr, data_endpoint_addr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_scheduler;
    }

    /* add the notification service endpoint fiber. */
    retval =
        protocolservice_notificationservice_endpoint_add(
            &notify_endpoint_addr, ctx, notifysock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* set the notificationservice endpoint address in the context. */
    ctx->notificationservice_endpoint_addr = notify_endpoint_addr;

    /* add the management fiber. */
    retval = protocolservice_management_fiber_add(alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* add the control fiber. */
    retval = protocolservice_control_fiber_add(alloc, ctx, controlsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* add the protocol accept fiber. */
    retval = protocolservice_accept_fiber_add(alloc, ctx, protosock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* get the main fiber. */
    retval = disciplined_fiber_scheduler_main_fiber_get(&main_fiber, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
    }

    /* set the main fiber in the context. */
    ctx->main_fiber = main_fiber;

    /* create the signal thread. */
    retval =
        signalthread_create(
            &signalthread, &signal_sock, alloc, main_fiber,
            PROTOCOLSERVICE_QUIESCE_SLEEP_USECS);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_context;
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
                ctx->terminate = true;
                retval =
                    disciplined_fiber_scheduler_send_terminate_request_to_all(
                        sched);
                if (STATUS_SUCCESS != retval)
                {
                    goto terminate_process;
                }
                break;
        }
    } while (!ctx->terminate);

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

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
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
