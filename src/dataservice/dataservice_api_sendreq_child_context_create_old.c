/**
 * \file dataservice/dataservice_api_sendreq_child_context_create_old.c
 *
 * \brief Request the creation of a child context.
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
 * \brief Create a child context with further reduced capabilities.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc_opts    The allocator to use for this operation.
 * \param caps          The capabilities to use for this child context.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this write operation would block this
 *        thread.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int dataservice_api_sendreq_child_context_create_old(
    ipc_socket_context_t* sock, allocator_options_t* alloc_opts,
    const void* caps, size_t size)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != caps);

    /* encode this request. */
    retval =
        dataservice_encode_request_child_context_create(
            &reqbuf, alloc_opts, caps, size);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the request packet to the socket. */
    retval = ipc_write_data_noblock(sock, reqbuf.data, reqbuf.size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK != retval && AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up the buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
