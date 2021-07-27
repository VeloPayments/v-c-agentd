/**
 * \file ipc/ipc_write_uint8_block.c
 *
 * \brief Blocking write of a uint8_t value.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/inet.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Write a uint8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_uint8_block(int sock, uint8_t val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);

    uint32_t typeval = htonl(IPC_DATA_TYPE_UINT8);

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the value to the socket. */
    if (sizeof(val) != write(sock, &val, sizeof(val)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
