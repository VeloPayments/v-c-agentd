/**
 * \file signalthread/signalthread_entry.c
 *
 * \brief Entry point for a signal thread.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <rcpr/socket_utilities.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "signalthread_internal.h"

RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief The entry point for the signal thread instance.
 *
 * \param context       Opaque pointer to the \ref signalthread_context instance
 *                      for this thread.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_entry(void* context)
{
    status retval, release_retval;
    sigset_t sigset;
    int sig;

    /* get the signal thread context. */
    signalthread_context* ctx = (signalthread_context*)context;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_signalthread_context_valid(ctx));

    /* wait and block on all signals. */
    sigfillset(&sigset);

    /* wait on a signal. */
    sigwait(&sigset, &sig);

    /* send the quiesce message. */
    retval = psock_write_boxed_int64(ctx->signal_sock, SIGNAL_STATE_QUIESCE);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* sleep before sending the terminate signal. */
    usleep(ctx->sleep_usecs);

    /* send the terminate message. */
    retval = psock_write_boxed_int64(ctx->signal_sock, SIGNAL_STATE_TERMINATE);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

done:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
