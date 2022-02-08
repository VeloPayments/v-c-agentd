/**
 * \file dataservice/dataservice_encode_request_transaction_submit.c
 *
 * \brief Encode a transaction submit request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

/**
 * \brief Encode a request to submit a transaction.
 *
 * \param buffer            Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                          receive the encoded request.
 * \param alloc_opts        The allocator options to use.
 * \param child             The child context for this request.
 * \param txn_id            The transaction id.
 * \param artifact_id       The artifact id for this transaction.
 * \param val               The transaction certificate value.
 * \param val_size          The transaction certificate size.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_transaction_submit(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    const RCPR_SYM(rcpr_uuid)* txn_id, const RCPR_SYM(rcpr_uuid)* artifact_id,
    const void* val, uint32_t val_size)
{
    status retval;
    vccrypt_buffer_t tmp;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));
    MODEL_ASSERT(NULL != txn_id);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != val);

    /* runtime parameter sanity checks. */
    if (NULL == buffer || NULL == alloc_opts || NULL == txn_id
     || NULL == artifact_id || NULL == val)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* | Transaction Submit Packet.                                      | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATA                                             | SIZE         | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT | 4 bytes      | */
    /* | child_context_index                              | 4 bytes      | */
    /* | txn_id                                           | 16 bytes     | */
    /* | artifact_id                                      | 16 bytes     | */
    /* | txn_cert                                         | n - 40 bytes | */
    /* | ------------------------------------------------ | ------------ | */

    /* compute the request buffer size. */
    size_t reqbuflen =
        sizeof(uint32_t)    /* request id. */
      + sizeof(child)
      + sizeof(*txn_id)
      + sizeof(*artifact_id)
      + val_size;

    /* create a buffer for holding the request. */
    retval = vccrypt_buffer_init(&tmp, alloc_opts, reqbuflen);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* make working with the buffer more convenient. */
    uint8_t* breq = (uint8_t*)tmp.data;

    /* copy the request id to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT);
    memcpy(breq, &req, sizeof(req));
    breq += sizeof(req);

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(breq, &nchild, sizeof(nchild));
    breq += sizeof(child);

    /* copy the txn id to the buffer. */
    memcpy(breq, txn_id, sizeof(*txn_id));
    breq += sizeof(*txn_id);

    /* copy the artifact id to the buffer. */
    memcpy(breq, artifact_id, sizeof(*artifact_id));
    breq += sizeof(*artifact_id);

    /* copy the certificate to the buffer. */
    memcpy(breq, val, val_size);
    breq += val_size;

    /* move the contents of the temporary buffer to the return buffer. */
    vccrypt_buffer_move(buffer, &tmp);

    /* success. */
    return STATUS_SUCCESS;
}
