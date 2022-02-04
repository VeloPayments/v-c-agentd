/**
 * \file dataservice/dataservice_api_sendreq_artifact_get_old.c
 *
 * \brief Get an artifact by id from the artifact database.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_uuid;

/**
 * \brief Get an artifact from the artifact database by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc_opts    The allocator to use for this operation.
 * \param child         The child index used for the query.
 * \param artifact_id   The artifact UUID of the artifact to retrieve.
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
int dataservice_api_sendreq_artifact_get_old(
    ipc_socket_context_t* sock, allocator_options_t* alloc_opts, uint32_t child,
    const uint8_t* artifact_id)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* encode this request to a buffer. */
    retval =
        dataservice_encode_request_artifact_get(
            &reqbuf, alloc_opts, child, (const rcpr_uuid*)artifact_id);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the request packet. */
    retval = ipc_write_data_noblock(sock, reqbuf.data, reqbuf.size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK != retval && AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
