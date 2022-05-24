/**
 * \file
 * protocolservice/protocolservice_control_dispatch_auth_entity_capability_add.c
 *
 * \brief Dispatch an auth entity capability add control command.
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

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/**
 * \brief Dispatch an auth entity capability add control request.
 *
 * \param ctx           The protocol service control fiber context.
 * \param payload       Pointer to the payload for this request.
 * \param size          Size of the request payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_control_dispatch_auth_entity_capability_add(
    protocolservice_control_fiber_context* ctx, const void* payload,
    size_t size)
{
    status retval, release_retval;
    rcpr_uuid entity_id, subject_id, verb_id, object_id;
    protocolservice_authorized_entity* entity;
    protocolservice_authorized_entity_capability* cap;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_protocolservice_control_fiber_context_valid(ctx));
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(size > 0);

    /* compute the message header size. */
    const size_t payload_size = sizeof(uint32_t) + 4 * 16;

    /* ensure that the payload size is at least large enough to hold this
     * header. */
    if (size < payload_size)
    {
        retval =
            protocolservice_control_write_response(
                ctx, UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_CAP_ADD,
                AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE);
        if (STATUS_SUCCESS == retval)
        {
            retval = AGENTD_ERROR_PROTOCOLSERVICE_REQUEST_PACKET_INVALID_SIZE;
        }
        goto done;
    }

    /* treat the request as a byte buffer for convenience. */
    const uint8_t* breq = (const uint8_t*)payload;

    /* get the request offset. */
    uint32_t noffset;
    memcpy(&noffset, breq, sizeof(uint32_t)); breq += sizeof(uint32_t);

    /* get the entity id. */
    memcpy(&entity_id, breq, sizeof(entity_id)); breq += sizeof(entity_id);

    /* get the subject id. */
    memcpy(&subject_id, breq, sizeof(subject_id)); breq += sizeof(subject_id);

    /* get the verb id. */
    memcpy(&verb_id, breq, sizeof(verb_id)); breq += sizeof(verb_id);

    /* get the object id. */
    memcpy(&object_id, breq, sizeof(object_id)); breq += sizeof(object_id);

    /* look up the entity id. */
    retval =
        rbtree_find(
            (resource**)&entity, ctx->ctx->authorized_entity_dict,
            (const void*)&entity_id);
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_CONTROL_ENTITY_NOT_FOUND;
        goto done;
    }

    /* create a capability. */
    retval =
        protocolservice_authorized_entity_capability_create(
            &cap, ctx->alloc, &subject_id, &verb_id, &object_id);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* insert this capability into the capabilities set. */
    retval = rbtree_insert(entity->capabilities, &cap->hdr);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_cap;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto done;

cleanup_cap:
    release_retval = resource_release(&cap->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
