/**
 * \file agentd/notificationservice/api.h
 *
 * \brief API for the notification service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_NOTIFICATIONSERVICE_API_HEADER_GUARD
#define AGENTD_NOTIFICATIONSERVICE_API_HEADER_GUARD

#include <agentd/notificationservice.h>
#include <rcpr/psock.h>
#include <stdbool.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus*/

enum notificationservice_api_methods
{
    AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_REDUCE_CAPS                = 0x00,
    AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_UPDATE               = 0x01,
    AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION            = 0x02,
    AGENTD_NOTIFICATIONSERVICE_API_METHOD_ID_BLOCK_ASSERTION_CANCEL     = 0x03,
};

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
    size_t* size);

/**
 * \brief Encode a request to the notification service connection, encoding
 * the method_id, the offset, and the payload into an allocated buffer.
 *
 * \param buf           Pointer to receive the allocated buffer.
 * \param size          Pointer to receive the buffer size.
 * \param alloc         The allocator to use for this operation.
 * \param method_id     The method id for this response.
 * \param offset        The offset for this response.
 * \param payload       Additional payload for this response.
 * \param payload_size  The size of this additional payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_encode_request(
    uint8_t** buf, size_t* size, RCPR_SYM(allocator)* alloc,
    uint32_t method_id, uint64_t offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode a request from the client, decoding the method_id, the offset,
 * and the payload.
 *
 * \param buf           The buffer to decode.
 * \param size          The size of this buffer.
 * \param method_id     Pointer to receive the method id.
 * \param offset        Pointer to receive the offset.
 * \param payload       Pointer to be updated to the start of the payload.
 * \param payload_size  Pointer to receive the size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_decode_request(
    const uint8_t* buf, size_t size, uint32_t* method_id, uint64_t* offset,
    const uint8_t** payload, size_t* payload_size);

/**
 * \brief Encode a response from the notification service connection, encoding
 * the method_id, the status, and the offset into an allocated buffer.
 *
 * \param buf           Pointer to receive the allocated buffer.
 * \param size          Pointer to receive the buffer size.
 * \param alloc         The allocator to use for this operation.
 * \param method_id     The method id for this response.
 * \param status_code   The status for this response.
 * \param offset        The offset for this response.
 * \param payload       Additional payload for this response.
 * \param payload_size  The size of this additional payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_encode_response(
    uint8_t** buf, size_t* size, RCPR_SYM(allocator)* alloc,
    uint32_t method_id, uint32_t status_code, uint64_t offset,
    const uint8_t* payload, size_t payload_size);

/**
 * \brief Decode a response from the notification service connection, decoding
 * the method_id, the status, and the offset.
 *
 * \param buf           The buffer to decode.
 * \param size          The size of this buffer.
 * \param method_id     Pointer to receive the method id.
 * \param status_code   Pointer to receive the status.
 * \param offset        Pointer to receive the offset.
 * \param payload       Pointer to be updated to the start of the payload.
 * \param payload_size  Pointer to receive the size of the payload.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_decode_response(
    const uint8_t* buf, size_t size, uint32_t* method_id, uint32_t* status_code,
    uint64_t* offset, const uint8_t** payload, size_t* payload_size);

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
    uint32_t* caps, size_t size);

/**
 * \brief Send a block update notification request to the notification service.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this operation.
 * \param block_id  The new block id.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_block_update(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset,
    const RCPR_SYM(rcpr_uuid)* block_id);

/**
 * \brief Assert that the given block id is the latest, and receive an
 * invalidation, potentially at a later date, if this block id is not the
 * latest.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this assertion.
 * \param block_id  The block id that this process is asserting to be the
 *                  latest.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_block_assertion(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset,
    const RCPR_SYM(rcpr_uuid)* block_id);

/**
 * \brief Cancel an assertion at the given offset, which will send a cancel
 * response at that offset.
 *
 * \param sock      The socket on which this request is made.
 * \param alloc     The allocator to use for this operation.
 * \param offset    The unique offset for this assertion.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
status notificationservice_api_sendreq_assertion_cancel(
    RCPR_SYM(psock)* sock, RCPR_SYM(allocator)* alloc, uint64_t offset);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  /*__cplusplus*/

#endif /*AGENTD_NOTIFICATIONSERVICE_API_HEADER_GUARD*/
