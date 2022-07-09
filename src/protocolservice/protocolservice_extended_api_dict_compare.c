/**
 * \file protocolservice/protocolservice_extended_api_dict_compare.c
 *
 * \brief Compare two entity UUID keys for the extended api dictionary.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_uuid;

/**
 * \brief Compare two opaque \ref protocolservice_extended_api_dict_entry
 * keys.
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
RCPR_SYM(rcpr_comparison_result) protocolservice_extended_api_dict_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    rcpr_uuid* lid = (rcpr_uuid*)lhs;
    rcpr_uuid* rid = (rcpr_uuid*)rhs;

    /* TODO - does this open a timing oracle on available extended apis? */
    int res = memcmp(lid, rid, sizeof(rcpr_uuid));

    if (res < 0)
    {
        return RCPR_COMPARE_LT;
    }
    else if (res > 0)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}
