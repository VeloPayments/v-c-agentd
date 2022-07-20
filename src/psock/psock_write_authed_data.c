/**
 * \file psock/psock_write_authed_data.c
 *
 * \brief Write an encrypted and authenticated packet to the psock stream.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/psock.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <string.h>
#include <vccrypt/compare.h>

RCPR_IMPORT_psock;

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
    vccrypt_suite_options_t* suite, const vccrypt_buffer_t* secret)
{
    status retval = 0;
    uint32_t type = htonl(AGENTD_PSOCK_BOXED_TYPE_AUTHED_PACKET);
    uint32_t nsize = htonl(size);

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(prop_vccrypt_suite_options_valid(suite));
    MODEL_ASSERT(prop_vccrypt_buffer_valid(secret));

    /* create a buffer for holding the digest. */
    vccrypt_buffer_t digest;
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            suite, &digest, true);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* create a packet buffer large enough for this authed packet. */
    size_t packet_size =
        sizeof(type) + sizeof(nsize) + digest.size + size;
    vccrypt_buffer_t packet;
    retval = vccrypt_buffer_init(&packet, suite->alloc_opts, packet_size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_digest;
    }

    /* create a stream cipher for encrypting this packet. */
    vccrypt_stream_context_t stream;
    retval = vccrypt_suite_stream_init(suite, &stream, secret);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_packet;
    }

    /* create a mac instance for building the packet authentication code. */
    vccrypt_mac_context_t mac;
    retval = vccrypt_suite_mac_short_init(suite, &mac, secret);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_stream;
    }

    /* start the stream cipher. */
    retval = vccrypt_stream_continue_encryption(&stream, &iv, sizeof(iv), 0);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* treat the packet as a byte array for convenience. */
    uint8_t* bpacket = (uint8_t*)packet.data;
    size_t offset = 0;

    /* encrypt the type. */
    retval =
        vccrypt_stream_encrypt(&stream, &type, sizeof(type), bpacket, &offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* encrypt the size. */
    retval =
        vccrypt_stream_encrypt(
            &stream, &nsize, sizeof(nsize), bpacket, &offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* encrypt the payload. */
    retval =
        vccrypt_stream_encrypt(
            &stream, val, size, bpacket + digest.size, &offset);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* digest the packet header. */
    retval = vccrypt_mac_digest(&mac, bpacket, sizeof(type) + sizeof(nsize));
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* digest the packet payload. */
    retval =
        vccrypt_mac_digest(
            &mac, bpacket + sizeof(type) + sizeof(nsize) + digest.size, size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* finalize the digest. */
    retval = vccrypt_mac_finalize(&mac, &digest);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* copy the digest to the packet. */
    memcpy(bpacket + sizeof(type) + sizeof(nsize), digest.data, digest.size);

    /* write the packet to the socket. */
    retval = psock_write_raw_data(sock, packet.data, packet.size);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;
        goto cleanup_mac;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_stream:
    dispose((disposable_t*)&stream);

cleanup_packet:
    dispose((disposable_t*)&packet);

cleanup_digest:
    dispose((disposable_t*)&digest);

done:
    return retval;
}
