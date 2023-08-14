/**
 * \file test_mock_notificationservice.h
 *
 * Private header for the mock notificationservice unit tests.
 *
 * \copyright 2022-2023 Velo Payments, Inc.  All rights reserved.
 */

#pragma once

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

#include <agentd/notificationservice.h>
#include <agentd/notificationservice/api.h>
#include "../mocks/notificationservice.h"

class mock_notificationservice_test {
public:
    void setUp();
    void tearDown();

    std::unique_ptr<mock_notificationservice::mock_notificationservice> mock;
    int notifysock;
    RCPR_SYM(psock)* sock;
    RCPR_SYM(allocator)* alloc;
    bool test_suite_valid;
};
