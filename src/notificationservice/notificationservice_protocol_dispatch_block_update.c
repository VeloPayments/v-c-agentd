/**
 * \file
 * notificationservice/notificationservice_protocol_dispatch_block_update.c
 *
 * \brief Dispatch a block update request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "notificationservice_internal.h"

RCPR_IMPORT_resource;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_slist;

/**
 * \brief Dispatch a block update request.
 *
 * \param context                   Notificationservice protocol fiber context.
 * \param offset                    The client-supplied request offset.
 * \param payload                   Payload data for this request.
 * \param payload_size              The size of the payload data.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_protocol_dispatch_block_update(
    notificationservice_protocol_fiber_context* context, uint64_t offset,
    const uint8_t* payload, size_t payload_size)
{
    status retval, status_retval, release_retval;
    slist* instance_lists;
    rbtree* notifytree;
    rbtree* assertiontree = NULL;
    slist_node* instance_node;
    rbtree_node* assertion_node;
    rbtree_node* nil;
    notificationservice_instance* inst;
    notificationservice_assertion_entry* assertion_entry;

    /* check to see if this call is permissible. */
    if (!BITCAP_ISSET(
            context->inst->caps, NOTIFICATIONSERVICE_API_CAP_BLOCK_UPDATE))
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_NOT_AUTHORIZED;
        goto report_status;
    }

    /* verify that the payload is set and the payload size is valid. */
    if (NULL == payload
     || sizeof(context->inst->ctx->latest_block_id) != payload_size)
    {
        /* this is a fatal error. */
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_MALFORMED_REQUEST;
        goto report_status;
    }

    /* set the block_id. */
    memcpy(&context->inst->ctx->latest_block_id, payload, payload_size);

    /* create an slist for holding the instance lists. */
    retval = slist_create(&instance_lists, context->alloc);
    if (STATUS_SUCCESS != retval)
    {
        /* this is a fatal error. */
        goto report_status;
    }

    /* get the head of the instances list. */
    retval = slist_head(&instance_node, context->inst->ctx->instances);
    if (STATUS_SUCCESS != retval)
    {
        /* this is a fatal error. */
        goto cleanup_instance_lists;
    }

    /* iterate through this list. */
    while (NULL != instance_node)
    {
        /* get the instance for this node. */
        retval = slist_node_child((resource**)&inst, instance_node);
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_instance_lists;
        }

        /* create an assertion rbtree instance for holding all assertions. */
        retval =
            notificationservice_assertion_rbtree_create(
                &assertiontree, context->alloc);
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_instance_lists;
        }

        /* swap this tree and the assertions for this instance. */
        rbtree_swap(assertiontree, inst->assertions);

        /* add the assertions to the end of our list. */
        retval =
            slist_append_tail(
                instance_lists, rbtree_resource_handle(assertiontree));
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_assertionlist;
        }

        /* the assertion tree is now owned by the instance work list. */
        assertiontree = NULL;

        /* get the next node in the instance list. */
        retval = slist_node_next(&instance_node, instance_node);
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_instance_lists;
        }
    }

    /* we can now block without impacting the other instance, so iterate over
     * our work list and notify the outbound fiber that there is work to do. */

    /* get the head of the instance work list. */
    retval = slist_head(&instance_node, instance_lists);
    if (STATUS_SUCCESS != retval)
    {
        /* this is a fatal error. */
        goto cleanup_instance_lists;
    }

    /* outer loop. */
    while (NULL != instance_node)
    {
        /* get the assertion tree. */
        retval = slist_node_child((resource**)&notifytree, instance_node);
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_instance_lists;
        }

        /* get the nil node of the notify tree. */
        nil = rbtree_nil_node(notifytree);

        /* get the root of the notify tree. */
        assertion_node = rbtree_root_node(notifytree);

        /* if the root node is not null, get the minimum of this subtree. */
        if (nil != assertion_node)
        {
            assertion_node = rbtree_minimum_node(notifytree, assertion_node);
        }

        /* inner loop: iterate through all assertions. */
        while (nil != assertion_node)
        {
            /* get the assertion entry. */
            assertion_entry =
                (notificationservice_assertion_entry*)rbtree_node_value(
                    notifytree, assertion_node);

            /* notify this offset that the assertion has been invalidated. */
            retval =
                notificationservice_protocol_send_response(
                    assertion_entry->context,
                    AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION,
                    assertion_entry->offset, STATUS_SUCCESS);
            if (STATUS_SUCCESS != retval)
            {
                /* this is a fatal error. */
                goto cleanup_instance_lists;
            }

            /* get the next node in the notify tree. */
            assertion_node = rbtree_successor_node(notifytree, assertion_node);
        }

        /* get the next node in the instance list. */
        retval = slist_node_next(&instance_node, instance_node);
        if (STATUS_SUCCESS != retval)
        {
            /* this is a fatal error. */
            goto cleanup_instance_lists;
        }
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_instance_lists;

cleanup_assertionlist:
    if (NULL != assertiontree)
    {
        release_retval =
            resource_release(rbtree_resource_handle(assertiontree));
        if (STATUS_SUCCESS != release_retval)
        {
            retval = release_retval;
        }
    }

cleanup_instance_lists:
    release_retval = resource_release(slist_resource_handle(instance_lists));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

report_status:
    status_retval =
        notificationservice_protocol_send_response(
            context, AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE,
            offset, retval);
    if (STATUS_SUCCESS != status_retval)
    {
        retval = status_retval;
    }

    return retval;
}
