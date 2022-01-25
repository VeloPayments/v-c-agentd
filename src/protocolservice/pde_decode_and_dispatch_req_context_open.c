/**
 * \file
 * protocolservice/pde_decode_and_dispatch_req_context_open.c
 *
 * \brief Decode and dispatch a context open request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

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
    /* TODO - implement. For now, just mark it as an invalid request. */
    return
        pde_decode_and_dispatch_invalid_req(
            ctx, req_payload, return_address, reply_payload);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
