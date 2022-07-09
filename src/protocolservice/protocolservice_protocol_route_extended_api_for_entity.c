/**
 * \file
 * protocolservice/protocolservice_protocol_route_extended_api_for_entity.c
 *
 * \brief Add routing to the extended API routing table for the given
 * connection's entity.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Route the extended API for a given sentinel entity.
 *
 * \param ctx           The protocolservice protocol fiber context for this
 *                      entity connection.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_protocol_route_extended_api_for_entity(
    protocolservice_protocol_fiber_context* ctx)
{
    status retval, release_retval;
    protocolservice_extended_api_dict_entry* tmp;

    /* first, check to see if there is already an entry for this entity. */
    retval =
        rbtree_find(
            (resource**)&tmp, ctx->ctx->extended_api_dict, &ctx->entity_uuid);
    if (STATUS_SUCCESS == retval)
    {
        /* an entry was found, so don't clobber it. */
        retval = AGENTD_ERROR_PROTOCOLSERVICE_EXTENDED_API_ALREADY_ENABLED;
        goto done;
    }

    /* create an entry to insert into the dictionary. */
    retval =
        protocolservice_extended_api_dict_entry_create(
            &tmp, ctx->alloc, &ctx->entity_uuid, ctx);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* insert this entry into the dictionary. */
    retval = rbtree_insert(ctx->ctx->extended_api_dict, (resource*)tmp);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
