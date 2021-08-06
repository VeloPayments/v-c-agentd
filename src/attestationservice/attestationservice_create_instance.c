/**
 * \file attestationservice/attestationservice_create_instance.c
 *
 * \brief Create an instance for the attestation service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/status_codes.h>
#include <string.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_compare;
RCPR_IMPORT_fiber;
RCPR_IMPORT_psock;
RCPR_IMPORT_rbtree;
RCPR_IMPORT_resource;

/* forward decls. */
static status attestationservice_instance_resource_release(resource* r);
static rcpr_comparison_result txn_tree_compare(
    void* context, const void* lhs, const void* rhs);
static const void* txn_tree_key(
    void* context, const resource* r);
static rcpr_comparison_result artifact_tree_compare(
    void* context, const void* lhs, const void* rhs);
static const void* artifact_tree_key(
    void* context, const resource* r);

/**
 * \brief Create an attestation service instance to pass to the attestation
 * service event loop.
 *
 * \param inst          Pointer to receive the instance.
 * \param alloc         The allocator to use for this operation.
 * \param sched         The fiber scheduler for this instance.
 * \param sleep_fd      The socket descriptor to use when communicating with the
 *                      sleep thread.
 * \param data_fd       The socket descriptor to use when communicating with the
 *                      data service instance dedicated to this attestation
 *                      service.
 * \param log_fd        The socket descriptor to use when communicating with the
 *                      logging service.
 * \param control_fd    The socket descriptor to use when communicating with the
 *                      supervisor during the bootstrap process.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status attestationservice_create_instance(
    attestationservice_instance** inst, rcpr_allocator* alloc,
    fiber_scheduler* sched, int sleep_fd, int data_fd, int log_fd,
    int control_fd)
{
    status retval, release_retval;
    psock* sleep_sock = NULL;
    psock* data_sock = NULL;
    psock* log_sock = NULL;
    attestationservice_instance* tmp;
    fiber* main_fiber = NULL;

    /* these parameters are unused. */
    (void)control_fd;

    /* allocate memory for the instance. */
    TRY_OR_FAIL(
        rcpr_allocator_allocate(
            alloc, (void**)&tmp, sizeof(attestationservice_instance)),
        done);

    /* clear out structure. */
    memset(tmp, 0, sizeof(attestationservice_instance));

    /* set up resource. */
    resource_init(&tmp->hdr, &attestationservice_instance_resource_release);

    /* set init values. */
    tmp->alloc = alloc;

    /* get the main fiber. */
    TRY_OR_FAIL(
        disciplined_fiber_scheduler_main_fiber_get(&main_fiber, sched),
        cleanup_tmp);

    /* set up the base sleep sock. */
    TRY_OR_FAIL(
        psock_create_from_descriptor(&sleep_sock, alloc, sleep_fd),
        cleanup_socks);

    /* set the fiber sleep sock. */
    TRY_OR_FAIL(
        psock_create_wrap_async(
            &tmp->sleep_sock, alloc, main_fiber, sleep_sock),
        cleanup_socks);

    /* the base sleep socket is now owned by the instance. */
    sleep_sock = NULL;

    /* set up the base data sock. */
    TRY_OR_FAIL(
        psock_create_from_descriptor(&data_sock, alloc, data_fd),
        cleanup_socks);

    /* set the fiber data sock. */
    TRY_OR_FAIL(
        psock_create_wrap_async(
            &tmp->data_sock, alloc, main_fiber, data_sock),
        cleanup_socks);

    /* the base data socket is now owned by the instance. */
    data_sock = NULL;

    /* set up the base log sock. */
    TRY_OR_FAIL(
        psock_create_from_descriptor(&log_sock, alloc, log_fd),
        cleanup_socks);

    /* set the fiber log sock. */
    TRY_OR_FAIL(
        psock_create_wrap_async(
            &tmp->log_sock, alloc, main_fiber, log_sock),
        cleanup_socks);

    /* the base log socket is now owned by the instance. */
    log_sock = NULL;

    /* create the transaction rbtree. */
    TRY_OR_FAIL(
        rbtree_create(
            &tmp->transaction_tree, alloc, &txn_tree_compare, &txn_tree_key,
            NULL),
        cleanup_tmp);

    /* create the artifact rbtree. */
    TRY_OR_FAIL(
        rbtree_create(
            &tmp->artifact_tree, alloc, &artifact_tree_compare,
            &artifact_tree_key, NULL),
        cleanup_tmp);

    /* TODO - add config setup and I/O. */

    /* success. */
    *inst = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_socks:
    if (NULL != sleep_sock)
    {
        CLEANUP_OR_FALLTHROUGH(
            resource_release(psock_resource_handle(sleep_sock)));
        sleep_sock = NULL;
    }

    if (NULL != data_sock)
    {
        CLEANUP_OR_FALLTHROUGH(
            resource_release(psock_resource_handle(data_sock)));
        data_sock = NULL;
    }

    if (NULL != log_sock)
    {
        CLEANUP_OR_FALLTHROUGH(
            resource_release(psock_resource_handle(log_sock)));
        log_sock = NULL;
    }

cleanup_tmp:
    CLEANUP_OR_FALLTHROUGH(resource_release(&tmp->hdr));

done:
    return retval;
}

/**
 * \brief Release the attestation service instance.
 *
 * \param r         The attestation service resource.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status attestationservice_instance_resource_release(resource* r)
{
    status sleep_sock_retval = STATUS_SUCCESS;
    status data_sock_retval = STATUS_SUCCESS;
    status log_sock_retval = STATUS_SUCCESS;
    status release_retval = STATUS_SUCCESS;
    status transaction_rbtree_retval = STATUS_SUCCESS;
    status artifact_rbtree_retval = STATUS_SUCCESS;

    attestationservice_instance* inst = (attestationservice_instance*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = inst->alloc;

    /* release sleep sock. */
    if (NULL != inst->sleep_sock)
    {
        sleep_sock_retval =
            resource_release(psock_resource_handle(inst->sleep_sock));
    }

    /* release data sock. */
    if (NULL != inst->data_sock)
    {
        data_sock_retval =
            resource_release(psock_resource_handle(inst->data_sock));
    }

    /* release log sock. */
    if (NULL != inst->log_sock)
    {
        log_sock_retval =
            resource_release(psock_resource_handle(inst->log_sock));
    }

    /* release transaction rbtree. */
    if (NULL != inst->transaction_tree)
    {
        transaction_rbtree_retval =
            resource_release(rbtree_resource_handle(inst->transaction_tree));
    }

    /* release artifact rbtree. */
    if (NULL != inst->artifact_tree)
    {
        artifact_rbtree_retval =
            resource_release(rbtree_resource_handle(inst->artifact_tree));
    }

    /* reclaim the instance structure. */
    release_retval = rcpr_allocator_reclaim(alloc, inst);

    /* decode the return code for this function. */
    if (STATUS_SUCCESS != sleep_sock_retval)
    {
        return sleep_sock_retval;
    }
    else if (STATUS_SUCCESS != data_sock_retval)
    {
        return data_sock_retval;
    }
    else if (STATUS_SUCCESS != log_sock_retval)
    {
        return log_sock_retval;
    }
    else if (STATUS_SUCCESS != transaction_rbtree_retval)
    {
        return transaction_rbtree_retval;
    }
    else if (STATUS_SUCCESS != artifact_rbtree_retval)
    {
        return artifact_rbtree_retval;
    }
    else
    {
        return release_retval;
    }
}

/**
 * \brief Compare two transaction ID values.
 */
static rcpr_comparison_result txn_tree_compare(
    void* context, const void* lhs, const void* rhs)
{
    (void)context;

    /* compare the two UUID values. */
    int val = memcmp(lhs, rhs, 16);
    if (val < 0)
        return RCPR_COMPARE_LT;
    if (val > 0)
        return RCPR_COMPARE_GT;
    else
        return RCPR_COMPARE_EQ;
}

/**
 * \brief Get the key value for a transaction tree node value.
 */
static const void* txn_tree_key(
    void* context, const resource* r)
{
    (void)context;
    transaction_record_value* val = (transaction_record_value*)r;

    return val->data.key;
}

/**
 * \brief Compare two artifact ID values.
 */
static rcpr_comparison_result artifact_tree_compare(
    void* context, const void* lhs, const void* rhs)
{
    (void)context;

    /* compare the two UUID values. */
    int val = memcmp(lhs, rhs, 16);
    if (val < 0)
        return RCPR_COMPARE_LT;
    if (val > 0)
        return RCPR_COMPARE_GT;
    else
        return RCPR_COMPARE_EQ;
}

/**
 * \brief Get the key value for a artifact tree node value.
 */
static const void* artifact_tree_key(
    void* context, const resource* r)
{
    (void)context;
    artifact_record_value* val = (artifact_record_value*)r;

    return val->data.key;
}
