/**
 * \file agentd/control.h
 *
 * \brief Control-flow helper macros.
 *
 * \copyright 2019-2021 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONTROL_HEADER_GUARD
#define AGENTD_CONTROL_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * \brief Helper macro to goto a label on failure.
 *
 * This macro assumes a "goto fail" organizational pattern with a function that
 * returns a return value which can either be AGENTD_STATUS_SUCESS or some
 * failure code.
 */
#define TRY_OR_FAIL(x, label) \
    do \
    { \
        retval = x; \
        if (AGENTD_STATUS_SUCCESS != retval) \
        { \
            goto label; \
        } \
    } while (0)

/**
 * \brief Helper macro to attempt cleanup while coalescing a cleanup error into
 * the return value.
 */
#define CLEANUP_OR_FALLTHROUGH(x) \
    do \
    { \
        release_retval = x; \
        if (AGENTD_STATUS_SUCCESS != release_retval) \
        { \
            retval = release_retval; \
        } \
    } while (0)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_CONTROL_HEADER_GUARD*/
