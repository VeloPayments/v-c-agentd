/**
 * \file notificationservice/notificationservice_run.c
 *
 * \brief The main entry point for the notification service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice.h>
#include <agentd/signalthread.h>
#include <vpr/parameters.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_thread;

/* the number of microseconds to sleep for quiescing fibers. */
#define NOTIFICATIONSERVICE_QUIESCE_SLEEP_USECS 100000 /* 100 milliseconds. */

/**
 * \brief Main entry point for the notification service.  It handles the details
 * of reacting to events sent over the protocol service socket.
 *
 * \param logsock       The socket to the logging service service.
 * \param consensussock Socket connection to the consensus service.
 * \param protocolsock  Socket connection to the protocol service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - a non-zero error code on failure.
 */
status notificationservice_run(
    int UNUSED(logsock), int consensussock, int protocolsock)
{
    status retval, release_retval;
    rcpr_allocator* alloc;
    fiber_scheduler* sched;
    fiber* main_fiber;
    thread* signalthread;
    psock* signal_sock;
    notificationservice_context* ctx;
    notificationservice_instance* cinst;
    notificationservice_instance* pinst;

    /* parameter sanity checks. */
    MODEL_ASSERT(logsock >= 0);
    MODEL_ASSERT(consensussock >= 0);
    MODEL_ASSERT(protocolsock >= 0);

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

    /* create the notification service context. */
    retval = notificationservice_context_create(&ctx, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_sched;
    }

    /* get the main fiber. */
    retval = disciplined_fiber_scheduler_main_fiber_get(&main_fiber, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* save the main fiber. */
    ctx->main_fiber = main_fiber;

    /* look up the messaging discipline. */
    retval = message_discipline_get_or_create(&ctx->msgdisc, alloc, sched);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* create an instance for the consensus socket. */
    retval = notificationservice_instance_create(&cinst, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* create an instance for the protocol socket. */
    retval = notificationservice_instance_create(&pinst, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_cinst;
    }

    /* add a protocol fiber for the consensus socket. */
    retval =
        notificationservice_protocol_fiber_add(
            alloc, cinst, consensussock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_pinst;
    }

    /* add a protocol fiber for the protocol socket. */
    retval =
        notificationservice_protocol_fiber_add(
            alloc, pinst, protocolsock);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_pinst;
    }

    /* add an outbound endpoint fiber for the consensus socket. */
    retval =
        notificationservice_protocol_outbound_endpoint_add(alloc, cinst);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_pinst;
    }

    /* add an outbound endpoint fiber for the protocol socket. */
    retval =
        notificationservice_protocol_outbound_endpoint_add(alloc, pinst);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_pinst;
    }

    /* create the signal thread. */
    retval =
        signalthread_create(
            &signalthread, &signal_sock, alloc, main_fiber,
            NOTIFICATIONSERVICE_QUIESCE_SLEEP_USECS);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_pinst;
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

cleanup_pinst:
    release_retval = resource_release(&pinst->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_cinst:
    release_retval = resource_release(&cinst->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_sched:
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
