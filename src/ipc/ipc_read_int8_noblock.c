/**
 * \file ipc/ipc_read_int8_noblock.c
 *
 * \brief Non-blocking read of an int8 value.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Read an int8_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_int8_noblock(ipc_socket_context_t* sock, int8_t* val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->readbuf);
    MODEL_ASSERT(NULL != val);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* compute the header size. */
    ssize_t header_sz = sizeof(uint8_t) + sizeof(uint32_t);

    /* we need the header data. */
    uint8_t* mem = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, header_sz);
    if (NULL == mem)
    {
        return AGENTD_ERROR_IPC_WOULD_BLOCK;
    }

    /* if the type does not match our expected type, return an error. */
    if (IPC_DATA_TYPE_INT8 != mem[0])
    {
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
    }

    /* decode the size of this packet. */
    uint32_t nsize = 0;
    memcpy(&nsize, mem + 1, sizeof(uint32_t));

    /* sanity check on size. */
    uint32_t size = ntohl(nsize);
    if (size != sizeof(int8_t))
    {
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
    }

    /* if the buffer size is less than this size, wait for more data to be
     * available. */
    if (evbuffer_get_length(sock_impl->readbuf) < (size + (size_t)header_sz))
    {
        return AGENTD_ERROR_IPC_WOULD_BLOCK;
    }

    /* drain the header from the buffer. */
    if (header_sz != evbuffer_drain(sock_impl->readbuf, header_sz))
    {
        return AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE;
    }

    /* read the data from the buffer. */
    if (size != (size_t)evbuffer_remove(sock_impl->readbuf, val, size))
    {
        return AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
