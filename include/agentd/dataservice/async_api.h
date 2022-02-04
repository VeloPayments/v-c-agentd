/**
 * \file agentd/dataservice/async_api.h
 *
 * \brief Asynchronous API for the data service.
 *
 * \copyright 2019-2022 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD
#define AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD

#include <agentd/dataservice.h>
#include <rcpr/uuid.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Response Payload Header.
 */
typedef struct dataservice_response_header
{
    disposable_t hdr;
    uint32_t method_code;
    uint32_t offset;
    uint32_t status;
    size_t payload_size;
} dataservice_response_header_t;

/**
 * \brief Root Context Init Response.
 */
typedef struct dataservice_response_root_context_init
{
    dataservice_response_header_t hdr;
} dataservice_response_root_context_init_t;

/**
 * \brief Root Context Reduce Caps Response.
 */
typedef struct dataservice_response_root_context_reduce_caps
{
    dataservice_response_header_t hdr;
} dataservice_response_root_context_reduce_caps_t;

/**
 * \brief Child Context Create Response.
 */
typedef struct dataservice_response_child_context_create
{
    dataservice_response_header_t hdr;
    uint32_t child;
} dataservice_response_child_context_create_t;

/**
 * \brief Child Context Close Response.
 */
typedef struct dataservice_response_child_context_close
{
    dataservice_response_header_t hdr;
    uint32_t child;
} dataservice_response_child_context_close_t;

/**
 * \brief Global Settings Get Response.
 */
typedef struct dataservice_response_global_settings_get
{
    dataservice_response_header_t hdr;
    const void* data;
    size_t data_size;
} dataservice_response_global_settings_get_t;

/**
 * \brief Global Settings Set Response.
 */
typedef struct dataservice_response_global_settings_set
{
    dataservice_response_header_t hdr;
} dataservice_response_global_settings_set_t;

/**
 * \brief Transaction Submit Response.
 */
typedef struct dataservice_response_transaction_submit
{
    dataservice_response_header_t hdr;
} dataservice_response_transaction_submit_t;

/**
 * \brief Transaction Get First Response.
 */
typedef struct dataservice_response_transaction_get_first
{
    dataservice_response_header_t hdr;
    data_transaction_node_t node;
    const void* data;
    size_t data_size;
} dataservice_response_transaction_get_first_t;

/**
 * \brief Transaction Get Response.
 */
typedef struct dataservice_response_transaction_get
{
    dataservice_response_header_t hdr;
    data_transaction_node_t node;
    const void* data;
    size_t data_size;
} dataservice_response_transaction_get_t;

/**
 * \brief Canonized Transaction Get Response.
 */
typedef struct dataservice_response_canonized_transaction_get
{
    dataservice_response_header_t hdr;
    data_transaction_node_t node;
    const void* data;
    size_t data_size;
} dataservice_response_canonized_transaction_get_t;

/**
 * \brief Transaction Drop Response.
 */
typedef struct dataservice_response_transaction_drop
{
    dataservice_response_header_t hdr;
} dataservice_response_transaction_drop_t;

/**
 * \brief Transaction Promote Response.
 */
typedef struct dataservice_response_transaction_promote
{
    dataservice_response_header_t hdr;
} dataservice_response_transaction_promote_t;

/**
 * \brief Block Make Response.
 */
typedef struct dataservice_response_block_make
{
    dataservice_response_header_t hdr;
} dataservice_response_block_make_t;

/**
 * \brief Block ID by Height Get Response.
 */
typedef struct dataservice_response_block_id_by_height_get
{
    dataservice_response_header_t hdr;
    uint8_t block_id[16];
} dataservice_response_block_id_by_height_get_t;

/**
 * \brief Latest Block ID Get Response.
 */
typedef struct dataservice_response_latest_block_id_get
{
    dataservice_response_header_t hdr;
    uint8_t block_id[16];
} dataservice_response_latest_block_id_get_t;

/**
 * \brief Artifact Get Response.
 */
typedef struct dataservice_response_artifact_get
{
    dataservice_response_header_t hdr;
    data_artifact_record_t record;
} dataservice_response_artifact_get_t;

/**
 * \brief Block Get Response.
 */
typedef struct dataservice_response_block_get
{
    dataservice_response_header_t hdr;
    data_block_node_t node;
    const void* data;
    size_t data_size;
} dataservice_response_block_get_t;

/**
 * \brief The memset disposer simply clears the data structure when disposed.
 *
 * \param disposable    The disposable to clear.
 */
void dataservice_decode_response_memset_disposer(void* disposable);

/**
 * \brief Decode a root context init response into its constituent pieces.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_root_context_init(
    const void* resp, size_t size,
    dataservice_response_root_context_init_t* dresp);

/**
 * \brief Decode a response from the root context reduce capabilities call.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_root_context_reduce_caps(
    const void* resp, size_t size,
    dataservice_response_root_context_reduce_caps_t* dresp);

/**
 * \brief Decode a response from the child context create API call.
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_child_context_create(
    const void* resp, size_t size,
    dataservice_response_child_context_create_t* dresp);

/**
 * \brief Decode a response from the child context close API call.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_child_context_close(
    const void* resp, size_t size,
    dataservice_response_child_context_close_t* dresp);

/**
 * \brief Decode a response from the global settings query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_global_settings_get(
    const void* resp, size_t size,
    dataservice_response_global_settings_get_t* dresp);

/**
 * \brief Decode a response from the global settings set operation.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_global_settings_set(
    const void* resp, size_t size,
    dataservice_response_global_settings_set_t* dresp);

/**
 * \brief Receive a response from the transaction submit operation.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_submit(
    const void* resp, size_t size,
    dataservice_response_transaction_submit_t* dresp);

/**
 * \brief Decode a response from the get first transaction query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_get_first(
    const void* resp, size_t size,
    dataservice_response_transaction_get_first_t* dresp);

/**
 * \brief Decode a response from the get transaction query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_get(
    const void* resp, size_t size,
    dataservice_response_transaction_get_t* dresp);

/**
 * \brief Decode a response from the get canonized transaction query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_canonized_transaction_get(
    const void* resp, size_t size,
    dataservice_response_canonized_transaction_get_t* dresp);

/**
 * \brief Decode a response from the drop transaction action.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_drop(
    const void* resp, size_t size,
    dataservice_response_transaction_drop_t* dresp);

/**
 * \brief Decode a response from the promote transaction action.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_promote(
    const void* resp, size_t size,
    dataservice_response_transaction_promote_t* dresp);

/**
 * \brief Decode a response from the block make operation.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_block_make(
    const void* resp, size_t size,
    dataservice_response_block_make_t* dresp);

/**
 * \brief Decode a response from the get block id by height query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_block_id_by_height_get(
    const void* resp, size_t size,
    dataservice_response_block_id_by_height_get_t* dresp);

/**
 * \brief Decode a response from the get latest block id query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_latest_block_id_get(
    const void* resp, size_t size,
    dataservice_response_latest_block_id_get_t* dresp);

/**
 * \brief Decode a response from the get artifact query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_artifact_get(
    const void* resp, size_t size,
    dataservice_response_artifact_get_t* dresp);

/**
 * \brief Decode a response from the get block query.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_block_get(
    const void* resp, size_t size,
    dataservice_response_block_get_t* dresp);

/**
 * \brief Encode a request to query an artifact by ID.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child context for this request.
 * \param artifact_id   The artifact UUID for this request.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_artifact_get(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    const RCPR_SYM(rcpr_uuid)* artifact_id);

/**
 * \brief Encode a request to query a block by ID.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child context for this request.
 * \param block_id      The block UUID for this request.
 * \param read_cert     Flag set to true if the block certificate should be
 *                      returned.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_block_get(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    const RCPR_SYM(rcpr_uuid)* block_id, bool read_cert);

/**
 * \brief Encode a request to query a block id by height.
 *
 * \param buffer        Pointer to an uninitialized \ref vccrypt_buffer_t to
 *                      receive the encoded request.
 * \param alloc_opts    The allocator options to use.
 * \param child         The child context for this request.
 * \param height        The block height to query.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status dataservice_encode_request_block_id_by_height_get(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc_opts, uint32_t child,
    uint64_t height);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD*/
