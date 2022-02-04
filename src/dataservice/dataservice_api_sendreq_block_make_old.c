/**
 * \file dataservice/dataservice_api_sendreq_block_make_old.c
 *
 * \brief Make a block from transactions in the transaction queue.
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
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_uuid;

/**
 * \brief Make a block from transactions in the transaction queue.
 *
 * Caller submits a valid signed block containing the transactions to drop from
 * the transaction queue.  If this call is successful, then this block and those
 * transactions are canonized.
 *
 * \param sock              The socket on which this request is made.
 * \param alloc_opts        The allocator options to use.
 * \param child             The child index used for this operation.
 * \param txn_id            The block UUID bytes for this transaction.
 * \param block_cert        Buffer holding the raw bytes for the block cert.
 * \param block_cert_size   The size of this block cert.
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
int dataservice_api_sendreq_block_make_old(
    ipc_socket_context_t* sock, allocator_options_t* alloc_opts, uint32_t child,
    const uint8_t* block_id, const void* block_cert, uint32_t block_cert_size)
{
    status retval;
    vccrypt_buffer_t reqbuf;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != block_cert);
    MODEL_ASSERT(block_cert_size > 0);

    /* encode this request to a buffer. */
    retval =
        dataservice_encode_request_block_make(
            &reqbuf, alloc_opts, child, (const rcpr_uuid*)block_id, block_cert,
            block_cert_size);
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
