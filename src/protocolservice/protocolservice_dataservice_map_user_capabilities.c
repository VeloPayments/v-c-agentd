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
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>

#include "protocolservice_internal.h"

#if defined(AGENTD_NEW_PROTOCOL)

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
    BITCAP(dataservice_caps, DATASERVICE_API_CAP_BITS_MAX);

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(prop_protocolservice_protocol_fiber_context_valid(ctx));

    /* TODO - derive these capabilities from the user cert. */
    BITCAP_INIT_FALSE(dataservice_caps);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_ARTIFACT_READ);

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

#endif /* defined(AGENTD_NEW_PROTOCOL) */
