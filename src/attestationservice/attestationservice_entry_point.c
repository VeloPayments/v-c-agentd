/**
 * \file attestationservice/attestationservice_entry_point.c
 *
 * \brief The entry point for the attestation service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/attestationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_fiber;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_thread;

/**
 * \brief Entry point for the attestation service.
 *
 * \param datasock      The data service socket.  The attestation service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The attestation service
 *                      logs on this socket.
 * \param controlsock   The socket used to control the attestation service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - a non-zero error code on failure.
 */
int attestationservice_entry_point(
    int datasock, int logsock, int controlsock)
{
    status retval, release_retval;
    rcpr_allocator* alloc;
    fiber_scheduler* sched;
    int signal_fd, sleep_fd;
    thread* signal_thread;
    thread* sleep_thread;
    attestationservice_instance* inst;
    fiber* reaper;

    /* create the allocator. */
    TRY_OR_FAIL(rcpr_malloc_allocator_create(&alloc), done);

    /* create a fiber scheduler. */
    TRY_OR_FAIL(
        fiber_scheduler_create_with_disciplines(&sched, alloc),
        cleanup_alloc);

    /* create a signal handling thread, returning a socket descriptor for
     * receiving quiesce / terminate events.
     */
    TRY_OR_FAIL(
        attestationservice_create_signal_thread(
            &signal_thread, alloc, &signal_fd),
        cleanup_sched);

    /* create a sleeper thread for waking the main fiber, returning a socket
     * descriptor for receiving wake-up events.
     */
    TRY_OR_FAIL(
        attestationservice_create_sleep_thread(
            &sleep_thread, alloc, &sleep_fd),
        cleanup_sched);

    /* create a reaper fiber for sending quiesce / terminate events from the
     * signal thread.
     */
    TRY_OR_FAIL(
        attestationservice_create_reaper_fiber(
            &reaper, alloc, sched, signal_fd),
        cleanup_sched);

    /* Add the reaper fiber to the fiber scheduler. */
    TRY_OR_FAIL(fiber_scheduler_add(sched, reaper), cleanup_reaper);

    /* create the attestation service context. */
    TRY_OR_FAIL(
        attestationservice_create_instance(
            &inst, alloc, sched, sleep_fd, datasock, logsock, controlsock),
        cleanup_sched);

    /* enter the main event loop. */
    retval = attestationservice_event_loop(inst);

    /* We can circumvent cleanup if the event loop exits. */
    goto done;

cleanup_reaper:
    CLEANUP_OR_FALLTHROUGH(resource_release(fiber_resource_handle(reaper)));

cleanup_sched:
    CLEANUP_OR_FALLTHROUGH(
        resource_release(fiber_scheduler_resource_handle(sched)));

cleanup_alloc:
    CLEANUP_OR_FALLTHROUGH(
        resource_release(rcpr_allocator_resource_handle(alloc)));

done:
    return retval;
}
