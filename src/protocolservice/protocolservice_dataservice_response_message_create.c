/**
 * \file protocolservice/protocolservice_dataservice_response_message_create.c
 *
 * \brief Create a response message for a protocol service dataservice endpoint
 * request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a dataservice endpoint response message.
 *
 * \param reply_payload     Pointer to the pointer to be updated on success.
 * \param ctx               The endpoint context.
 * \param request_id        The request id.
 * \param status            The status code.
 * \param offset            The offset code.
 * \param payload           The payload data.
 *
 * If \p payload is not NULL, then the data in \p payload is moved into an
 * internal structure that is part of the response message owned by the caller
 * on success. Either on success or failure, \p payload should be disposed
 * after this call.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_response_message_create(
    protocolservice_dataservice_response_message** reply_payload,
    protocolservice_dataservice_endpoint_context* ctx, uint32_t request_id,
    uint32_t status_code, uint32_t offset, vccrypt_buffer_t* payload)
{
    status retval;
    protocolservice_dataservice_response_message* tmp = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != reply_payload);
    MODEL_ASSERT(prop_protocolservice_dataservice_endpoint_context_valid(ctx));

    /* allocate memory for the response message. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* initialize resource. */
    resource_init(
        &tmp->hdr, &protocolservice_dataservice_response_message_release);

    /* set values. */
    tmp->alloc = ctx->alloc;
    tmp->request_id = request_id;
    tmp->status = status_code;
    tmp->offset = offset;

    /* if the payload is set, move it to our payload. */
    if (NULL != payload)
    {
        vccrypt_buffer_move(&tmp->payload, payload);
    }

    /* return this instance. */
    *reply_payload = tmp;

    /* success. */
    return STATUS_SUCCESS;
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
