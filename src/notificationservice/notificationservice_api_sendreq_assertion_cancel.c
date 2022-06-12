/**
 * \file notificationservice/notificationservice_api_sendreq_assertion_cancel.c
 *
 * \brief Send an assertion cancel request.
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
 * \brief Cancel an assertion at the given offset, which will send a cancel
 * response at that offset.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this assertion.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_assertion_cancel(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset)
{
    status retval, release_retval;
    uint8_t* buf;
    size_t bufsize;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));

    /* runtime parameter checks. */
    if (NULL == sock || NULL == alloc)
    {
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
        goto done;
    }

    /* encode request. */
    retval =
        notificationservice_api_encode_request(
            &buf, &bufsize, alloc,
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL,
            offset, NULL, 0U);
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
