/**
 * \file agentd/protocolservice/control_api.h
 *
 * \brief Control API for the protocol service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONTROL_PROTOCOLSERVICE_API_HEADER_GUARD
#define AGENTD_CONTROL_PROTOCOLSERVICE_API_HEADER_GUARD

#include <agentd/protocolservice.h>
#include <vccrypt/suite.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Request IDs for the protocol service control API.
 */
typedef enum unauthorized_protocol_control_request_id
{
    UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD = 0x00000000,
    UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET = 0x00000001,
    UNAUTH_PROTOCOL_CONTROL_REQ_ID_FINALIZE        = 0x00000002,
} unauthorized_protocol_control_request_id_t;

/**
 * \brief Add an authorized entity to the protocol service.
 *
 * This entity is allowed to connect to the protocol service and send requests
 * to this service.
 *
 * \param sock                  The socket to which this request is written.
 * \param alloc_opts            The allocator options.
 * \param entity_id             The entity UUID.
 * \param entity_enc_pubkey     The entity's encryption public key.
 * \param entity_sign_pubkey    The entity's signing public key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_control_api_sendreq_authorized_entity_add(
    int sock, allocator_options_t* alloc_opts, const uint8_t* entity_id,
    const vccrypt_buffer_t* entity_enc_pubkey,
    const vccrypt_buffer_t* entity_sign_pubkey);

/**
 * \brief Receive a response from the authorized entity add request.
 *
 * \param sock                      The socket from which this response is read.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_control_api_recvresp_authorized_entity_add(
    int sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Set the private key for the protocol service.
 *
 * \param sock                  The socket to which this request is written.
 * \param alloc_opts            The allocator options.
 * \param entity_id             The blockchain agent entity UUID.
 * \param entity_enc_pubkey     The entity's encryption public key.
 * \param entity_enc_privkey    The entity's encryption private key.
 * \param entity_sign_pubkey    The entity's signing public key.
 * \param entity_sign_privkey   The entity's signing private key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_control_api_sendreq_private_key_set(
    int sock, allocator_options_t* alloc_opts, const uint8_t* entity_id,
    const vccrypt_buffer_t* entity_enc_pubkey,
    const vccrypt_buffer_t* entity_enc_privkey,
    const vccrypt_buffer_t* entity_sign_pubkey,
    const vccrypt_buffer_t* entity_sign_privkey);

/**
 * \brief Receive a response from the private key set request.
 *
 * \param sock                      The socket from which this response is read.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_control_api_recvresp_private_key_set(
    int sock, uint32_t* offset, uint32_t* status);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONTROL_PROTOCOLSERVICE_API_HEADER_GUARD*/
