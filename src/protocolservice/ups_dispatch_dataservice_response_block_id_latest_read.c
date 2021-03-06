/**
 * \file
 * protocolservice/ups_dispatch_dataservice_response_block_id_latest_read.c
 *
 * \brief Handle the response from the dataservice block id latest read request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a block_id_latest_read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_block_id_latest_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_latest_block_id_get_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_latest_block_id_get(
            resp, resp_size, &dresp))
    {
        /* TODO - log a fatal error here. */
        unauthorized_protocol_service_exit_event_loop(svc);
        return;
    }

    /* get the connection associated with this child id. */
    unauthorized_protocol_connection_t* conn =
        svc->dataservice_child_map[dresp.hdr.offset];
    if (NULL == conn)
    {
        /* TODO - warn level log about mismatch. */
        goto cleanup_dresp;
    }

    /* build the payload. */
    uint32_t net_method = htonl(UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET);
    uint32_t net_status = dresp.hdr.status;
    uint32_t net_offset = conn->current_request_offset;
    uint8_t payload[3 * sizeof(uint32_t) + 16];
    memcpy(payload, &net_method, 4);
    memcpy(payload + 4, &net_status, 4);
    memcpy(payload + 8, &net_offset, 4);
    memcpy(payload + 12, dresp.block_id, 16);

    /* attempt to write this payload to the socket. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_write_authed_data_noblock(
            &conn->ctx, conn->server_iv, payload, sizeof(payload),
            &conn->svc->suite, &conn->shared_secret))
    {
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_dresp;
    }

    /* Update the server iv on success. */
    ++conn->server_iv;

    /* evolve connection state. */
    conn->state = APCS_WRITE_COMMAND_RESP_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* success. */

cleanup_dresp:
    dispose((disposable_t*)&dresp);
}
