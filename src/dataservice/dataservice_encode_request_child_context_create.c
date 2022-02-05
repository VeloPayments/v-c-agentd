/**
 * \file dataservice/dataservice_encode_request_child_context_create.c
 *
 * \brief Encode a request to create a child context.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

/**
 * \brief Encode a request to create a child context.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param caps          Pointer to the capabilities buffer.
 * \param size          Size of the capabilities buffer.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_child_context_create(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, const void* caps,
    size_t size)
{
    status retval;
    vccrypt_buffer_t tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));
    MODEL_ASSERT(NULL != caps);

    /* runtime parameter sanity checks. */
    if (NULL == buffer || NULL == alloc_opts || NULL == caps)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* | Child context create packet.                                 | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE| 4 bytes      | */
    /* | caps                                          | n - 4 bytes  | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the request buffer size. */
    size_t reqbuflen =
        sizeof(uint32_t) + size;

    /* create a buffer for holding the request. */
    retval = vccrypt_buffer_init(&tmp, alloc_opts, reqbuflen);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* make working with the buffer more convenient. */
    uint8_t* breq = (uint8_t*)tmp.data;

    /* copy the request id to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE);
    memcpy(breq, &req, sizeof(req));
    breq += sizeof(req);

    /* copy the caps parameter to this buffer. */
    memcpy(breq, caps, size);
    breq += size;

    /* move the contents of the temporary buffer to the return buffer. */
    vccrypt_buffer_move(buffer, &tmp);

    /* success. */
    return STATUS_SUCCESS;
}
