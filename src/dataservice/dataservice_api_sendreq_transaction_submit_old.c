/**
 * \file dataservice/dataservice_api_sendreq_transaction_submit_old.c
 *
 * \brief Submit a transaction to the transaction queue.
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

RCPR_IMPORT_uuid;

/**
 * \brief Submit a transaction to the transaction queue.
 *
 * \param sock          The socket on which this request is made.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child index used for this operation.
 * \param txn_id        The transaction UUID bytes for this transaction.
 * \param artifact_id   The artifact UUID bytes for this transaction.
 * \param val           Buffer holding the raw bytes for the transaction cert.
 * \param val_size      The size of this transaction cert.
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
int dataservice_api_sendreq_transaction_submit_old(
    ipc_socket_context_t* sock, allocator_options_t* alloc_opts, uint32_t child,
    const uint8_t* txn_id, const uint8_t* artifact_id, const void* val,
    uint32_t val_size)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != txn_id);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != val);

    /* encode this request to a buffer. */
    retval =
        dataservice_encode_request_transaction_submit(
            &reqbuf, alloc_opts, child, (const rcpr_uuid*)txn_id,
            (const rcpr_uuid*)artifact_id, val, val_size);
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

    /* clean up the buffer. */
    dispose((disposable_t*)&reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
