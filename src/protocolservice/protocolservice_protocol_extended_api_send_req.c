/**
 * \file
 * protocolservice/protocolservice_protocol_extended_api_send_req.c
 *
 * \brief Look up a sentinel and forward a request to it.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Forward an extended API request to the appropriate sentinel.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      operation.
 * \param req           The request to forward.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_extended_api_send_req(
    protocolservice_protocol_fiber_context* ctx,
    const protocol_req_extended_api* req)
{
    status retval;
    protocolservice_extended_api_dict_entry* entry;

    /* attempt to look up the entity route mapping. */
    retval =
        rbtree_find(
            (resource**)&entry, ctx->ctx->extended_api_dict, &req->entity_id);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_EXTENDED_API_UNKNOWN_ENTITY;
        goto done;
    }

    /* TODO - finish this method. */
    retval = -1;
    goto done;

done:
    return retval;
}
