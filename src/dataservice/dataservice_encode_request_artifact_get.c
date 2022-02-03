/**
 * \file dataservice/dataservice_encode_request_artifact_get.c
 *
 * \brief Encode a get artifact by id request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

/**
 * \brief Encode a request to query an artifact by ID.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child context for this request.
 * \param artifact_id   The artifact UUID for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_artifact_get(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    const RCPR_SYM(rcpr_uuid)* artifact_id)
{
    status retval;
    vccrypt_buffer_t tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));
    MODEL_ASSERT(NULL != artifact_id);

    /* runtime parameter sanity checks. */
    if (NULL == buffer || NULL == alloc_opts || NULL == artifact_id)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* compute the request buffer size. */
    size_t reqbuflen =
        sizeof(uint32_t)  /* request id */
      + sizeof(child)
      + sizeof(*artifact_id);

    /* create a buffer for holding the request. */
    retval = vccrypt_buffer_init(&tmp, alloc_opts, reqbuflen);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* make working with the buffer more convenient. */
    uint8_t* breq = (uint8_t*)tmp.data;

    /* copy the request id to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_ARTIFACT_READ);
    memcpy(breq, &req, sizeof(req));
    breq += sizeof(req);

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(breq, &nchild, sizeof(nchild));
    breq += sizeof(nchild);

    /* copy the artifact id to this buffer. */
    memcpy(breq, artifact_id, sizeof(*artifact_id));

    /* move the contents of the temporary buffer to the return buffer. */
    vccrypt_buffer_move(buffer, &tmp);

    /* success. */
    return STATUS_SUCCESS;
}
