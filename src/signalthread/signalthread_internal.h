/**
 * \file signalthread/signalthread_internal.h
 *
 * \brief Internal types and definitions for signal thread.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <agentd/signalthread.h>
#include <rcpr/psock.h>
#include <rcpr/resource/protected.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/** \brief the size of the signal thread stack. */
#define SIGNALTHREAD_STACK_SIZE     16384

/**
 * \brief The context for a signal thread.
 */
typedef struct signalthread_context signalthread_context;

struct signalthread_context
{
    RCPR_SYM(resource) hdr;

    RCPR_MODEL_STRUCT_TAG(signalthread_context);

    RCPR_SYM(allocator)* alloc;
    RCPR_SYM(psock)* signal_sock;
    useconds_t sleep_usecs;
};

/**
 * \brief Release a \ref signalthread_context resource.
 *
 * \param r             The signalthread_context resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status signalthread_context_resource_release(RCPR_SYM(resource)* r);

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
status signalthread_entry(void* context);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus
