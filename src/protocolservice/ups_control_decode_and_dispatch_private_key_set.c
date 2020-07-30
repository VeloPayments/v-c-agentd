/**
 * \file protocolservice/ups_control_decode_and_dispatch_private_key_set.c
 *
 * \brief Decode and dispatch the authorized entity add command.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Decode and dispatch a private key set request
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int ups_control_decode_and_dispatch_private_key_set(
    unauthorized_protocol_service_instance_t* instance,
    ipc_socket_context_t* sock, const void* req, size_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* get the expected size for all keys from the crypto suite. */
    const size_t expected_enc_pubkey_size =
        instance->suite.key_cipher_opts.public_key_size;
    const size_t expected_enc_privkey_size =
        instance->suite.key_cipher_opts.private_key_size;
    const size_t expected_sign_pubkey_size =
        instance->suite.sign_opts.public_key_size;
    const size_t expected_sign_privkey_size =
        instance->suite.sign_opts.private_key_size;

    /* compute the message header size. */
    const size_t payload_header_size = 5 * sizeof(uint32_t);

    /* ensure that the payload size is at least large enough to hold this
     * header. */
    if (size < payload_header_size)
    {
        ups_control_decode_and_dispatch_write_status(
            sock, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET, 0U,
            AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL, 0);
        return AGENTD_STATUS_SUCCESS;
    }

    /* treat the request as a byte buffer for convenience. */
    uint8_t* breq = (uint8_t*)req;

    /* get the request offset. */
    uint32_t noffset;
    memcpy(&noffset, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t offset = ntohl(noffset);

    /* get the encryption pubkey size. */
    uint32_t nenc_pubkey_size;
    memcpy(&nenc_pubkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t enc_pubkey_size = ntohl(nenc_pubkey_size);

    /* get the encryption privkey size. */
    uint32_t nenc_privkey_size;
    memcpy(&nenc_privkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t enc_privkey_size = ntohl(nenc_privkey_size);

    /* get the signing pubkey size. */
    uint32_t nsig_pubkey_size;
    memcpy(&nsig_pubkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sig_pubkey_size = ntohl(nsig_pubkey_size);

    /* get the signing privkey size. */
    uint32_t nsig_privkey_size;
    memcpy(&nsig_privkey_size, breq, sizeof(uint32_t));
    breq += sizeof(uint32_t);
    uint32_t sig_privkey_size = ntohl(nsig_privkey_size);

    /* verify key sizes. */
    if (    expected_enc_pubkey_size != enc_pubkey_size
        ||  expected_enc_privkey_size != enc_privkey_size
        ||  expected_sign_pubkey_size != sig_pubkey_size
        ||  expected_sign_privkey_size != sig_privkey_size)
    {
        ups_control_decode_and_dispatch_write_status(
            sock, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET, offset,
            AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL, 0);
        return AGENTD_STATUS_SUCCESS;
    }

    /* compute remaining payload size. */
    const size_t payload_size =
          16 /* entity id */
        + enc_pubkey_size
        + enc_privkey_size
        + sig_pubkey_size
        + sig_privkey_size;

    /* verify that the remaining payload size is large enough. */
    size -= payload_header_size;
    if (size < payload_size)
    {
        ups_control_decode_and_dispatch_write_status(
            sock, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET, offset,
            AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL, 0);
        return AGENTD_STATUS_SUCCESS;
    }

    /* get the id and key buffers. */
    const uint8_t* id = breq; breq += 16;
    const uint8_t* enc_pubkey = breq; breq += enc_pubkey_size;
    const uint8_t* enc_privkey = breq; breq += enc_privkey_size;
    const uint8_t* sig_pubkey = breq; breq += sig_pubkey_size;
    const uint8_t* sig_privkey = breq; breq += sig_privkey_size;

    /* set the private key. */
    int retval =
        ups_private_key_set(
            instance, id, enc_pubkey, enc_privkey, sig_pubkey, sig_privkey);

    /* write the return value of this operation to the control socket. */
    ups_control_decode_and_dispatch_write_status(
        sock, UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET, offset, retval,
        NULL, 0);

    return AGENTD_STATUS_SUCCESS;
}
