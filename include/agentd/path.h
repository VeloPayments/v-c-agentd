/**
 * \file agentd/path.h
 *
 * \brief Utility methods for resolving paths.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PATH_HEADER_GUARD
#define AGENTD_PATH_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Append the default path onto a given path.
 *
 * On success, outpath is updated to the appended path.  This value is owned by
 * the caller and must be free()d when no longer needed.
 *
 * \param path              The path to append.
 * \param outpath           The character pointer that this parameter points to
 *                          is updated to the appended path on success.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if the operation cannot
 *            be completed due to a memory allocation error.
 */
int path_append_default(const char* path, char** outpath);

/**
 * \brief Given a filename and a path, attempt to resolve the filename using
 * this path.
 *
 * On success, resolved is updated to the resolved filename.  This value is
 * owned by the caller and must be free()d when no longer needed.
 *
 * \param filename          The filename to resolve.
 * \param path              A colon separated list of path values.
 * \param resolved          The character pointer that this parameter points to
 *                          is updated to the resolved path on success.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if the operation cannot
 *            be completed due to a memory allocation error.
 *          - AGENTD_ERROR_GENERAL_PATH_NOT_FOUND if the filename could not be
 *            found in the given path.
 */
int path_resolve(const char* filename, const char* path, char** resolved);

/**
 * \brief Given a pathname, return the directory portion of this pathname.
 *
 * On success, dirname is updated to the directory portion of the filename.
 * This value is owned by the caller and must be free()d when no longer needed.
 *
 * \param filename          The filename to resolve.
 * \param dirname           The character pointer that this parameter points to
 *                          is updated to the directory name on success.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if the operation cannot
 *            be completed due to a memory allocation error.
 */
int path_dirname(const char* filename, char** dirname);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PATH_HEADER_GUARD*/
