/**
 * \file
 * protocolservice/protocolservice_extended_api_response_xlat_entry_key.c
 *
 * \brief Get the server offset from an xlat entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

/**
 * \brief Given a \ref protocolservice_extended_api_response_xlat_entry resource
 * handle, return its \ref rcpr_uuid key.
 *
 * \param context       Unused.
 * \param r             The resource handle of an extended api xlat entry.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_extended_api_response_xlat_entry_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    protocolservice_extended_api_response_xlat_entry* entry =
        (protocolservice_extended_api_response_xlat_entry*)r;

    return &entry->server_offset;
}
