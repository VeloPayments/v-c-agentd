/**
 * \file attestationservice/attestationservice_create_signal_thread.c
 *
 * \brief Create the signal thread for the attestation service.
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
#include <rcpr/socket_utilities.h>
#include <vpr/parameters.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_socket_utilities;
RCPR_IMPORT_thread;

typedef struct signal_thread_instance signal_thread_instance;
struct signal_thread_instance
{
    resource hdr;

    rcpr_allocator* alloc;
    psock* sock;
};

//forward decls
static status signal_thread_entry(void* context);
static status signal_thread_instance_release(resource* r);

/**
 * \brief Create a signal thread for the attestation service.
 *
 * The signal thread listens for signals, and upon detecting one, translates the
 * signal into either a quiesce or a termination request.
 *
 * \param th            The thread instance to create.
 * \param alloc         The allocator to use for this operation.
 * \param signal_fd     Pointer to receive the descriptor to which the reaper
 *                      fiber should listen in order to forward quesce or
 *                      termination events to the fiber scheduler.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_signal_thread(
    thread** th, rcpr_allocator* alloc, int* signal_fd)
{
    status retval, release_retval;
    sigset_t sigset;
    signal_thread_instance* inst;
    int lhs = -1, rhs = -1;

    /* fill the signal set. */
    sigfillset(&sigset);

    /* block all signals at the process level. */
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    /* create the socketpair used for thread communication. */
    TRY_OR_FAIL(
        socket_utility_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs),
        done);

    /* allocate memory for the signal thread instance. */
    TRY_OR_FAIL(
        rcpr_allocator_allocate(
            alloc, (void**)&inst, sizeof(signal_thread_instance)),
        close_fds);

    /* initialize the instance resource. */
    resource_init(&inst->hdr, &signal_thread_instance_release);

    /* set values. */
    inst->alloc = alloc;
    inst->sock = NULL;

    /* create the psock instance for communicating with the reaper fiber. */
    TRY_OR_FAIL(
        psock_create_from_descriptor(&inst->sock, alloc, lhs),
        cleanup_inst);

    /* the instance takes ownership of lhs. */
    lhs = -1;

    /* create the thread for this instance. */
    TRY_OR_FAIL(
        thread_create(th, alloc, 16384, inst, &signal_thread_entry),
        cleanup_inst);

    /* the caller owns rhs on success. */
    *signal_fd = rhs;
    rhs = -1;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

close_fds:
    if (lhs != -1)
        close(lhs);
    if (rhs != -1)
        close(rhs);

done:
    return retval;
}

/**
 * \brief Clean up the thread instance on release.
 *
 * \param r             The thread instance to clean up.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status signal_thread_instance_release(resource* r)
{
    status psock_retval = STATUS_SUCCESS;
    status reclaim_retval;
    signal_thread_instance* inst = (signal_thread_instance*)r;

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
 * \brief Entry point for the signal thread.
 *
 * \param context           The signal thread instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status signal_thread_entry(void* context)
{
    status retval, release_retval;
    sigset_t sigset;
    int sig;
    signal_thread_instance* inst = (signal_thread_instance*)context;

    /* empty the signal set. */
    sigemptyset(&sigset);

    /* unblock all signals. */
    pthread_sigmask(SIG_SETMASK, &sigset, NULL);

    /* fill the signal set. */
    sigfillset(&sigset);

    /* wait on a signal. */
    sigwait(&sigset, &sig);

    /* send the quiesce message. */
    TRY_OR_FAIL(
        psock_write_boxed_uint64(inst->sock, SIGNAL_STATE_QUIESCE),
        cleanup_inst);

    /* wait 2 seconds. */
    sleep(2);

    /* send the terminate message. */
    TRY_OR_FAIL(
        psock_write_boxed_uint64(inst->sock, SIGNAL_STATE_TERMINATE),
        cleanup_inst);

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_inst;

cleanup_inst:
    CLEANUP_OR_FALLTHROUGH(resource_release(&inst->hdr));

    return retval;
}
