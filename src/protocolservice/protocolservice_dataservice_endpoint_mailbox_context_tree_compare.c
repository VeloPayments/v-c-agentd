/**
 * \file
 * protocolservice/protocolservice_dataservice_endpoint_mailbox_context_tree_compare.c
 *
 * \brief Compare two mailbox IDs.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_message;

/**
 * \brief Compare two opaque \ref mailbox_address values.
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
protocolservice_dataservice_endpoint_mailbox_context_tree_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    const mailbox_address* left = (const mailbox_address*)lhs;
    const mailbox_address* right = (const mailbox_address*)rhs;

    if (*left < *right)
    {
        return RCPR_COMPARE_LT;
    }
    else if (*left > *right)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}
