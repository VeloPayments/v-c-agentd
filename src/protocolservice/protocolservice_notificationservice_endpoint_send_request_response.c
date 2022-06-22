/**
 * \file
 * protocolservice/protocolservice_notificationservice_endpoint_send_request_response.c
 *
 * \brief Send the response to the endpoint request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_message;
RCPR_IMPORT_resource;

/**
 * \brief Send a response for a request sent to the notificationservice.
 *
 * \param ctx           The endpoint context.
 * \param reply_addr    The reply address.
 * \param msg_offset    The server-side offset.
 * \param success       Flag indicating success or failure.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_notificationservice_endpoint_send_request_response(
    protocolservice_notificationservice_fiber_context* ctx,
    RCPR_SYM(mailbox_address) reply_addr, uint64_t msg_offset, bool success)
{
    status retval, release_retval;
    protocolservice_notificationservice_block_assertion_response* tmp;
    message* reply_msg;
    
    /* parameter sanity checks. */
    MODEL_ASSERT(
        prop_protocolservice_notificationservice_fiber_context_valid(ctx));
    MODEL_ASSERT(msg_offset > 0);
    MODEL_ASSERT(reply_addr > 0);

    /* create the response. */
    retval =
        protocolservice_notificationservice_block_assertion_response_create(
            &tmp, ctx->alloc, msg_offset, success);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create the response message. */
    retval =
        message_create(&reply_msg, ctx->alloc, ctx->notify_addr, &tmp->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* the message payload is now owned by the message. */
    tmp = NULL;

    /* send the response message. */
    retval = message_send(reply_addr, reply_msg, ctx->msgdisc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_reply_msg;
    }

    /* the reply message is now owned by the message discipline. */
    reply_msg = NULL;

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_reply_msg:
    if (NULL != reply_msg)
    {
        release_retval = resource_release(message_resource_handle(reply_msg));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_tmp:
    if (NULL != tmp)
    {
        release_retval = resource_release(&tmp->hdr);
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

done:
    return retval;
}
