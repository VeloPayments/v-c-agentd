/**
 * \file ipc/ipc_read_string_block.c
 *
 * \brief Blocking read of a string value.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Read a character string from the blocking socket.
 *
 * On success, a character string value is allocated and read, along with type
 * information and size.  The caller owns this character string and is
 * responsible for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to the string pointer to hold the string value
 *                      on success.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size is too
 *        large.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int ipc_read_string_block(int sock, char** val)
{
    uint32_t type = 0U;
    uint32_t nsize = 0U;
    uint32_t size = 0U;

    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != val);

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* verify that the type is IPC_DATA_TYPE_STRING. */
    if (IPC_DATA_TYPE_STRING != ntohl(type))
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* convert the size to host byte order. */
    size = ntohl(nsize);

    /* cap the maximum string size at 10 MB. */
    if (size > (10 * 1024 * 1024))
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;

    /* attempt to allocate memory for this string. */
    *val = (char*)malloc(size + 1);
    if (NULL == *val)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* attempt to read the string. */
    if (size != read(sock, *val, size))
    {
        free(*val);
        *val = NULL;
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
    }

    /* set the asciiz. */
    (*val)[size] = 0;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
