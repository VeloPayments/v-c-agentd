/**
 * \file
 * notificationservice/notificationservice_protocol_read_decode_and_dispatch_packet.c
 *
 * \brief Read, decode, and dispatch a client packet.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Read, decode, and dispatch a request from the client socket.
 *
 * \param context                   Notificationservice protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS if the fiber should retry the yield.
 *      - a non-zero error code if the fiber should exit.
 */
status notificationservice_protocol_read_decode_and_dispatch_packet(
    notificationservice_protocol_fiber_context* context)
{
    status retval, release_retval;
    uint8_t* buf = NULL;
    size_t size = 0U;
    uint32_t method_id;
    uint64_t offset;
    const uint8_t* payload = NULL;
    size_t payload_size = 0U;

    /* read a data packet from the client. */
    retval =
        psock_read_boxed_data(
            context->inst->protosock, context->alloc, (void**)&buf, &size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* decode the data packet. */
    retval =
        notificationservice_api_decode_request(
            buf, size, &method_id, &offset, &payload, &payload_size);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_buf;
    }

    /* dispatch the request. */
    switch (method_id)
    {
        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS:
            retval =
                notificationservice_protocol_dispatch_reduce_caps(
                    context, offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE:
            retval =
                notificationservice_protocol_dispatch_block_update(
                    context, offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION:
            retval =
                notificationservice_protocol_dispatch_block_assertion(
                    context, offset, payload, payload_size);
            break;

        case AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL:
            retval =
                notificationservice_protocol_dispatch_block_assertion_cancel(
                    context, offset, payload, payload_size);
            break;

        default:
            retval =
                notificationservice_protocol_send_response(
                    context, method_id,
                    offset,
                    AGENTD_ERROR_NOTIFICATIONSERVICE_INVALID_REQUEST_ID);
            if (STATUS_SUCCESS == retval)
            {
                retval = AGENTD_ERROR_NOTIFICATIONSERVICE_INVALID_REQUEST_ID;
            }
            break;
    }

    /* Any error returned above means that we should terminate the process. */
    /* Recoverable protocol related errors should be sent back to the client. */
    goto cleanup_buf;

cleanup_buf:
    release_retval = rcpr_allocator_reclaim(context->alloc, buf);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
