/**
 * \file protocolservice/ups_dispatch_dataservice_response_block_read_id_prev.c
 *
 * \brief Handle the response from the dataservice block read id prev request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <config.h>
#include <agentd/status_codes.h>
#include <stddef.h>
#include <vccrypt/compare.h>

#if !defined(AGENTD_NEW_PROTOCOL)

#include "unauthorized_protocol_service_private.h"

static uint8_t zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * Handle a block id read prev response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_block_read_id_prev(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_block_get_t* dresp)
{
    /* build the payload. */
    uint32_t net_method = htonl(conn->request_id);
    uint32_t net_status = htonl(dresp->hdr.status);
    uint32_t net_offset = htonl(conn->current_request_offset);

    /* if the API call wasn't successful, return the error payload. */
    if (AGENTD_STATUS_SUCCESS != dresp->hdr.status)
    {
        uint8_t payload[3 * sizeof(uint32_t)];
        memcpy(payload, &net_method, 4);
        memcpy(payload + 4, &net_status, 4);
        memcpy(payload + 8, &net_offset, 4);

        /* attempt to write this payload to the socket. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, sizeof(payload),
                &conn->svc->suite, &conn->shared_secret))
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
    }
    /* full payload. */
    else
    {
        bool copy_uuid = true;

        /* if prev is the beginning sentry, return a not found error. */
        if (!crypto_memcmp(dresp->node.prev, zero_uuid, 16))
        {
            copy_uuid = false;
            net_status = htonl(AGENTD_ERROR_DATASERVICE_NOT_FOUND);
        }

        /* compute the payload size. */
        size_t payload_size =
            /* method, status, offset */
            3 * sizeof(uint32_t);

        /* should we copy the prev uuid? */
        if (copy_uuid)
        {
            payload_size += 1 * 16;
        }

        /* allocate the payload data. */
        uint8_t* payload = (uint8_t*)malloc(payload_size);
        if (NULL == payload)
        {
            unauthorized_protocol_service_error_response(
                conn, UNAUTH_PROTOCOL_REQ_ID_BLOCK_BY_ID_GET,
                AGENTD_ERROR_GENERAL_OUT_OF_MEMORY,
                conn->current_request_offset, true);
            return;
        }

        /* populate header info. */
        memcpy(payload, &net_method, 4);
        memcpy(payload + 4, &net_status, 4);
        memcpy(payload + 8, &net_offset, 4);

        /* populate block uuid. */
        if (copy_uuid)
        {
            memcpy(payload + 12, dresp->node.prev, 16);
        }

        /* attempt to write this payload to the socket. */
        int retval =
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, payload_size,
                &conn->svc->suite, &conn->shared_secret);

        /* clean up payload. */
        memset(payload, 0, payload_size);
        free(payload);

        /* check status of write. */
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
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
}

#endif /* !defined(AGENTD_NEW_PROTOCOL) */
