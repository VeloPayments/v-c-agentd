/**
 * \file listenservice/listenservice_listen_fiber_entry.c
 *
 * \brief Entry point for the listen fiber.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <rcpr/uuid.h>
#include <unistd.h>

#include "listenservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_message;
RCPR_IMPORT_psock;
RCPR_IMPORT_resource;

/* forward decls. */
static status accept_message_payload_create(
    resource** payload, rcpr_allocator* alloc, int desc);
static status accept_message_payload_release(resource* r);

/**
 * \brief Entry point for the listen service listen fiber.
 *
 * This fiber listens to a socket for new connections, and passes these to the
 * accept endpoint, where they are sent to the protocol service.
 *
 * \param vctx          The type erased context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status listenservice_listen_fiber_entry(void* vctx)
{
    status retval, release_retval;
    socklen_t peerlen;
    struct sockaddr_in peeraddr;
    resource* payload;
    message* msg;

    listenservice_listen_fiber_context* ctx =
        (listenservice_listen_fiber_context*)vctx;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_listenservice_listen_fiber_context_valid(ctx));

    /* loop through connections. */
    while (!ctx->quiesce)
    {
        /* accept a new connection from the listen socket. */
        int desc;
        peerlen = sizeof(peeraddr);
        retval =
            psock_accept(
                ctx->listen_socket, &desc, (struct sockaddr*)&peeraddr,
                &peerlen);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_context;
        }

        /* on quiesce, close this connection. */
        if (ctx->quiesce)
        {
            close(desc);
            continue;
        }

        /* create a message payload. */
        retval = accept_message_payload_create(&payload, ctx->alloc, desc);
        if (STATUS_SUCCESS != retval)
        {
            close(desc);
            goto cleanup_context;
        }

        /* create a message to send this socket to the accept endpoint. */
        retval = message_create(&msg, ctx->alloc, ctx->return_addr, payload);
        if (STATUS_SUCCESS != retval)
        {
            resource_release(payload);
            close(desc);
            goto cleanup_context;
        }

        /* send this message to the endpoint. */
        retval = message_send(ctx->endpoint_addr, msg, ctx->msgdisc);
        if (STATUS_SUCCESS != retval)
        {
            resource_release(message_resource_handle(msg));
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

/**
 * \brief Create an accept message payload.
 *
 * \param payload           Pointer to the payload pointer to be set with the
 *                          created payload on success.
 * \param alloc             The allocator to use to create this payload.
 * \param desc              The new socket descriptor that has been accepted.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status accept_message_payload_create(
    resource** payload, rcpr_allocator* alloc, int desc)
{
    status retval;
    listenservice_accept_message* tmp;

    /* allocate memory for this resource. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* clear this memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set the resource release method. */
    resource_init(&tmp->hdr, &accept_message_payload_release);

    /* set all other fields. */
    tmp->alloc = alloc;
    tmp->desc = desc;

    /* success. Set the return value. */
    *payload = &tmp->hdr;
    return STATUS_SUCCESS;
}

/**
 * \brief Release an accept message payload resource.
 *
 * \param r             The resource to release.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status accept_message_payload_release(resource* r)
{
    listenservice_accept_message* payload = (listenservice_accept_message*)r;

    rcpr_allocator* alloc = payload->alloc;

    /* close the accepted socket descriptor if valid. */
    if (payload->desc >= 0)
    {
        close(payload->desc);
    }

    /* reclaim the memory for this payload. */
    return rcpr_allocator_reclaim(alloc, payload);
}
