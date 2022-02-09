/**
 * \file
 * protocolservice/pde_decode_and_dispatch_req_dataservice_req.c
 *
 * \brief Decode and dispatch a generic dataservice request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Decode and dispatch a generic dataservice request.
 *
 * \param ctx               The endpoint context.
 * \param req_payload       The request payload.
 * \param return_address    The return mailbox address, needed for looking up
 *                          the request context.
 * \param reply_payload     Pointer to the pointer to receive the reply payload
 *                          for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status pde_decode_and_dispatch_req_dataservice_req(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_protocol_write_endpoint_message** reply_payload)
{
    status retval, release_retval;
    protocolservice_dataservice_mailbox_context_entry* context_entry;
    void* reply_data = NULL;
    size_t reply_data_size;

    /* look up the child context entry. */
    retval =
        rbtree_find(
            (resource**)&context_entry, ctx->mailbox_context_tree,
            &return_address);
    if (STATUS_SUCCESS != retval)
    {
        /* if we were sent an ill-formed message, terminate this fiber. */
        goto done;
    }

    /* get a pointer to the request message payload. */
    uint8_t* breq = (uint8_t*)req_payload->payload.data;

    /* set the child context in the request message. */
    uint32_t ncontext = htonl(context_entry->context);
    memcpy(breq + 4, &ncontext, sizeof(ncontext));

    /* write this message to the dataservice socket. */
    retval =
        psock_write_boxed_data(
            ctx->datasock, req_payload->payload.data,
            req_payload->payload.size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* read the response back from the dataservice socket. */
    retval =
        psock_read_boxed_data(
            ctx->datasock, ctx->alloc, &reply_data, &reply_data_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create the payload to send to the protocolservice write endpoint. */
    retval =
        protocolservice_protocol_write_endpoint_message_create(
            reply_payload, ctx,
            PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_DATASERVICE_MSG,
            req_payload->offset, reply_data, reply_data_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_reply_data;
    }

    /* success. */
    goto cleanup_reply_data;

cleanup_reply_data:
    if (NULL != reply_data)
    {
        release_retval = rcpr_allocator_reclaim(ctx->alloc, reply_data);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        reply_data = NULL;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
