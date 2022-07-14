/**
 * \file protocolservice/protocolservice_capabilities.c
 *
 * \brief The list of capability definitions supported by the protocol service.
 *
 * \copyright 2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/protocolservice_capabilities.h>

RCPR_IMPORT_uuid;

/**
 * \brief Get the latest block id.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_LATEST_READ = {
    .data = {
        0xc5, 0xb0, 0xeb, 0x04, 0x6b, 0x24, 0x48, 0xbe,
        0xb7, 0xd9, 0xbf, 0x90, 0x83, 0xa4, 0xbe, 0x5d
    }
};

/**
 * \brief Submit a transaction.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_SUBMIT = {
    .data = {
        0xef, 0x56, 0x0d, 0x24, 0xee, 0xa6, 0x48, 0x47,
        0x90, 0x09, 0x46, 0x4b, 0x12, 0x7f, 0x24, 0x9b
    }
};

/**
 * \brief Read a block by id.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_BLOCK_READ = {
    .data = {
        0xf3, 0x82, 0xe3, 0x65, 0x12, 0x24, 0x43, 0xb4,
        0x92, 0x4a, 0x1d, 0xe4, 0xd9, 0xf4, 0xcf, 0x25
    }
};

/**
 * \brief Read a block id by block height.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_BLOCK_ID_BY_HEIGHT_READ = {
    .data = {
        0x91, 0x5a, 0x5e, 0xf4, 0x8f, 0x96, 0x4e, 0xf5,
        0x95, 0x88, 0x0a, 0x75, 0xb1, 0xca, 0xe6, 0x8d
    }
};

/**
 * \brief Read a transaction by id.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_TRANSACTION_READ = {
    .data = {
        0x7d, 0xf2, 0x10, 0xd6, 0xf0, 0x0b, 0x47, 0xc4,
        0xa6, 0x08, 0x6f, 0x3f, 0x1d, 0xf7, 0x51, 0x1a
    }
};

/**
 * \brief Read an artifact by id.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_ARTIFACT_READ = {
    .data = {
        0xfc, 0x0e, 0x22, 0xea, 0x1e, 0x77, 0x4e, 0xa4,
        0xa2, 0xae, 0x08, 0xbe, 0x5f, 0xf7, 0x3c, 0xcc
    }
};

/**
 * \brief Assert that a given block id is the latest.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_ASSERT_LATEST_BLOCK_ID = {
    .data = {
        0x44, 0x76, 0x17, 0xb4, 0xa8, 0x47, 0x43, 0x7c,
        0xb6, 0x2b, 0x5b, 0xc6, 0xa9, 0x42, 0x06, 0xfa
    }
};

/**
 * \brief Cancel a block assertion
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_ASSERT_LATEST_BLOCK_ID_CANCEL = {
    .data = {
        0xd8, 0x48, 0xb1, 0x18, 0x7c, 0x34, 0x46, 0xc5,
        0x80, 0xdb, 0xd4, 0xff, 0xd9, 0x21, 0xbb, 0x50
    }
};

/**
 * \brief Extend the API so that this entity can accept API requests.
 */
const rcpr_uuid PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_ENABLE = {
    .data = {
        0xc4, 0x1b, 0x05, 0x3c, 0x6b, 0x4a, 0x40, 0xa1,
        0x98, 0x1b, 0x88, 0x2b, 0xde, 0xff, 0xe9, 0x78
    }
};

/**
 * \brief Respond to a client extended API request.
 */
const RCPR_SYM(rcpr_uuid) PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_RESP = {
    .data = {
        0x25, 0x79, 0x5b, 0x47, 0xb0, 0xf0, 0x45, 0x6f,
        0xaa, 0xc4, 0x22, 0x13, 0x1f, 0x4e, 0xac, 0xe2
    }
};

/**
 * \brief Send an extended api request.
 */
const RCPR_SYM(rcpr_uuid)
PROTOCOLSERVICE_API_CAPABILITY_EXTENDED_API_SENDRECV = {
    .data = {
        0x51, 0xb9, 0xe4, 0x24, 0x0c, 0x45, 0x49, 0x1b,
        0x9b, 0xda, 0x69, 0x0e, 0x10, 0x87, 0x3c, 0x1c
    }
};
