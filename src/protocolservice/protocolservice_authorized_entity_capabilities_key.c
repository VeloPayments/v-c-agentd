/**
 * \file protocolservice/protocolservice_authorized_entity_capabilities_key.c
 *
 * \brief Get the key of a given authorized entity capability.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

/**
 * \brief Given an authorized entity capability resource handle, return its
 * \ref authorized_entity_capability_key.
 * value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_authorized_entity_capabilities_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    const protocolservice_authorized_entity_capability* cap =
        (const protocolservice_authorized_entity_capability*)r;

    /* return the capability key. */
    return &cap->key;
}
