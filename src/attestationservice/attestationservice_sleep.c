/**
 * \file attestationservice/attestationservice_sleep.c
 *
 * \brief Sleep for the given amount of time using the sleep thread.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/status_codes.h>

#include "attestationservice_internal.h"

RCPR_IMPORT_psock;

/**
 * \brief Sleep for the given amount of time using the sleep thread.
 *
 * \param sleep_sock    Socket for sleep thread communication.
 * \param sleep_time    Sleep time in microseconds.
 *
 * \returns a status code on success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero failure code on failure.
 */
status attestationservice_sleep(psock* sleep_sock, uint64_t sleep_time)
{
    status retval;

    /* send a sleep request to the sleep thread. */
    TRY_OR_FAIL(psock_write_boxed_uint64(sleep_sock, sleep_time), done);

    /* receive a wake-up response from the sleep thread. */
    TRY_OR_FAIL(psock_read_boxed_uint64(sleep_sock, &sleep_time), done);

done:
    return retval;
}
