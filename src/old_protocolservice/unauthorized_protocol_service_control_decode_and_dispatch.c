/**
 * \file
 * protocolservice/unauthorized_protocol_service_control_decode_and_dispatch.c
 *
 * \brief Decode and dispatch commands from the control socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Decode and dispatch requests received by the protocol service on
 * the control socket.
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
 *      - AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the control socket.
 */
int unauthorized_protocol_service_control_decode_and_dispatch(
    unauthorized_protocol_service_instance_t* instance,
    ipc_socket_context_t* sock, const void* req, size_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* make working with the request convenient. */
    const uint8_t* breq = (uint8_t*)req;

    /* the payload size should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = ntohl(nmethod);

    /* increment breq past the method. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = size - sizeof(uint32_t);
    MODEL_ASSERT(payload_size >= 0);

    /* decode the method. */
    switch (method)
    {
        /* add an authorized entity to the service. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD:
            return
                ups_control_decode_and_dispatch_auth_entity_add(
                    instance, sock, breq, payload_size);

        /* set the private key for the service. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET:
            return
                ups_control_decode_and_dispatch_private_key_set(
                    instance, sock, breq, payload_size);

        /* unknown method. Return an error. */
        default:
            ups_control_decode_and_dispatch_write_status(
                sock, method, 0U,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_BAD, NULL, 0);

            return AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_BAD;
    }
}
