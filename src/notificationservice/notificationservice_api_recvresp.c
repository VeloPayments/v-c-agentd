/**
 * \file notificationservice/notificationservice_api_recvresp.c
 *
 * \brief Receive a response from the notificationservice API.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/notificationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

RCPR_IMPORT_psock;

/**
 * \brief Receive a response from the notification service connection.
 *
 * \param sock      The socket from which to read the response.
 * \param alloc     The allocator to use for this operation.
 * \param buf       Pointer to receive the response buffer.
 * \param size      Pointer to receive the buffer size.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_recvresp(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint8_t** buf,
    size_t* size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));
    MODEL_ASSERT(rcpr_prop_allocator_valid(alloc));
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != size);

    /* runtime parameter checks. */
    if (NULL == sock || NULL == alloc || NULL == buf || NULL == size)
    {
        return AGENTD_ERROR_NOTIFICATIONSERVICE_API_BAD_ARGUMENT;
    }

    /* read a boxed data packet. */
    return psock_read_boxed_data(sock, alloc, (void**)buf, size);
}
