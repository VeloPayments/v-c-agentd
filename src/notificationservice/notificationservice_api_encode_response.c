/**
 * \file notificationservice/notificationservice_api_encode_response.c
 *
 * \brief Encode a response from the notification service connection.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Encode a response from the notification service connection, encoding
 * the method_id, the status, and the offset into an allocated buffer.
 *
 * \param buf           Pointer to receive the allocated buffer.
 * \param size          Pointer to receive the buffer size.
 * \param alloc         The allocator to use for this operation.
 * \param method_id     The method id for this response.
 * \param status_code   The status for this response.
 * \param offset        The offset for this response.
 * \param payload       Additional payload for this response.
 * \param payload_size  The size of this additional payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_encode_response(
    uint8_t** buf, size_t* size, RCPR_SYM(allocator)* alloc,
    uint32_t method_id, uint32_t status_code, uint64_t offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval;
    uint8_t* tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != payload);

    /* runtime parameter checks. */
    if (NULL == buf || NULL == size || NULL == alloc)
    {
        return AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
    }

    /* compute the buffer size. */
    size_t buffer_size =
        sizeof(method_id) + sizeof(offset) + sizeof(status_code) + payload_size;

    /* allocate the buffer. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, buffer_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* pointer for convenience. */
    uint8_t* tptr = tmp;

    /* set the method id. */
    uint32_t net_method_id = htonl(method_id);
    memcpy(tptr, &net_method_id, sizeof(net_method_id));
    tptr += sizeof(net_method_id);

    /* set the offset. */
    uint64_t net_offset = htonll(offset);
    memcpy(tptr, &net_offset, sizeof(net_offset));
    tptr += sizeof(net_offset);

    /* set the status. */
    uint32_t net_status = htonl(status_code);
    memcpy(tptr, &net_status, sizeof(net_status));
    tptr += sizeof(net_status);

    /* copy the payload. */
    if (NULL != payload)
    {
        memcpy(tptr, payload, payload_size);
    }

    /* success. Return the buffer. */
    *buf = tmp;
    *size = buffer_size;
    retval = STATUS_SUCCESS;
    goto done;

done:
    return retval;
}
