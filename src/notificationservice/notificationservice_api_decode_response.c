/**
 * \file notificationservice/notificationservice_api_decode_response.c
 *
 * \brief Decode a response from the notification service connection.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Decode a response from the notification service connection, decoding
 * the method_id, the status, and the offset.
 *
 * \param buf           The buffer to decode.
 * \param size          The size of this buffer.
 * \param method_id     Pointer to receive the method id.
 * \param status_code   Pointer to receive the status.
 * \param offset        Pointer to receive the offset.
 * \param payload       Pointer to be updated to the start of the payload.
 * \param payload_size  Pointer to receive the size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_decode_response(
    const uint8_t* buf, size_t size, uint32_t* method_id, uint32_t* status_code,
    uint64_t* offset, const uint8_t** payload, size_t* payload_size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != method_id);
    MODEL_ASSERT(NULL != status_code);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);

    /* runtime parameter checks. */
    if (NULL == buf || NULL == method_id || NULL == status_code
     || NULL == offset || NULL == payload || NULL == payload_size)
    {
        return AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
    }

    /* verify that the buffer size is large enough at least for decoding the
     * parameters. */
    size_t computed_header_size =
        sizeof(*method_id) + sizeof(*offset) + sizeof(*status_code);
    if (size < computed_header_size)
    {
        return AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
    }

    /* decode the method id. */
    uint32_t net_method_id;
    memcpy(&net_method_id, buf, sizeof(net_method_id));
    buf += sizeof(net_method_id); size -= sizeof(net_method_id);
    *method_id = ntohl(net_method_id);

    /* decode the offset. */
    uint64_t net_offset;
    memcpy(&net_offset, buf, sizeof(net_offset));
    buf += sizeof(net_offset); size -= sizeof(net_offset);
    *offset = ntohll(net_offset);

    /* decode the status code. */
    uint32_t net_status;
    memcpy(&net_status, buf, sizeof(net_status));
    buf += sizeof(net_status); size -= sizeof(net_status);
    *status_code = ntohl(net_status);

    /* if the size is not zero, then set the payload. */
    if (size > 0)
    {
        *payload = buf;
        *payload_size = size;
    }
    else
    {
        *payload = NULL;
        *payload_size = 0U;
    }

    /* success. */
    return STATUS_SUCCESS;
}
