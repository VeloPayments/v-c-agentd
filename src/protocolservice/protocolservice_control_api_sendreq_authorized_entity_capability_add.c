/**
 * \file
 * protocolservice/protocolservice_control_api_sendreq_authorized_entity_add.c
 *
 * \brief Send the authorized entity add request to the protocol service control
 * socket.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include <agentd/protocolservice/control_api.h>

/**
 * \brief Add a capability for the given authorized entity.
 *
 * This entity is allowed to perform this capability in the protocol service.
 *
 * \param sock                  The socket to which this request is written.
 * \param alloc_opts            The allocator options.
 * \param entity_id             The entity UUID.
 * \param subject_id            The subject id for the capability.
 * \param verb_id               The verb id for the capability.
 * \param object_id             The object id for the capability.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_control_api_sendreq_authorized_entity_capability_add(
    int sock, allocator_options_t* alloc_opts, const uint8_t* entity_id,
    const uint8_t* subject_id, const uint8_t* verb_id,
    const uint8_t* object_id)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != subject_id);
    MODEL_ASSERT(NULL != verb_id);
    MODEL_ASSERT(NULL != object_id);

    /* create a buffer for holding the request. */
    size_t req_size =
        2 * sizeof(uint32_t)
      + 4 * 16; /* uuids. */
    vccrypt_buffer_t req;
    if (VCCRYPT_STATUS_SUCCESS
     != vccrypt_buffer_init(&req, alloc_opts, req_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* get a buffer pointer for convenience. */
    uint8_t* breq = (uint8_t*)req.data;

    /* write the method id. */
    uint32_t net_method_id =
        htonl(UNAUTH_PROTOCOL_CONTROL_REQ_ID_AUTH_ENTITY_CAP_ADD);
    memcpy(breq, &net_method_id, sizeof(net_method_id));
    breq += sizeof(net_method_id);

    /* write the request id. */
    uint32_t net_request_id = htonl(0UL);
    memcpy(breq, &net_request_id, sizeof(net_request_id));
    breq += sizeof(net_request_id);

    /* write the entity id. */
    memcpy(breq, entity_id, 16);
    breq += 16;

    /* write the subject id. */
    memcpy(breq, subject_id, 16);
    breq += 16;

    /* write the verb id. */
    memcpy(breq, verb_id, 16);
    breq += 16;

    /* write the object id. */
    memcpy(breq, object_id, 16);
    breq += 16;

    /* write the request packet to the server. */
    retval = ipc_write_data_block(sock, req.data, req.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_req;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
