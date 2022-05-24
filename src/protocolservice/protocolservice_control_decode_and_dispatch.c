/**
 * \file protocolservice/protocolservice_control_decode_and_dispatch.c
 *
 * \brief Decode and dispatch control messages from the supervisor.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/uuid.h>
#include <string.h>

#include "protocolservice_internal.h"

/**
 * \brief Decode and dispatch a control packet from the supervisor.
 *
 * \param ctx           The protocol service control fiber context.
 * \param req           Pointer to the control packet.
 * \param size          The size of the control packet.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_decode_and_dispatch(
    protocolservice_control_fiber_context* ctx, const void* req, size_t size)
{
    status retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != req);

    /* make working with the request packet more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

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

    /* calculate the payload size. */
    size_t payload_size = size - sizeof(uint32_t);
    MODEL_ASSERT(payload_size >= 0);

    /* decode the method. */
    switch (method)
    {
        /* add an authorized entity to the service. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_ADD:
            return
                protocolservice_control_dispatch_auth_entity_add(
                    ctx, breq, payload_size);

        #if 0
        /* add a capability to an authorized entity. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_CAP_ADD:
            return
                protocolservice_control_dispatch_auth_entity_capability_add(
                    ctx, breq, payload_size);
        #endif

        /* set the private key for the service. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_PRIVATE_KEY_SET:
            return
                protocolservice_control_dispatch_private_key_set(
                    ctx, breq, payload_size);

        /* close the control socket. */
        case UNAUTH_PROTOCOL_CONTROL_REQ_ID_FINALIZE:
            return
                protocolservice_control_dispatch_finalize(
                    ctx, breq, payload_size);

        /* unknown method. Return an error. */
        default:
            retval =
                protocolservice_control_write_response(
                    ctx, method,
                    AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_BAD);
            if (STATUS_SUCCESS != retval)
            {
                return retval;
            }

            return AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_BAD;
    }
}
