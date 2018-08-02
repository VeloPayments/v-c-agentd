/**
 * \file agentd/privsep.h
 *
 * \brief Privilege separation support.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PRIVSEP_HEADER_GUARD
#define AGENTD_PRIVSEP_HEADER_GUARD

#include <grp.h>
#include <pwd.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Get the user and group IDs by name.
 *
 * \param user          The user name to look up.
 * \param group         The group name to look up.
 * \param uid           Pointer to the user ID to set.
 * \param gid           Pointer to the group ID to set.
 *
 * \returns 0 on success and non-zero on failure.
 */
int privsep_lookup_usergroup(
    const char* user, const char* group, uid_t* uid, gid_t* gid);

/**
 * \brief Change the root directory.
 *
 * Must be root.
 *
 * \param dir           The new root directory.
 *
 * \returns 0 on success and non-zero on failure.
 */
int privsep_chroot(const char* dir);

/**
 * \brief Assume the given user and group, dropping root privileges.
 *
 * Must be root.
 *
 * \param user          The user ID to assume.
 * \param group         The group ID to assume.
 *
 * \returns 0 on success and non-zero on failure.
 */
int privsep_drop_privileges(uid_t user, gid_t gid);

/**
 * \brief Execute a private command.
 *
 * \param command       The private command to execute.
 *
 * \returns non-zero on failure.  Does not return on success; process is
 * replaced.
 */
int privsep_exec_private(const char* command);

/**
 * \brief Set file descriptors for a new process.
 *
 * Descriptors are described in pairs.  The first descriptor is the current
 * descriptor, and the second descriptor is the descriptor that the first is
 * mapped to in the new process.  A negative value ends this sequence.  A
 * negative value must be the last value in this sequence to act as a sentry
 * value.
 *
 * This method also closes all standard descriptors, such as standard in,
 * standard out, and standard error.
 *
 * \param curr          The current descriptor.
 * \param mapped        The mapped descriptor.
 *
 * \returns Zero on success and non-zero on failure.
 */
int privsep_setfds(int curr, int mapped, ...);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PRIVSEP_HEADER_GUARD*/
