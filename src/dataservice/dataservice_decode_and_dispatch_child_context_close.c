/**
 * \file dataservice/dataservice_decode_and_dispatch_child_context_close.c
 *
 * \brief Decode requests and dispatch a child context close call.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
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
 * \brief Decode and dispatch a child context close request.
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
int dataservice_decode_and_dispatch_child_context_close(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    bool dispose_dreq = false;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* child context close request structure. */
    dataservice_request_child_context_close_t dreq;

    /* parse the request payload. */
    retval = dataservice_decode_request_child_context_close(req, size, &dreq);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* be sure to clean up dreq. */
    dispose_dreq = true;

    /* look up the child context. */
    dataservice_child_context_t* ctx = NULL;
    retval = dataservice_child_context_lookup(&ctx, inst, dreq.hdr.child_index);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* call the child context close method. */
    retval = dataservice_child_context_close(ctx);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_CHILD_DETAILS_DELETE_FAILURE;
        goto done;
    }

    /* clean up the child instance. */
    dataservice_child_details_delete(inst, dreq.hdr.child_index);

    /* success. Fall through. */

done:
    /* write the status to output. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE,
            dreq.hdr.child_index, (uint32_t)retval, NULL, 0);

    /* clean up dreq. */
    if (dispose_dreq)
    {
        dispose((disposable_t*)&dreq);
    }

    return retval;
}
