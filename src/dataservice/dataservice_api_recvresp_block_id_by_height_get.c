/**
 * \file dataservice/dataservice_api_recvresp_block_id_by_height_get.c
 *
 * \brief Read the response from the block get by height call.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/inet.h>
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
 * \brief Receive a response from the get block id by height query.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc         The allocator to use for this operation.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param block_id      Pointer to the 16 byte array to receive the block id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the block id array is updated with the UUID for the block associated
 * with the height provided when the original request was sent.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the requested data was not
 *        found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this client node is not
 *        authorized to perform the requested operation.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX if the child context
 *        index is out of bounds.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID if the child context is
 *        invalid.
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
int dataservice_api_recvresp_block_id_by_height_get(
    psock* sock, rcpr_allocator* alloc, uint32_t* offset, uint32_t* status,
    uint8_t* block_id)
{
    int retval = 0, release_retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != block_id);

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
    dataservice_response_block_id_by_height_get_t dresp;
    retval =
        dataservice_decode_response_block_id_by_height_get(val, size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = dresp.hdr.offset;

    /* get the status code. */
    *status = dresp.hdr.status;

    /* if the status code is successful, then the block id will be in this
     * payload. */
    if (AGENTD_STATUS_SUCCESS != (int)dresp.hdr.status)
        goto cleanup_dresp;

    /* copy the block id. */
    memcpy(block_id, dresp.block_id, 16);

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
        retval = release_retval;
    }

done:
    return retval;
}
