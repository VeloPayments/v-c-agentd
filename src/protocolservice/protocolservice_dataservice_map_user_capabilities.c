/**
 * \file protocolservice/protocolservice_dataservice_map_user_capabilities.c
 *
 * \brief Map the list of user capabilities to dataservice child context
 * capabilities.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/dataservice.h>
#include <agentd/protocolservice/protocolservice_capabilities.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/**
 * \brief Map the user capabilities in a form that the data service open context
 * request can understand.
 *
 * \param payload           The buffer to receive the payload to the open
 *                          context request. This buffer must be uninitialized.
 * \param ctx               The protocol service protocol fiber context.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status protocolservice_dataservice_map_user_capabilities(
    vccrypt_buffer_t* payload, protocolservice_protocol_fiber_context* ctx)
{
    status retval;
    vccrypt_buffer_t datacap_buffer;
    protocolservice_authorized_entity* entity;
    BITCAP(dataservice_caps, DATASERVICE_API_CAP_BITS_MAX);

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* look up the entity. */
    retval =
        rbtree_find(
            (resource**)&entity, ctx->ctx->authorized_entity_dict,
            (const void*)&ctx->entity_uuid);
    if (STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED;
    }

    /* clear the capabilities. */
    BITCAP_INIT_FALSE(dataservice_caps);

    /* This capability is always set so we can close the child context. */
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* check block id latest read cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_LATEST_READ,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    }

    /* check transaction submit cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_SUBMIT,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    }

    /* check block read cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_BLOCK_READ,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_READ);
    }

    /* check block id by height read cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_BY_HEIGHT_READ,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);
    }

    /* check transaction read cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_READ,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    }

    /* check artifact read cap. */
    if (protocolservice_authorized_entity_capability_check(
            entity, &ctx->entity_uuid,
            &PROTOCOLSERVICE_API_CAPABILITY_ARTIFACT_READ,
            &ctx->ctx->agentd_uuid))
    {
        BITCAP_SET_TRUE(
            dataservice_caps, DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    }

    /* initialize the datacap buffer. */
    retval =
        vccrypt_buffer_init(
            &datacap_buffer, &ctx->ctx->vpr_alloc, sizeof(dataservice_caps));
    if (STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the data capabilities to our buffer. */
    memcpy(datacap_buffer.data, dataservice_caps, sizeof(dataservice_caps));

    /* move this buffer to the caller's buffer. */
    vccrypt_buffer_move(payload, &datacap_buffer);

    /* success. */
    return STATUS_SUCCESS;
}
