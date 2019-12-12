/**
 * \file agentd/consensusservice/api.h
 *
 * \brief Internal API for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONSENSUSSERVICE_API_HEADER_GUARD
#define AGENTD_CONSENSUSSERVICE_API_HEADER_GUARD

#include <agentd/dataservice.h>
#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Consensus service API methods.
 */
enum consensusservice_api_method_enum
{
    /**
     * \brief Lower bound of API methods.  Must be the first value in this
     * enumeration.
     */
    CONSENSUSSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Configure the consensus service.
     */
    CONSENSUSSERVICE_API_METHOD_CONFIGURE =
        CONSENSUSSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Start the consensus service.
     */
    CONSENSUSSERVICE_API_METHOD_START,

    /**
     * \brief Stop the consensus service.
     */
    CONSENSUSSERVICE_API_METHOD_STOP,

    /**
     * \brief The number of methods in this API.
     *
     * Must be immediately after the last enumerated method ID.
     */
    CONSENSUSSERVICE_API_METHOD_UPPER_BOUND
};

/**
 * \brief Configure the consensus service.
 *
 * \param sock          The socket on which this request is made.
 * \param conf          The config data for this agentd instance.
 *
 * This must be the first API call on the consensus control socket.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if an error
 *        occurred when writing to the socket.
 */
int consensus_api_sendreq_configure(
    int sock, const agent_config_t* conf);

/**
 * \brief Receive a response from the consensus service configure call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_READ_DATA_FAILURE if reading data
 *        from the socket failed.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if
 *        the data packet size is unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int consensus_api_recvresp_configure(
    int sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Start the consensus service.
 *
 * \param sock          The socket on which this request is made.
 *
 * This call starts the consensus service, and must occur after it has been
 * successfully configured.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if an error
 *        occurred when writing to the socket.
 */
int consensus_api_sendreq_start(
    int sock);

/**
 * \brief Receive a response from the consensus service start call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_READ_DATA_FAILURE if reading data
 *        from the socket failed.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if
 *        the data packet size is unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int consensus_api_recvresp_start(
    int sock, uint32_t* offset, uint32_t* status);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONSENSUSSERVICE_API_HEADER_GUARD*/
