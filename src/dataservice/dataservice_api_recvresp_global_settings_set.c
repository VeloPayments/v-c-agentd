/**
 * \file dataservice/dataservice_api_recvresp_global_settings_set.c
 *
 * \brief Read the response from the global settings set call.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a response from the global settings set operation.
 *
 * \param sock          The socket on which this request is made.
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
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the requested global setting was
 *        not found.
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
int dataservice_api_recvresp_global_settings_set(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != status);

    /* | Global Settings set response packet.                               | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE     | 4 bytes      | */
    /* | offset                                              | 4 bytes      | */
    /* | status                                              | 4 bytes      | */
    /* | --------------------------------------------------- | ------------ | */

    /* read a data packet from the socket. */
    uint32_t* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_noblock(sock, (void**)&val, &size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        goto done;
    }
    else if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* the size should be greater than or equal to the size we respect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t);
    if (size != response_packet_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE != code)
    {
        retval = AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(val[1]);

    /* get the status code. */
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
