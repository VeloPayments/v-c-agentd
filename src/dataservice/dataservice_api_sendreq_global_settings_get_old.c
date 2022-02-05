/**
 * \file dataservice/dataservice_api_sendreq_global_settings_get_old.c
 *
 * \brief Request the query of a global settings value.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <agentd/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Query a global setting using the given child context.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc_opts    The allocator to use for this operation.
 * \param child         The child index used for the query.
 * \param key           The global key to query.
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
int dataservice_api_sendreq_global_settings_get_old(
    ipc_socket_context_t* sock, allocator_options_t* alloc_opts, uint32_t child,
    uint64_t key)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* encode this request. */
    retval =
        dataservice_encode_request_global_settings_get(
            &reqbuf, alloc_opts, child, key);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* the request packet consists of the command, index, and key. */
    retval = ipc_write_data_noblock(sock, reqbuf.data, reqbuf.size);
    if (AGENTD_STATUS_SUCCESS != retval && AGENTD_ERROR_IPC_WOULD_BLOCK != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up the buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
