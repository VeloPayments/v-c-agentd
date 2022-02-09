/**
 * \file
 * protocolservice/pde_decode_and_dispatch_invalid_req.c
 *
 * \brief Manage an invalid request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Report an error for an invalid dataservice endpoint request.
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
status pde_decode_and_dispatch_invalid_req(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) /*return_address*/,
    protocolservice_protocol_write_endpoint_message** reply_payload)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_dataservice_request_message_valid(req_payload));
    MODEL_ASSERT(return_address > 0);
    MODEL_ASSERT(NULL != reply_payload);

    /* create a response message payload. */
    return
        protocolservice_protocol_write_endpoint_message_create(
            reply_payload, ctx,
            PROTOCOLSERVICE_PROTOCOL_WRITE_ENDPOINT_ERROR_MESSAGE,
            req_payload->offset, NULL, 0U);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
