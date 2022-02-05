/**
 * \file dataservice/dataservice_encode_request_global_settings_get.c
 *
 * \brief Encode a global settings get request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>

/**
 * \brief Encode a request to query the global settings table.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child context used for this call.
 * \param key           The global key to query.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_global_settings_get(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    uint64_t key)
{
    status retval;
    vccrypt_buffer_t tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));

    /* runtime parameter sanity checks. */
    if (NULL == buffer || NULL == alloc_opts)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* | Global Settings get packet.                                   | */
    /* | ---------------------------------------------- | ------------ | */
    /* | DATA                                           | SIZE         | */
    /* | ---------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ | 4 bytes      | */
    /* | child_context_index                            | 4 bytes      | */
    /* | key                                            | 8 bytes      | */
    /* | ---------------------------------------------- | ------------ | */

    /* compute the request buffer size. */
    size_t reqbuflen =
        sizeof(uint32_t)    /* request id */
      + sizeof(child)
      + sizeof(key);

    /* create a buffer for holding the request. */
    retval = vccrypt_buffer_init(&tmp, alloc_opts, reqbuflen);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* make working with the buffer more convenient. */
    uint8_t* breq = (uint8_t*)tmp.data;

    /* copy the request id to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ);
    memcpy(breq, &req, sizeof(req));
    breq += sizeof(req);

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(breq, &nchild, sizeof(nchild));
    breq += sizeof(nchild);

    /* copy the key to the buffer. */
    uint64_t nkey = htonll(key);
    memcpy(breq, &nkey, sizeof(nkey));
    breq += sizeof(nkey);

    /* move the contents of the temporary buffer to the return buffer. */
    vccrypt_buffer_move(buffer, &tmp);

    /* success. */
    return STATUS_SUCCESS;
}
