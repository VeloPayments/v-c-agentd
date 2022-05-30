/**
 * \file notificationservice/notificationservice_api_sendreq_block_assertion.c
 *
 * \brief Send a block assertion request.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/inet.h>
#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_psock;

/**
 * \brief Assert that the given block id is the latest, and receive an
 * invalidation, potentially at a later date, if this block id is not the
 * latest.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this assertion.
 * \param block_id  The block id that this process is asserting to be the
 *                  latest.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_block_assertion(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset,
    const RCPR_SYM(rcpr_uuid)* block_id)
{
    status retval, release_retval;
    uint8_t* buf;
    size_t bufsize;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != caps);

    /* runtime parameter checks. */
    if (NULL == sock || NULL == alloc || NULL == block_id)
    {
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
        goto done;
    }

    /* encode request. */
    retval =
        notificationservice_api_encode_request(
            &buf, &bufsize, alloc,
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE, offset,
            (const uint8_t*)block_id, sizeof(*block_id));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* send request. */
    retval = psock_write_boxed_data(sock, buf, bufsize);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_buffer;
    }

    /* success. */
    retval = STATUS_SUCCESS;
    goto cleanup_buffer;

cleanup_buffer:
    release_retval = rcpr_allocator_reclaim(alloc, buf);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}
