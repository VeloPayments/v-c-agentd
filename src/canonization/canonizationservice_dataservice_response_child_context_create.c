/**
 * \file
 * canonization/canonizationservice_dataservice_response_child_context_create.c
 *
 * \brief Handle the response from the data service child context create call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle the response from the data service child context create call.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_child_context_create(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size)
{
    int retval;
    dataservice_response_child_context_create_t dresp;

    /* decode the response. */
    retval =
        dataservice_decode_response_child_context_create(
            resp, resp_size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval || AGENTD_STATUS_SUCCESS != dresp.hdr.status)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* save the child instance index. */
    instance->data_child_context = dresp.child;

    /* send the request to read the latest block id. */
    retval =
        canonizationservice_dataservice_sendreq_block_id_latest_get(
            instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

done:;
}
