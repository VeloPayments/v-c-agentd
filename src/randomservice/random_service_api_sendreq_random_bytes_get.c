/**
 * \file randomservice/random_service_api_recvresp_random_bytes_get.c
 *
 * \brief Request some random bytes from the random service.
 *
 * \copyright 2020-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/randomservice.h>
#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <rcpr/psock.h>
#include <unistd.h>
#include <vpr/parameters.h>

RCPR_IMPORT_psock;

/**
 * \brief Request some random bytes from the random service.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset for the request; will be returned in the
 *                      response.
 * \param count         The number of bytes requested.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int random_service_api_sendreq_random_bytes_get(
    RCPR_SYM(psock)* sock, uint32_t offset, uint32_t count)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(prop_psock_valid(sock));

    /* + ------------------------------------------------------------ + */
    /* | Random bytes read request.                                   | */
    /* + --------------------------------------------- + ------------ + */
    /* | DATA                                          | SIZE         | */
    /* + --------------------------------------------- + ------------ + */
    /* | RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES     | 4 bytes      | */
    /* | request offset                                | 4 bytes      | */
    /* | number of bytes                               | 4 bytes      | */
    /* + --------------------------------------------- + ------------ + */

    /* build the payload. */
    uint32_t payload[3] = {
        htonl(RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES),
        htonl(offset),
        htonl(count)
    };

    /* write a data packet to the random socket. */
    status retval = psock_write_boxed_data(sock, payload, sizeof(payload));
    if (STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up the request payload. */
    memset(payload, 0, sizeof(payload));

    /* return the status of this request to the caller. */
    return retval;
}
