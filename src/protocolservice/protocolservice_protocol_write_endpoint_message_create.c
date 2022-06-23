/**
 * \file
 * protocolservice/protocolservice_protocol_write_endpoint_message_create.c
 *
 * \brief Create a write endpoint message.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;

/**
 * \brief Create a write endpoint message.
 *
 * \param reply_payload     Pointer to the pointer to be updated on success.
 * \param ctx               The endpoint context.
 * \param message_type      The message type.
 * \param original_req_id   The original protocol request id.
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
status protocolservice_protocol_write_endpoint_message_create(
    protocolservice_protocol_write_endpoint_message** reply_payload,
    protocolservice_context* ctx, uint32_t message_type,
    uint32_t original_req_id, uint32_t offset, const void* payload,
    size_t payload_size)
{
    status retval, release_retval;
    protocolservice_protocol_write_endpoint_message* tmp = NULL;

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
        &tmp->hdr, &protocolservice_protocol_write_endpoint_message_release);

    /* set values. */
    tmp->alloc = ctx->alloc;
    tmp->message_type = message_type;
    tmp->original_request_id = original_req_id;
    tmp->offset = offset;

    /* if the payload is set, move it to our payload. */
    if (NULL != payload)
    {
        retval =
            vccrypt_buffer_init(&tmp->payload, &ctx->vpr_alloc, payload_size);
        if (STATUS_SUCCESS != retval)
        {
            release_retval = rcpr_allocator_reclaim(ctx->alloc, tmp);
            if (STATUS_SUCCESS != release_retval)
            {
                retval = release_retval;
            }

            return retval;
        }

        memcpy(tmp->payload.data, payload, payload_size);
    }

    /* return this instance. */
    *reply_payload = tmp;

    /* success. */
    return STATUS_SUCCESS;
}
