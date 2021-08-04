/**
 * \file attestationservice/attestationservice_create_reaper_fiber.c
 *
 * \brief Create the reaper fiber for the attestation service.
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

typedef struct reaper_fiber_instance reaper_fiber_instance;
struct reaper_fiber_instance
{
    resource hdr;

    rcpr_allocator* alloc;
    fiber_scheduler* sched;
    psock* sock;
};

//forward decls
static status reaper_fiber_entry(void* context);
static status reaper_fiber_instance_release(resource* r);

/**
 * \brief Create a fiber to listen to quiesce / terminate events, and broadcast
 * these to all other fibers to reap them.
 *
 * \param th            The thread instance to create.
 * \param alloc         The allocator to use for this operation.
 * \param signal_fd     The descriptor that this fiber uses to listen for
 *                      events.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_reaper_fiber(
    fiber** fib, rcpr_allocator* alloc, fiber_scheduler* sched, int signal_fd)
{
    status retval, release_retval;
    reaper_fiber_instance* inst;
    psock* base_sock;
    fiber* tmp;

    /* allocate memory for the reaper fiber instance. */
    TRY_OR_FAIL(
        rcpr_allocator_allocate(
            alloc, (void**)&inst, sizeof(reaper_fiber_instance)),
        close_fds);

    /* initialize the instance resource. */
    resource_init(&inst->hdr, &reaper_fiber_instance_release);

    /* set values. */
    inst->alloc = alloc;
    inst->sock = NULL;
    inst->sched = sched;

    /* create the fiber for this instance. */
    TRY_OR_FAIL(
        fiber_create(
            &tmp, alloc, sched, 1024 * 1024, inst, &reaper_fiber_entry),
        cleanup_inst);

    /* create the base psock instance for communicating with the signal
     * thread. */
    TRY_OR_FAIL(
        psock_create_from_descriptor(&base_sock, alloc, signal_fd),
        cleanup_tmp);

    /* the descriptor is now owned by this psock instance. */
    signal_fd = -1;

    /* create the async psock instance. */
    TRY_OR_FAIL(
        psock_create_wrap_async(&inst->sock, alloc, tmp, base_sock),
        cleanup_base_sock);

    /* base sock is now owned by the instance. */
    base_sock = NULL;

    /* send the fiber to the caller on success. */
    *fib = tmp;
    tmp = NULL;
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_base_sock:
    if (NULL != base_sock)
    {
        CLEANUP_OR_FALLTHROUGH(
            resource_release(psock_resource_handle(base_sock)));
    }

cleanup_tmp:
    CLEANUP_OR_FALLTHROUGH(resource_release(fiber_resource_handle(tmp)));

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

close_fds:
    if (signal_fd != -1)
        close(signal_fd);

done:
    return retval;
}

/**
 * \brief Clean up the fiber instance on release.
 *
 * \param r             The fiber instance to clean up.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status reaper_fiber_instance_release(resource* r)
{
    status psock_retval = STATUS_SUCCESS;
    status reclaim_retval;
    reaper_fiber_instance* inst = (reaper_fiber_instance*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = inst->alloc;

    /* clean up the socket if it exists. */
    if (NULL != inst->sock)
    {
        psock_retval = resource_release(psock_resource_handle(inst->sock));
    }

    /* clean up the instance structure. */
    reclaim_retval = rcpr_allocator_reclaim(alloc, inst);

    if (STATUS_SUCCESS != psock_retval)
    {
        return psock_retval;
    }
    else
    {
        return reclaim_retval;
    }
}

/**
 * \brief Entry point for the reaper fiber.
 *
 * \param context           The reaper fiber instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status reaper_fiber_entry(void* context)
{
    status retval, release_retval;
    reaper_fiber_instance* inst = (reaper_fiber_instance*)context;

    for (;;)
    {
        uint64_t msg;

        /* read an event from the signal socket. */
        TRY_OR_FAIL(
            psock_read_boxed_uint64(inst->sock, &msg),
            cleanup_inst);

        /* decode the message. */
        switch (msg)
        {
            /* handle quiesce event. */
            case SIGNAL_STATE_QUIESCE:
                TRY_OR_FAIL(
                    disciplined_fiber_scheduler_send_quiesce_request_to_all(
                        inst->sched),
                    cleanup_inst);
                break;

            /* handle terminate event / exit fiber. */
            case SIGNAL_STATE_TERMINATE:
                TRY_OR_FAIL(
                    disciplined_fiber_scheduler_send_terminate_request_to_all(
                        inst->sched),
                    cleanup_inst);
                goto cleanup_inst;

            /* exit fiber for any other event. */
            default:
                goto cleanup_inst;
        }
    }

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

    return retval;
}
