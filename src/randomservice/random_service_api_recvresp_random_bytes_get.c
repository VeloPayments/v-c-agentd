/**
 * \file dataservice/random_service_api_recvresp_random_bytes_get.c
 *
 * \brief Read the response from the random bytes get call.
 *
 * \copyright 2020-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/randomservice.h>
#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/psock.h>
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Receive the response from the random bytes call from the random
 * service.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc         The allocator to use for this operation.
 * \param offset        The offset of the response.
 * \param status        The status of the response.
 * \param bytes         Pointer to receive an allocated buffer of random bytes
 *                      on success.
 * \param bytes_size    The number of bytes received in this buffer on success.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE if an error occurred
 *        when reading from the socket.
 */
int random_service_api_recvresp_random_bytes_get(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint32_t* offset,
    uint32_t* status_, void** bytes, size_t* bytes_size)
{
    status retval, release_retval;

    /* parameter sanity check. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status_);
    MODEL_ASSERT(NULL != bytes);
    MODEL_ASSERT(NULL != bytes_size);

    /* read a data packet from the socket. */
    uint32_t* resp = NULL;
    size_t resp_size = 0U;
    retval = psock_read_boxed_data(sock, alloc, (void*)&resp, &resp_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* verify the size of the response packet. */
    if (resp_size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto cleanup_resp;
    }

    /* decode response packet. */
    uint32_t method_id = ntohl(resp[0]);
    *offset = ntohl(resp[1]);
    *status_ = ntohl(resp[2]);
    void* data = (void*)(resp + 3);
    *bytes_size = resp_size - 3 * sizeof(uint32_t);

    /* sanity check of response from random read. */
    if (
        RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES != method_id
     || AGENTD_STATUS_SUCCESS != *status_
     || 0 == *bytes_size)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_BAD;
        goto cleanup_resp;
    }

    /* allocate memory for the response. */
    retval = rcpr_allocator_allocate(alloc, bytes, *bytes_size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_resp;
    }

    /* copy the bytes. */
    memcpy(*bytes, data, *bytes_size);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_resp:
    memset(resp, 0, resp_size);
    release_retval = rcpr_allocator_reclaim(alloc, resp);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
