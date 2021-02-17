/**
 * \file bootstrap_config/bootstrap_config_set_version_request.c
 *
 * \brief Set the version_request flag in the bootstrap config.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <string.h>

/**
 * \brief Set agentd to perform a version request.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param version_req   Set to true: service a version request.
 */
void bootstrap_config_set_version_request(
    bootstrap_config_t* bconf, bool version_req)
{
    MODEL_ASSERT(NULL != bconf);
    bconf->version_request = version_req;
}
