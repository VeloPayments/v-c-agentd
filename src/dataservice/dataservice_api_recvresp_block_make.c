/**
 * \file dataservice/dataservice_api_recvresp_block_make.c
 *
 * \brief Read the response from the block make call.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/psock.h>
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Receive a response from the block make operation.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc         The allocator to use for this operation.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this client node is not
 *        authorized to perform the requested operation.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_BLOCK_HEIGHT if the
 *        block height for this block was not valid.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_PREVIOUS_BLOCK_UUID if
 *        the previous block uuid was not valid for this block.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_BLOCK_UUID if the block
 *        uuid for this block was missing or already exists.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_NO_CHILD_TRANSACTIONS
 *        if no child transactions were included in this block.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_BLOCK_INSERTION_FAILURE if the
 *        block could not be inserted.
 *      - AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CHILD_TRANSACTION_FAILURE if the
 *        processing a child transaction failed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the operation was halted because it
 *        would block this thread.
 *      - AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE if reading data from
 *        the socket failed.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if the
 *        data packet size is unexpected.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int dataservice_api_recvresp_block_make(
    psock* sock, rcpr_allocator* alloc, uint32_t* offset,
    uint32_t* status)
{
    int retval = 0, release_retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);

    /* read a data packet from the socket. */
    uint32_t* val = NULL;
    size_t size = 0U;
    retval = psock_read_boxed_data(sock, alloc, (void**)&val, &size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* decode the response. */
    dataservice_response_block_make_t dresp;
    retval =
        dataservice_decode_response_block_make(val, size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = dresp.hdr.offset;

    /* get the status code. */
    *status = dresp.hdr.status;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_dresp;

cleanup_dresp:
    dispose((disposable_t*)&dresp);

cleanup_val:
    memset(val, 0, size);
    release_retval = rcpr_allocator_reclaim(alloc, val);
    if (STATUS_SUCCESS != release_retval)
    {
        release_retval = retval;
    }

done:
    return retval;
}
