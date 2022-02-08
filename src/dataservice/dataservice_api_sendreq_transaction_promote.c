/**
 * \file dataservice/dataservice_api_sendreq_transaction_promote.c
 *
 * \brief Promote a transaction by id from the transaction queue.
 *
 * \copyright 2020-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/psock.h>
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_psock;
RCPR_IMPORT_uuid;

/**
 * \brief Promote a transaction from the transaction queue by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc_opts    The allocator options to use for this operation.
 * \param child         The child index used for the query.
 * \param txn_id        The transaction UUID of the transaction to promote.
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
int dataservice_api_sendreq_transaction_promote(
    RCPR_SYM(psock)* sock, allocator_options_t* alloc_opts, uint32_t child,
    const uint8_t* txn_id)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* encode this request. */
    retval =
        dataservice_encode_request_transaction_promote(
            &reqbuf, alloc_opts, child, (const rcpr_uuid*)txn_id);
    if (STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* the request packet consists of the command, index, and transaction id. */
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
