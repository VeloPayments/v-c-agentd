/**
 * \file ipc/ipc_read_int64_block.c
 *
 * \brief Blocking read of a int64_t value.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Read an int64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_int64_block(int sock, int64_t* val)
{
    uint32_t type = 0U;
    int64_t nval = 0U;

    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != val);

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* verify that the type is IPC_DATA_TYPE_INT64. */
    if (IPC_DATA_TYPE_INT64 != ntohl(type))
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;

    /* convert the size to host byte order. */
    ssize_t size = sizeof(int64_t);

    /* attempt to read the value. */
    if (size != read(sock, &nval, sizeof(int64_t)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* convert this value to host byte order. */
    *val = ntohll(nval);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
