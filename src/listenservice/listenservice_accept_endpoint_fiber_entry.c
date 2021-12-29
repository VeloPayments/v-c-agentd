/**
 * \file listenservice/listenservice_accept_endpoint_fiber_entry.c
 *
 * \brief Entry point for the accept endpoint fiber.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <rcpr/uuid.h>
#include <unistd.h>

#include "listenservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/**
 * \brief Entry point for the accept endpoint fiber.
 *
 * This fiber receives sockets from each of the listen fibers and forwards these
 * to the protocol service.
 *
 * \param vctx          The type erased context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_accept_endpoint_fiber_entry(void* vctx)
{
    status retval, release_retval;
    listenservice_accept_endpoint_context* ctx =
        (listenservice_accept_endpoint_context*)vctx;
    message* recvmsg;
    listenservice_accept_message* payload;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_listenservice_accept_endpoint_context_valid(ctx));

    /* read message loop. */
    while (!ctx->quiesce)
    {
        /* read a message from the message queue. */
        retval = message_receive(ctx->endpoint_addr, &recvmsg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* get the payload. */
        payload =
            (listenservice_accept_message*)message_payload(recvmsg, false);

        /* write the descriptor to the accept socket. */
        retval = psock_write_raw_descriptor(ctx->accept_socket, payload->desc);
        /* TODO - log failure. */

        /* clean up the message. */
        release_retval = resource_release(message_resource_handle(recvmsg));
        if (STATUS_SUCCESS != release_retval)
        {
            /* TODO - log failure. */
            retval = release_retval;
        }

        /* if either of the above operations failed, exit. */
        if (STATUS_SUCCESS != retval)
        {
            /* TODO - log failure. */
            goto cleanup_context;
        }
    }

cleanup_context:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    return retval;
}
