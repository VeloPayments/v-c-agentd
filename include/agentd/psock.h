/**
 * \file agentd/psock.h
 *
 * \brief psock helpers for agentd.
 *
 * This includes psock functionality that we need for agentd that isn't provided
 * by RCPR, namely, auth data packet I/O.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <rcpr/psock.h>
#include <vccrypt/suite.h>

/* C++ compatibility. */
# ifdef   __cplusplus
extern "C" {
# endif /*__cplusplus*/

enum agentd_psock_boxed_type
{
    AGENTD_PSOCK_BOXED_TYPE_AUTHED_PACKET       = 0x00000030,
};

/**
 * \brief Write an authenticated data packet.
 *
 * On success, the authenticated data packet value will be written, along with
 * type information and size.
 *
 * \param sock          The psock instance to which this packet is written.
 * \param iv            The 64-bit IV to use for this packet.
 * \param val           The payload data to write.
 * \param size          The size of the payload data to write.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 */
int psock_write_authed_data(
    RCPR_SYM(psock)* sock, uint64_t iv, const void* val, uint32_t size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/**
 * \brief Read an authenticated data packet.
 *
 * On success, an authenticated data buffer is allocated and read, along with
 * type information and size.  The caller owns this buffer and is responsible
 * for freeing it when it is no longer in use.
 *
 * \param sock          The psock instance from which the packet is read.
 * \param iv            The 64-bit IV to expect for this packet.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 *      - AGENTD_ERROR_IPC_AUTHENTICATION_FAILURE if the packet could not be
 *        authenticated.
 */
int psock_read_authed_data(
    RCPR_SYM(psock)* sock, uint64_t iv, void** val, uint32_t* size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/* C++ compatibility. */
# ifdef   __cplusplus
}
# endif /*__cplusplus*/
