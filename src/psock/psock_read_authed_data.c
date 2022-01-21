/**
 * \file psock/psock_read_authed_data.c
 *
 * \brief Read an encrypted and authenticated packet from the psock stream.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <string.h>
#include <vccrypt/compare.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Read an authenticated data packet.
 *
 * On success, an authenticated data buffer is allocated and read, along with
 * type information and size.  The caller owns this buffer and is responsible
 * for freeing it when it is no longer in use.
 *
 * \param sock          The psock instance from which the packet is read.
 * \param alloc         The allocator to use for this function.
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
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t iv,
    void** val, uint32_t* size, vccrypt_suite_options_t* suite,
    vccrypt_buffer_t* secret)
{
    status retval = 0;
    uint32_t type = 0U;
    uint32_t nsize = 0U;
    uint8_t* header = NULL;
    uint8_t* dheader = NULL;
    size_t read_size = 0U;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(prop_vccrypt_suite_options_valid(suite));
    MODEL_ASSERT(prop_vccrypt_buffer_valid(secret));

    /* attempt to allocate space for the header. */
    const size_t header_size =
        sizeof(type) + sizeof(nsize) + suite->mac_short_opts.mac_size;
    vccrypt_buffer_t hbuffer;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&hbuffer, suite->alloc_opts, header_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* attempt to allocate space for the decrypted header. */
    const size_t dheader_size =
        sizeof(type) + sizeof(nsize);
    vccrypt_buffer_t dhbuffer;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&dhbuffer, suite->alloc_opts, dheader_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_hbuffer;
    }

    /* set up pointers for convenience. */
    header = (uint8_t*)hbuffer.data;
    dheader = (uint8_t*)dhbuffer.data;

    /* attempt to read the header. */
    void* data = NULL;
    retval = psock_read_raw_data(sock, alloc, &data, header_size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
        goto cleanup_dhbuffer;
    }

    /* copy the data. */
    memcpy(header, data, header_size);

    /* free the memory. */
    retval = rcpr_allocator_reclaim(alloc, data);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_dhbuffer;
    }

    /* set up the stream cipher. */
    vccrypt_stream_context_t stream;
    retval = vccrypt_suite_stream_init(suite, &stream, secret);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_dhbuffer;
    }

    /* set up the MAC. */
    vccrypt_mac_context_t mac;
    retval = vccrypt_suite_mac_short_init(suite, &mac, secret);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_stream;
    }

    /* start decryption of the stream. */
    retval = vccrypt_stream_continue_decryption(&stream, &iv, sizeof(iv), 0);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* decrypt enough of the header to determine the type and size. */
    size_t offset = 0;
    retval =
        vccrypt_stream_decrypt(
            &stream, hbuffer.data, dheader_size, dhbuffer.data, &offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* verify that the type is AGENTD_PSOCK_BOXED_TYPE_AUTHED_PACKET. */
    memcpy(&type, dheader, sizeof(type));
    if (AGENTD_PSOCK_BOXED_TYPE_AUTHED_PACKET != ntohl(type))
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_mac;
    }

    /* verify that the size makes sense. */
    memcpy(&nsize, dheader + sizeof(type), sizeof(nsize));
    *size = ntohl(nsize);
    if (*size > 10ULL * 1024ULL * 1024ULL /* 10 MB */)
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_mac;
    }

    /* create a payload buffer for holding the encrypted payload. */
    vccrypt_buffer_t payload;
    retval = vccrypt_buffer_init(&payload, suite->alloc_opts, *size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_mac;
    }

    /* read the payload. */
    read_size = *size;
    retval = psock_read_raw(sock, payload.data, &read_size);
    if (VCCRYPT_STATUS_SUCCESS != retval || *size != read_size)
    {
        retval = AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
        goto cleanup_payload;
    }

    /* digest the packet. */
    retval = vccrypt_mac_digest(&mac, hbuffer.data, dheader_size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_payload;
    }

    retval = vccrypt_mac_digest(&mac, payload.data, *size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_payload;
    }

    /* create a buffer to hold the digest. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    retval =
        vccrypt_buffer_init(
            &digest, suite->alloc_opts, suite->mac_short_opts.mac_size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_payload;
    }

    /* finalize the mac. */
    retval = vccrypt_mac_finalize(&mac, &digest);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_digest;
    }

    /* compare the digest against the mac in the packet. */
    if (0 !=
        crypto_memcmp(
            digest.data, header + sizeof(type) + sizeof(nsize),
            digest.size))
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_digest;
    }

    /* the payload has been authenticated.  create output buffer. */
    *val = malloc(*size);
    if (NULL == *val)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_digest;
    }

    /* continue decryption in the payload. */
    retval =
        vccrypt_stream_continue_decryption(
            &stream, &iv, sizeof(iv), offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_val;
    }

    /* reset the offset. */
    offset = 0;

    /* decrypt the payload. */
    retval =
        vccrypt_stream_decrypt(
            &stream, payload.data, *size, *val, &offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_val;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_digest;

cleanup_val:
    memset(*val, 0, *size);
    free(*val);
    *val = NULL;

cleanup_digest:
    dispose((disposable_t*)&digest);

cleanup_payload:
    dispose((disposable_t*)&payload);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_stream:
    dispose((disposable_t*)&stream);

cleanup_dhbuffer:
    dispose((disposable_t*)&dhbuffer);

cleanup_hbuffer:
    dispose((disposable_t*)&hbuffer);

done:
    return retval;
}
