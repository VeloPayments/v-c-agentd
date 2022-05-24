/**
 * \file
 * protocolservice/protocolservice_control_api_recvresp_authorized_entity_capability_add.c
 *
 * \brief Receive the response for the authorized entity capability add control
 * command.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/control_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Receive a response from the authorized entity add capability request.
 *
 * \param sock                      The socket from which this response is read.
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
int protocolservice_control_api_recvresp_authorized_entity_capability_add(
    int sock, uint32_t* offset, uint32_t* status)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);

    /* read the response from the server. */
    uint32_t* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_block(sock, (void*)&val, &size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* compute the expected size of the response packet. */
    uint32_t response_packet_size =
          /* size of the API method. */
          sizeof(uint32_t)
          /* size of the offset. */
        + sizeof(uint32_t)
          /* size of the status. */
        + sizeof(uint32_t);
    if (size != response_packet_size)
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t method = ntohl(val[0]);
    if (UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_CAP_ADD != method)
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(val[1]);

    /* get the status. */
    *status = ntohl(val[2]);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
