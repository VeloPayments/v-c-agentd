/**
 * \file
 * protocolservice/protocolservice_protocol_unroute_extended_api_for_entity.c
 *
 * \brief Remove routing from the extended API routing table for the given
 * connection's entity.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;

/**
 * \brief Unroute the extended API for a given sentinel entity.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      entity connection.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_unroute_extended_api_for_entity(
    protocolservice_protocol_fiber_context* ctx)
{
    /* Delete the entry from the rbtree, if found. */
    return rbtree_delete(NULL, ctx->ctx->extended_api_dict, &ctx->entity_uuid);
}
