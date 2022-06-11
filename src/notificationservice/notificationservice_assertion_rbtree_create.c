/**
 * \file notificationservice/notificationservice_assertion_rbtree_create.c
 *
 * \brief Create a notificationservice assertion rbtree. 
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include "notificationservice_internal.h"

RCPR_IMPORT_rbtree;

/**
 * \brief Create an assertion rbtree instance.
 *
 * \param tree          Pointer to receive the new rbtree instance on success.
 * \param alloc         The allocator to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_assertion_rbtree_create(
    RCPR_SYM(rbtree)** tree, RCPR_SYM(allocator)* alloc)
{
    return
        rbtree_create(
            tree, alloc, &notificationservice_assertion_entry_compare,
            &notificationservice_assertion_entry_key, NULL);
}
