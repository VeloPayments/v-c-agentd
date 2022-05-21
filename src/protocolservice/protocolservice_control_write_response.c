/**
 * \file protocolservice/protocolservice_control_write_response.c
 *
 * \brief Write a response to the control socket.
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

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Write a response to the control socket.
 *
 * \param ctx           The control fiber context.
 * \param request_id    The id of the request.
 * \param status        The status code.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_write_response(
    protocolservice_control_fiber_context* ctx, int request_id, int status_)
{
    status retval, release_retval;
    uint8_t* resp;

    /* parameter sanity checking. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));

    /* | Response packet.                                             | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | method_id                                     | 4 bytes      | */
    /* | offset                                        | 4 bytes      | */
    /* | status                                        | 4 bytes      | */
    /* | data                                          | n - 12 bytes | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the size of the response. */
    size_t respsize =
        /* the size of the method. */
        sizeof(uint32_t) +
        /* the size of the offset. */
        sizeof(uint32_t) +
        /* the size of the status. */
        sizeof(uint32_t);

    /* allocate memory for the response. */
    retval = rcpr_allocator_allocate(ctx->alloc, (void**)&resp, respsize);
    if (STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* set the values for the response. */
    uint32_t net_method = htonl(request_id);
    uint32_t net_offset = htonl(0);
    uint32_t net_status = htonl(status_);

    memcpy(resp + 0 * sizeof(uint32_t), &net_method, sizeof(net_method));
    memcpy(resp + 1 * sizeof(uint32_t), &net_offset, sizeof(net_offset));
    memcpy(resp + 2 * sizeof(uint32_t), &net_status, sizeof(net_status));

    /* write the data packet. */
    retval = psock_write_boxed_data(ctx->controlsock, resp, respsize);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(resp, 0, respsize);
    release_retval = rcpr_allocator_reclaim(ctx->alloc, resp);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

    /* return the status of the response write to the caller. */
    return retval;
}
