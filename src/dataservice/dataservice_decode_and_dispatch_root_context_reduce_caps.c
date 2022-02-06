/**
 * \file dataservice/dataservice_decode_and_dispatch_root_context_reduce_caps.c
 *
 * \brief Decode and dispatch a root context reduce capabilities call.
 *
 * \copyright 2018-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"
#include "dataservice_protocol_internal.h"

/**
 * \brief Decode and dispatch a root capabilities reduction request.
 *
 * Returns 0 on success or non-fatal error.  If a non-zero error message is
 * returned, then a fatal error has occurred that should not be recovered from.
 * Any additional information on the socket is suspect.
 *
 * \param inst          The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int dataservice_decode_and_dispatch_root_context_reduce_caps(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* request structure. */
    dataservice_request_payload_root_context_reduce_caps_t dreq;

    /* parse the request. */
    retval =
        dataservice_decode_request_root_context_reduce_caps(req, size, &dreq);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* call the root context reduce capabilites method. */
    retval =
        dataservice_root_context_reduce_capabilities(&inst->ctx, dreq.caps);

    /* cleanup. */
    dispose((disposable_t*)&dreq);

done:
    /* write the status to output. */
    return dataservice_decode_and_dispatch_write_status(
        sock, DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS, 0,
        (uint32_t)retval, NULL, 0);
}
