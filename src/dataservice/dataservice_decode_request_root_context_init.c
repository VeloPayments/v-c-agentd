/**
 * \file dataservice/dataservice_decode_request_root_context_init.c
 *
 * \brief Decode a root context init request payload.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include "dataservice_protocol_internal.h"

/* forward decls. */
static void dataservice_decode_request_root_context_init_disposer(void* disp);

/**
 * \brief Decode a root context init request into its constituent pieces.
 *
 * \param req           The request payload to parse.
 * \param alloc_opts    The allocator options to use for this operation.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_root_context_init(
    const void* req, allocator_options_t* alloc_opts, size_t size,
    dataservice_request_payload_root_context_init_t* dreq)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(prop_allocator_options_valid(alloc_opts));
    MODEL_ASSERT(NULL != dreq);

    /* runtime parameter sanity checks. */
    if (NULL == req || NULL == alloc_opts || NULL == dreq)
    {
        return AGENTD_ERROR_DATASERVICE_INVALID_PARAMETER;
    }

    /* the payload size should be greater than eight. */
    if (size <= 8U)
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* clear out the structure. */
    memset(dreq, 0, sizeof(*dreq));

    /* set the dispose method and size. */
    dreq->hdr.hdr.dispose =
        &dataservice_decode_request_root_context_init_disposer;
    dreq->hdr.size = sizeof(*dreq);

    /* set the child index to 0. */
    dreq->hdr.child_index = 0U;

    /* save the allocator. */
    dreq->alloc_opts = alloc_opts;

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* get the max database size. */
    uint64_t net_max_database_size = 0U;
    memcpy(&net_max_database_size, breq, sizeof(net_max_database_size));
    dreq->max_database_size = ntohll(net_max_database_size);
    size -= sizeof(net_max_database_size);
    breq += sizeof(net_max_database_size);

    /* allocate memory for the datadir string. */
    dreq->datadir = allocate(alloc_opts, size + 1);
    if (NULL == dreq->datadir)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the datadir string. */
    memcpy(dreq->datadir, breq, size);
    dreq->datadir[size] = 0;
    dreq->datadir_size = size;

    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Disposer for the root context init request structure.
 *
 * \param disp      An opaque pointer to the structure to be disposed.
 */
static void dataservice_decode_request_root_context_init_disposer(void* disp)
{
    dataservice_request_payload_root_context_init_t* req =
        (dataservice_request_payload_root_context_init_t*)disp;

    /* clear datadir. */
    memset(req->datadir, 0, req->datadir_size);

    /* release datadir. */
    release(req->alloc_opts, req->datadir);

    /* clear the data structure. */
    memset(req, 0, sizeof(*req));
}
