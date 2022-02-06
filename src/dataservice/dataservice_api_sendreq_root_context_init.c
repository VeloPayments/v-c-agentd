/**
 * \file dataservice/dataservice_api_sendreq_root_context_init.c
 *
 * \brief Request the creation of a root data service context.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/psock.h>
#include <unistd.h>
#include <vcblockchain/byteswap.h>
#include <vpr/parameters.h>

RCPR_IMPORT_psock;

/**
 * \brief Request the creation of a root data service context.
 *
 * \param sock              The socket on which this request is made.
 * \param alloc_opts        The allocator to use for this operation.
 * \param max_database_size The maximum size of the database.
 * \param datadir           The data directory to open.
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
int dataservice_api_sendreq_root_context_init(
    RCPR_SYM(psock)* sock, allocator_options_t* alloc_opts,
    uint64_t max_database_size, const char* datadir)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != datadir);

    /* encode this request. */
    retval =
        dataservice_encode_request_root_context_init(
            &reqbuf, alloc_opts, max_database_size, datadir);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* write the request packet to the socket. */
    retval = psock_write_boxed_data(sock, reqbuf.data, reqbuf.size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up the buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
