/**
 * \file
 * protocolservice/pde_decode_and_dispatch_req_context_open.c
 *
 * \brief Decode and dispatch a context open request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Decode and dispatch a dataservice context open request.
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
status pde_decode_and_dispatch_req_context_open(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_dataservice_response_message** reply_payload)
{
    status retval, release_retval;
    uint32_t offset, status, child;
    protocolservice_dataservice_mailbox_context_entry* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_dataservice_request_message_valid(req_payload));
    MODEL_ASSERT(return_address > 0);
    MODEL_ASSERT(NULL != reply_payload);

    /* create a mailbox_context entry. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear entry. */
    memset(tmp, 0, sizeof(*tmp));

    /* init entry. */
    resource_init(
        &tmp->hdr, &protocolservice_dataservice_mailbox_context_release);
    tmp->alloc = ctx->alloc;
    tmp->reference_count = 1;
    tmp->addr = return_address;

    /* send a dataservice child context create request to the data service. */
    retval =
        dataservice_api_sendreq_child_context_create(    
            ctx->datasock, req_payload->payload.data,
            req_payload->payload.size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_mailbox_context;
    }

    /* read the response from this operation. */
    retval =
        dataservice_api_recvresp_child_context_create(
            ctx->datasock, ctx->alloc, &offset, &status, &child);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_mailbox_context;
    }

    /* verify that context allocation was successful. */
    if (STATUS_SUCCESS != status)
    {
        goto cleanup_mailbox_context;
    }

    /* set the context. */
    tmp->context = child;

    /* insert this record into the mailbox_context map. */
    retval = rbtree_insert(ctx->mailbox_context_tree, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_mailbox_context;
    }

    /* bump the reference count. */
    tmp->reference_count += 1;

    /* insert this record into the context_mailbox map. */
    retval = rbtree_insert(ctx->context_mailbox_tree, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto remove_mailbox_context_entry;
    }

    /* on success, this map owns our reference. */
    tmp = NULL;

    /* create the response message. */
    retval =
        protocolservice_dataservice_response_message_create(
            reply_payload, ctx, req_payload->request_id, STATUS_SUCCESS,
            req_payload->offset, NULL);
    if (STATUS_SUCCESS != retval)
    {
        goto remove_context_mailbox_entry;
    }

    /* success. */
    goto done;

remove_context_mailbox_entry:
    release_retval = rbtree_delete(NULL, ctx->context_mailbox_tree, &child);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

remove_mailbox_context_entry:
    release_retval =
        rbtree_delete(NULL, ctx->mailbox_context_tree, &return_address);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_mailbox_context:
    if (NULL != tmp)
    {
        release_retval = resource_release(&tmp->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
        tmp = NULL;
    }

done:
    return retval;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
