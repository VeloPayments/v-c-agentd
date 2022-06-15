/**
 * \file canonization/canonizationservice_complete_update.c
 *
 * \brief Complete an update run of the canonization service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Finish the canonizationservice update.
 *
 * \param instance      The canonization service instance.
 */
void canonizationservice_complete_update(
    canonizationservice_instance_t* instance)
{
    /* on the first time through, send a block update notification. */
    if (instance->first_time)
    {
        /* reset the first time flag. */
        instance->first_time = false;

        /* update the block id. */
        memcpy(instance->block_id, instance->previous_block_id, 16);

        /* send the block notification. */
        canonizationservice_notify_block_update(instance);
    }
    else
    {
        /* close the child context. */
        canonizationservice_child_context_close(instance);
    }
}
