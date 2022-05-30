/**
 * \file notificationservice/notificationservice_api_sendreq_reduce_caps.c
 *
 * \brief Send a capabilities reduction request.
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
 * \brief Request that the capabilities of the notification service connection
 * be reduced.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this operation.
 * \param caps      The capabilities to use for this reduction.
 * \param size      The size of the capabilities in bytes.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_reduce_caps(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset,
    uint32_t* caps, size_t size)
{
    status retval, release_retval;
    BITCAP(testcap, NOTIFICATIONSERVICE_API_CAP_BITS_MAX);
    uint8_t* buf;
    size_t bufsize;

    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != caps);

    /* runtime parameter checks. */
    if (NULL == sock || NULL == alloc || NULL == caps)
    {
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
        goto done;
    }

    /* verify the cap size. */
    if (size != sizeof(testcap))
    {
        retval = AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
        goto done;
    }

    /* encode request. */
    retval =
        notificationservice_api_encode_request(
            &buf, &bufsize, alloc,
            AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS, offset,
            (const uint8_t*)caps, size);
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
