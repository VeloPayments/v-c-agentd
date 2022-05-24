/**
 * \file
 * protocolservice/protocolservice_authorized_entity_capabilities_compare.c
 *
 * \brief Compare two capability keys.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>

#include "protocolservice_internal.h"

RCPR_IMPORT_uuid;

/**
 * \brief Compare two opaque \ref authorized_entity_capability_key values.
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
protocolservice_authorized_entity_capabilities_compare(
    void* /*context*/, const void* lhs, const void* rhs)
{
    const protocolservice_authorized_entity_capability_key* l =
        (protocolservice_authorized_entity_capability_key*)lhs;
    const protocolservice_authorized_entity_capability_key* r =
        (protocolservice_authorized_entity_capability_key*)rhs;

    int result = memcmp(l, r, sizeof(*l));

    if (result < 0)
    {
        return RCPR_COMPARE_LT;
    }
    else if (result > 0)
    {
        return RCPR_COMPARE_GT;
    }
    else
    {
        return RCPR_COMPARE_EQ;
    }
}
