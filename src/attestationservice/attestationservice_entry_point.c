/**
 * \file attestationservice/attestationservice_entry_point.c
 *
 * \brief The entry point for the attestation service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/attestationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

/**
 * \brief Entry point for the attestation service.
 *
 * \param datasock      The data service socket.  The attestation service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The attestation service
 *                      logs on this socket.
 * \param controlsock   The socket used to control the attestation service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - a non-zero error code on failure.
 */
int attestationservice_entry_point(
    int UNUSED(datasock), int UNUSED(logsock), int UNUSED(controlsock))
{
    /* TODO - this is currently a stub service. */
    for (;;)
    {
        sleep(5);
    }
}
