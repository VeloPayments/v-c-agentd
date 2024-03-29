/**
 * \file
 * protocolservice/protocolservice_api_recvresp_block_id_by_height_get_block.c
 *
 * \brief Receive the block id by height get response.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a block id by height get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param block_id                  A pointer to the buffer to receive the
 *                                  block ID.  Should NOT BE initialized
 *                                  by the caller / will be valid if both the
 *                                  call succeeds and the remote call to the
 *                                  server succeeds.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * If this call succeeds AND the remote call to the server succeeds, then
 * block_id will be initialized with the the UUID from the server.  This buffer
 * is owned by the caller and must be disposed when no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_block_id_by_height_get_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    vccrypt_buffer_t* block_id)
{
    bool cleanup_block_id = false;
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != server_iv);
    MODEL_ASSERT(NULL != shared_secret);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != block_id);

    /* allocate a buffer for storing the block_id. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            block_id, suite->alloc_opts, 16))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* if anything fails below, clean up the block_id. */
    cleanup_block_id = true;

    /* read the response from the server. */
    uint32_t* val;
    uint32_t size;
    retval =
        ipc_read_authed_data_block(
            sock, *server_iv, (void**)&val, &size, suite, shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_block_id;
    }

    /* update the server_iv on sucessful read. */
    *server_iv += 1;

    /* verify that the response is the correct size. */
    if (size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto cleanup_val;
    }

    /* verify the request id. */
    if (UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_BY_HEIGHT_GET != ntohl(val[0]))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
        goto cleanup_val;
    }

    /* set the status and offset. */
    *status = ntohl(val[1]);
    *offset = ntohl(val[2]);
    /* decrement size. */
    size -= 3 * sizeof(uint32_t);

    /* was the status successful? */
    if (AGENTD_STATUS_SUCCESS != *status)
    {
        retval = AGENTD_STATUS_SUCCESS;
        goto cleanup_val;
    }

    /* verify that there is enough size for the block id. */
    if (size != 16)
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto cleanup_val;
    }

    /* copy the block id. */
    memcpy(block_id->data, val + 3, 16);

    /* we want to pass the block_id to the caller. */
    cleanup_block_id = false;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_val;

cleanup_val:
    memset(val, 0, size);
    free(val);

cleanup_block_id:
    if (cleanup_block_id)
    {
        dispose((disposable_t*)block_id);
    }

done:
    return retval;
}
