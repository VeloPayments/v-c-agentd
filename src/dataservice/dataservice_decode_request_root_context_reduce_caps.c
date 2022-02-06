/**
 * \file dataservice/dataservice_decode_request_root_context_reduce_caps.c
 *
 * \brief Decode a root context reduce capabilities request payload.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a root context reduce caps request into its constituent pieces.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_root_context_reduce_caps(
    const void* req, size_t size,
    dataservice_request_payload_root_context_reduce_caps_t* dreq)
{
    int retval = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != dreq);

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* initialize the request structure. */
    retval =
        dataservice_request_init_empty(&breq, &size, &dreq->hdr, sizeof(*dreq));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* the payload size should be equal to the size of the capabilities. */
    if (size != sizeof(dreq->caps))
    {
        retval = AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto cleanup_dreq;
    }

    /* copy the caps. */
    memcpy(dreq->caps, breq, size);

    /* success. dreq contents are owned by the caller. */
    goto done;

cleanup_dreq:
    /* we failed, so don't pass dreq contents to the caller. */
    dispose((disposable_t*)dreq);

done:
    return retval;
}
