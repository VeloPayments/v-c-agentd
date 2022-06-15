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
    canonizationservice_child_context_close(instance);
}
