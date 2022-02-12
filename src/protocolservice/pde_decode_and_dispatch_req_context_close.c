/**
 * \file
 * protocolservice/pde_decode_and_dispatch_req_context_close.c
 *
 * \brief Decode and dispatch a context close request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Decode and dispatch a dataservice context close request.
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
status pde_decode_and_dispatch_req_context_close(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) /*return_address*/,
    protocolservice_protocol_write_endpoint_message** reply_payload)
{
    status retval;
    uint32_t status, offset;
    protocolservice_dataservice_mailbox_context_entry* entry = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_dataservice_request_message_valid(req_payload));
    MODEL_ASSERT(return_address > 0);
    MODEL_ASSERT(NULL != reply_payload);

    /* look up the context entry by return address. */
    retval =
        rbtree_find(
            (resource**)&entry, ctx->mailbox_context_tree, &req_payload->data);
    if (STATUS_SUCCESS != retval)
    {
        goto send_response;
    }

    /* send the dataservice child context close request. */
    retval =
        dataservice_api_sendreq_child_context_close(
            ctx->datasock, &ctx->vpr_alloc, entry->context);
    if (STATUS_SUCCESS != retval)
    {
        goto send_response;
    }

    /* receive the dataservice child context close response. */
    retval =
        dataservice_api_recvresp_child_context_close(
            ctx->datasock, ctx->alloc, &offset, &status);
    if (STATUS_SUCCESS != retval)
    {
        goto send_response;
    }

    /* if the request failed on the dataservice side, log the error. */
    if (STATUS_SUCCESS != status)
    {
        retval = status;
        goto send_response;
    }

    /* remove the entry from the mailbox_context tree. */
    retval = rbtree_delete(NULL, ctx->mailbox_context_tree, &entry->addr);
    if (STATUS_SUCCESS != retval)
    {
        goto send_response;
    }

    /* remove the entry from the context_mailbox tree. */
    retval = rbtree_delete(NULL, ctx->context_mailbox_tree, &entry->context);
    if (STATUS_SUCCESS != retval)
    {
        goto send_response;
    }

    /* success. */
    goto send_response;

send_response:
    return
        protocolservice_protocol_write_endpoint_message_create(
            reply_payload, ctx,
            PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_DATASERVICE_CONTEXT_CREATE_MSG,
            0U, req_payload->offset, NULL, 0U);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
