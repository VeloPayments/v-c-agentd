/**
 * \file
 * protocolservice/protocolservice_notificationservice_server_xlat_map_key.c
 *
 * \brief Get the server offset for a server translation map.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

/**
 * \brief Given a protocolservice_notificationservice_xlat_entry resource handle,
 * return its \ref server_offset value.
 *
 * \param context       Unused.
 * \param r             The resource handle of an authorized entity.
 *
 * \returns the key for the authorized entity resource.
 */
const void* protocolservice_notificationservice_server_xlat_map_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    protocolservice_notificationservice_xlat_entry* entry =
        (protocolservice_notificationservice_xlat_entry*)r;

    return &entry->server_offset;
}
