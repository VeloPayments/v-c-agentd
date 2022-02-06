/**
 * \file dataservice/dataservice_encode_request_root_context_init.c
 *
 * \brief Encode the root context init request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>

/**
 * \brief Encode a request to create the root dataservice context.
 *
 * \param buffer                Pointer to an uninitialized
 *                              \ref vccrypt_buffer_t to receive the encoded
 *                              request.
 * \param alloc_opts            The allocator options to use.
 * \param max_database_size     The maximum database size.
 * \param datadir               The data directory to open.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_root_context_init(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts,
    uint64_t max_database_size, const char* datadir)
{
    status retval;
    vccrypt_buffer_t tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));
    MODEL_ASSERT(NULL != datadir);

    /* runtime parameter sanity checks. */
    if (NULL == buffer || NULL == alloc_opts || NULL == datadir)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* | Root context init request packet.                            | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE | 4 bytes      | */
    /* | max database size                             | 8 bytes      | */
    /* | datadir                                       | n - 4 bytes  | */
    /* | --------------------------------------------- | ------------ | */

    /* get the datadir length. */
    size_t datadirlen = strlen(datadir);

    /* compute the request buffer size. */
    size_t reqbuflen =
        sizeof(uint32_t)            /* request id */
      + sizeof(max_database_size)
      + datadirlen;

    /* create a buffer for holding the request. */
    retval = vccrypt_buffer_init(&tmp, alloc_opts, reqbuflen);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* make working with the buffer more convenient. */
    uint8_t* breq = (uint8_t*)tmp.data;

    /* copy the request id to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE);
    memcpy(breq, &req, sizeof(req));
    breq += sizeof(req);

    /* copy the max database size parameter to this buffer. */
    uint64_t net_max_database_size = htonll(max_database_size);
    memcpy(breq, &net_max_database_size, sizeof(net_max_database_size));
    breq += sizeof(net_max_database_size);

    /* copy the datadir parameter to this buffer. */
    memcpy(breq, datadir, datadirlen);
    breq += datadirlen;

    /* move the contents of the temporary buffer to the return buffer. */
    vccrypt_buffer_move(buffer, &tmp);

    /* success. */
    return STATUS_SUCCESS;
}
