/**
 * \file
 * protocolservice/protocolservice_notificationservice_client_xlat_map_compare.c
 *
 * \brief Compare the client addresses of two translation map entries.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "protocolservice_internal.h"

RCPR_IMPORT_message;

/**
 * \brief Compare two opaque \ref protocolservice_notificationservice_xlat_entry
 * values.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side of the comparison.
 * \param rhs           The right-hand side of the comparison.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs.
 */
RCPR_SYM(rcpr_comparison_result)
protocolservice_notificationservice_client_xlat_map_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    const mailbox_address* l = (const mailbox_address*)lhs;
    const mailbox_address* r = (const mailbox_address*)rhs;

    if (*l < *r)
    {
        return RCPR_COMPARE_LT;
    }
    else if (*l > *r)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}
