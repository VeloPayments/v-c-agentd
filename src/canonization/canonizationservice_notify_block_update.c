/**
 * \file canonization/canonizationservice_notify_block_update.c
 *
 * \brief Send a request to the notification service to notify any waiting
 * sentinels that the latest block id has been updated.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);

/**
 * \brief Send a block update request to the notification service.
 *
 * \param instance      The canonization service instance.
 */
void canonizationservice_notify_block_update(
    canonizationservice_instance_t* instance)
{
    status retval, release_retval;
    uint8_t* buf = NULL;
    size_t size = 0U;

    /* encode the block update request. */
    retval =
        notificationservice_api_encode_request(
            &buf, &size, instance->rcpr_alloc,
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE,
            7474, instance->block_id, 16);
    if (STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* send the request to the notification service. */
    retval = ipc_write_data_noblock(instance->notify, buf, size);
    if (STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_buf;
    }

    /* wait for the block update to complete. */
    instance->state = CANONIZATIONSERVICE_STATE_WAITRESP_NOTIFY_BLOCK_UPDATE;

    /* set the write callback for the notificationservice socket. */
    ipc_set_writecb_noblock(
        instance->notify, &canonizationservice_notify_write,
        instance->loop_context);

    /* success. */

cleanup_buf:
    memset(buf, 0, size);
    release_retval = rcpr_allocator_reclaim(instance->rcpr_alloc, buf);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:;
}
