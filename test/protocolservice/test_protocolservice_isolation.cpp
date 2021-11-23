/**
 * \file test_protocolservice_isolation.cpp
 *
 * Isolation tests for the protocol service.
 *
 * \copyright 2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/api.h>
#include <agentd/protocolservice/control_api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_protocolservice_isolation.h"

using namespace std;

#if defined(AGENTD_NEW_PROTOCOL)

/**
 * Test that we can spawn the unauthorized protocol service.
 */
TEST_F(protocolservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, proto_proc_status);
}

#endif /* defined(AGENTD_NEW_PROTOCOL) */
