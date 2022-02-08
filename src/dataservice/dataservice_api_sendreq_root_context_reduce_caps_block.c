/**
 * \file dataservice/dataservice_api_sendreq_root_context_reduce_caps_block.c
 *
 * \brief Request that the capabilities of the root context be reduced, using a
 * blocking socket.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Request that the capabilities of the root context be reduced.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc         The allocator to use for this operation.
 * \param caps          The capabilities to use for the reduction.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        capabilities size is invalid.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int dataservice_api_sendreq_root_context_reduce_caps_block(
    int sock, allocator_options_t* alloc_opts, uint32_t* caps, size_t size)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != caps);
    MODEL_ASSERT(size == sizeof(bitcaps));

    /* encode this request. */
    retval =
        dataservice_encode_request_root_context_reduce_caps(
            &reqbuf, alloc_opts, caps, size);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the request packet to the socket. */
    retval = ipc_write_data_block(sock, reqbuf.data, reqbuf.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up the buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
