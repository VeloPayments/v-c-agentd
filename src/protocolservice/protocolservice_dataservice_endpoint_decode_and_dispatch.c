/**
 * \file
 * protocolservice/protocolservice_dataservice_endpoint_decode_and_dispatch.c
 *
 * \brief Decode and dispatch a dataservice endpoint request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * \brief Decode and dispatch a dataservice endpoint request.
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
status protocolservice_dataservice_endpoint_decode_and_dispatch(
    protocolservice_dataservice_endpoint_context* ctx,
    protocolservice_dataservice_request_message* req_payload,
    RCPR_SYM(mailbox_address) return_address,
    protocolservice_dataservice_response_message** reply_payload)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));
    MODEL_ASSERT(
        prop_protocolservice_dataservice_request_message_valid(req_payload));
    MODEL_ASSERT(return_address > 0);
    MODEL_ASSERT(NULL != reply_payload);

    switch (req_payload->request_id)
    {
        case PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_CONTEXT_OPEN:
            return
                pde_decode_and_dispatch_req_context_open(
                    ctx, req_payload, return_address, reply_payload);

        case PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_CONTEXT_CLOSE:
            return
                pde_decode_and_dispatch_req_context_close(
                    ctx, req_payload, return_address, reply_payload);

        case PROTOCOLSERVICE_DATASERVICE_ENDPOINT_REQ_DATASERVICE_REQ:
            return pde_decode_and_dispatch_req_dataservice_req(
                    ctx, req_payload, return_address, reply_payload);

        default:
            return pde_decode_and_dispatch_invalid_req(
                    ctx, req_payload, return_address, reply_payload);
    }
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
