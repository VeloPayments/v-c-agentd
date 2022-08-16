/**
 * \file canonization/canonizationservice_dataservice_response_block_read.c
 *
 * \brief Handle the response from the data service block read call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <vccert/fields.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/* forward decls. */
static int parse_block(
    canonizationservice_instance_t* instance, const uint8_t* cert,
    size_t cert_size);

/**
 * \brief Handle the response from the data service block read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_block_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size)
{
    int retval;
    dataservice_response_block_get_t dresp;

    /* decode the response. */
    retval =
        dataservice_decode_response_block_get(
            resp, resp_size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval || AGENTD_STATUS_SUCCESS != dresp.hdr.status)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* get the block height. */
    instance->block_height = ntohll(dresp.node.net_block_height) + 1;

    /* parse the block certificate to get any additional details required for
     * building a block. */
    retval =
        parse_block(instance, dresp.data, dresp.data_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* get the first transaction in the process queue. */
    retval =
        canonizationservice_dataservice_sendreq_transaction_get_first(
            instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

done:;
}

/**
 * \brief Parse the previous block certificate to get any details required to
 * build the new block.
 *
 * \param instance              The canonization service instance.
 * \param cert                  The block certificate.
 * \param cert_size             The size of the block certificate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int parse_block(
    canonizationservice_instance_t* instance, const uint8_t* cert,
    size_t cert_size)
{
    int retval;
    vccert_parser_options_t parser_opts;
    vccert_parser_context_t parser;

    /* initialize the parser options. */
    retval =
        vccert_parser_options_simple_init(
            &parser_opts, &instance->alloc_opts, &instance->crypto_suite);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create a parser instance. */
    retval =
        vccert_parser_init(
            &parser_opts, &parser, cert, cert_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser_opts;
    }

    /* read the signature. */
    const uint8_t* signature;
    size_t signature_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_SIGNATURE, &signature, &signature_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* verify signature size. */
    if (signature_size != sizeof(instance->previous_block_signature))
    {
        goto cleanup_parser;
    }

    /* copy signature. */
    memcpy(instance->previous_block_signature, signature, signature_size);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_parser;

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_parser_opts:
    dispose((disposable_t*)&parser_opts);

done:
    return retval;
}
