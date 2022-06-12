/**
 * \file notificationservice/notificationservice_assertion_entry_key.c
 *
 * \brief Get the offset for a given notificationservice assertion entry.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

/**
 * \brief Given a notificationservice_assertion_entry, return the offset.
 *
 * \param context       Unused.
 * \param r             The resource handle of the
 *                      notificationservice_assertion_entry.
 *
 * \returns the offset key for the entry.
 */
const void* notificationservice_assertion_entry_key(
    void* /*context*/, const RCPR_SYM(resource)* r)
{
    notificationservice_assertion_entry* entry =
        (notificationservice_assertion_entry*)r;

    return &entry->offset;
}
