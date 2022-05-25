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
extern RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_LATEST_READ;

/**
 * \brief Submit a transaction.
 */
extern RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_SUBMIT;

/**
 * \brief Read a block by id.
 */
extern RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_BLOCK_READ;

/**
 * \brief Read a block id by block height.
 */
extern RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_BY_HEIGHT_READ;

/**
 * \brief Read a transaction by id.
 */
extern RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_READ;

/**
 * \brief Read an artifact by id.
 */
extern RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_ARTIFACT_READ;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

