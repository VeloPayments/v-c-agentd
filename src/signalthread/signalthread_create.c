/**
 * \file signalthread/signalthread_create.c
 *
 * \brief Create a signal thread instance.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <rcpr/socket_utilities.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "signalthread_internal.h"

RCPR_IMPORT_allocator;
RCPR_IMPORT_fiber;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;
RCPR_IMPORT_socket_utilities;
RCPR_IMPORT_thread;

RCPR_MODEL_STRUCT_TAG_GLOBAL_EXTERN(signalthread_context);

/**
 * \brief Create a signal thread to manage signals for a given agentd service.
 *
 * The signal thread allows signals to be processed independently of the fiber
 * scheduler. This is a requirement for fiber management, since fibers can't be
 * interrupted by signals.
 *
 * \param th            Pointer to the thread pointer, populated by the thread
 *                      instance on success.
 * \param signal_sock   Pointer to the psock pointer, populated by the signal
 *                      psock on success.
 * \param alloc         The allocator to use for this operation.
 * \param calling_fiber The caller's fiber instance, which is tied to the signal
 *                      socket on success.
 * \param sleep_usecs   The number of microseconds to sleep between quiesce and
 *                      terminate.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_create(
    RCPR_SYM(thread)** th, RCPR_SYM(psock)** signal_sock,
    RCPR_SYM(allocator)* alloc, RCPR_SYM(fiber)* calling_fiber,
    useconds_t sleep_usecs)
{
    sigset_t sigset;
    status retval, release_retval;
    signalthread_context* ctx;
    int fiberdesc = -1, threaddesc = -1;
    psock* tmp = NULL;

    /* empty the signal set. */
    sigfillset(&sigset);

    /* set the signal mask for this thread. */
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /* create the file descriptors for the thread / fiber communications. */
    retval =
        socket_utility_socketpair(
            AF_UNIX, SOCK_DGRAM, 0, &fiberdesc, &threaddesc);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* allocate memory for the thread context. */
    retval =
        allocator_allocate(alloc, (void**)&ctx, sizeof(signalthread_context));
    if (STATUS_SUCCESS != retval)
    {
        goto close_fds;
    }

    /* clear the struct. */
    memset(ctx, 0, sizeof(signalthread_context));

    /* the tag is not set by default. */
    RCPR_MODEL_ASSERT_STRUCT_TAG_NOT_INITIALIZED(
        ctx->RCPR_MODEL_STRUCT_TAG_REF(signalthread_context),
        signalthread_context);

    /* set the tag. */
    RCPR_MODEL_STRUCT_TAG_INIT(
        ctx->RCPR_MODEL_STRUCT_TAG_REF(signalthread_context),
        signalthread_context);

    /* set the resource handler. */
    resource_init(&ctx->hdr, &signalthread_context_resource_release);

    /* set the init values. */
    ctx->alloc = alloc;
    ctx->sleep_usecs = sleep_usecs;

    /* create the fiber psock. */
    retval = psock_create_from_descriptor(&tmp, alloc, fiberdesc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* the fiber descriptor is now owned by the fiber psock. */
    fiberdesc = -1;

    /* wrap the psock. */
    retval = psock_create_wrap_async(signal_sock, alloc, calling_fiber, tmp);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_fiber_psock;
    }

    /* the psock is now owned by the async psock instance. */
    tmp = NULL;

    /* create the thread psock. */
    retval = psock_create_from_descriptor(&ctx->signal_sock, alloc, threaddesc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_fiber_async_sock;
    }

    /* the thread descriptor is now owned by the thread psock. */
    threaddesc = -1;

    /* create the thread. */
    retval =
        thread_create(
            th, alloc, SIGNALTHREAD_STACK_SIZE, ctx, &signalthread_entry);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_fiber_async_sock;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_fiber_async_sock:
    release_retval = resource_release(psock_resource_handle(*signal_sock));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_fiber_psock:
    if (NULL != tmp)
    {
        release_retval = resource_release(psock_resource_handle(tmp));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }

        tmp = NULL;
    }

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

close_fds:
    if (threaddesc >= 0)
    {
        close(threaddesc);
    }

    if (fiberdesc >= 0)
    {
        close(fiberdesc);
    }

done:
    return retval;
}
