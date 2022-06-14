/**
 * \file canonization/canonizationservice_child_context_close.c
 *
 * \brief Close the child context, leading to reset of the canonization service.
 *
 * \copyright 2020-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Close the child context, leading to reset of the canonization service.
 *
 * \param instance      The canonization service instance.
 */
void canonizationservice_child_context_close(
    canonizationservice_instance_t* instance)
{
    /* close the child context. */
    int retval =
        dataservice_api_sendreq_child_context_close_old(
            instance->data, &instance->alloc_opts,
            instance->data_child_context);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        return;
    }

    /* wait for the child context to close. */
    instance->state = CANONIZATIONSERVICE_STATE_WAITRESP_CHILD_CONTEXT_CLOSE;

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &canonizationservice_data_write,
        instance->loop_context);
}
