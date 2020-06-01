/**
 * \file protocolservice/protocolservice_api_recvresp_status_get.c
 *
 * \brief Receive the status response.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Receive a status get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_status_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status)
{
    int retval;
    uint32_t* val;
    uint32_t size;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != server_id);
    MODEL_ASSERT(NULL != shared_secret);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);

    /* read the response from the server. */
    /* TODO - fix constness in ipc method for shared secret. */
    retval =
        ipc_read_authed_data_block(
            sock, *server_iv, (void**)&val, &size, suite,
            (vccrypt_buffer_t*)shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* update the server_iv on successful read. */
    *server_iv += 1;

    /* verify that the response is the correct size. */
    if (size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto cleanup_val;
    }

    /* verify the request id. */
    if (UNAUTH_PROTOCOL_REQ_ID_STATUS_GET != ntohl(val[0]))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
        goto cleanup_val;
    }

    /* set the status and offset. */
    *status = ntohl(val[1]);
    *offset = ntohl(val[2]);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}
