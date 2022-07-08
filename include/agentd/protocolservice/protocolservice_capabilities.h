/**
 * \file agentd/protocolservice/protocolservice_capabilities.h
 *
 * \brief The list of capabilities supported by the protocol service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

#include <rcpr/uuid.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Get the latest block id.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_LATEST_READ;

/**
 * \brief Submit a transaction.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_SUBMIT;

/**
 * \brief Read a block by id.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_BLOCK_READ;

/**
 * \brief Read a block id by block height.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_BY_HEIGHT_READ;

/**
 * \brief Read a transaction by id.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_READ;

/**
 * \brief Read an artifact by id.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_ARTIFACT_READ;

/**
 * \brief Assert that a given block id is the latest.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_ASSERT_LATEST_BLOCK_ID;

/**
 * \brief Cancel a block assertion.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_ASSERT_LATEST_BLOCK_ID_CANCEL;

/**
 * \brief Extend the API so that this entity can accept API requests.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_ENABLE;

/**
 * \brief Respond to a client extended API request.
 */
extern const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_RESP;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

