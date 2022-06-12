/**
 * \file notificationservice/notificationservice_assertion_entry_compare.c
 *
 * \brief Compare two notificationservice assertion offsets.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

/**
 * \brief Compare two opaque notificationservice_assertion_entry offset
 * pointers.
 *
 * \param context       Unused.
 * \param lhs           The left-hand side of the comparison.
 * \param rhs           The right-hand side of the comparison.
 *
 * \returns an integer value representing the comparison result.
 *      - RCPR_COMPARE_LT if \p lhs &lt; \p rhs.
 *      - RCPR_COMPARE_EQ if \p lhs == \p rhs.
 *      - RCPR_COMPARE_GT if \p lhs &gt; \p rhs;
 */
RCPR_SYM(rcpr_comparison_result) notificationservice_assertion_entry_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    const uint64_t* l = (const uint64_t*)lhs;
    const uint64_t* r = (const uint64_t*)rhs;

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
