/**
 * \file readers/config_read_private_key_proc.c
 *
 * \brief Spawn a process as the blockchain user/group to read the private key
 * file.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/status_codes.h>

/* forward decls. */
static void private_key_dispose(void* disp);

/**
 * \brief Spawn a process to read the private key file, populating the
 * provided private key structure.
 *
 * On success, a private key file structure is initialized with data from the
 * private key reader process. This is owned by the caller and must be
 * disposed by calling \ref dispose() when no longer needed.
 *
 * \param bconf         The bootstrap configuration used to spawn the process.
 * \param conf          The config structure used to spawn the process.
 * \param private_key   The \ref config_private_key_t to populate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_PROC_RUNSECURE_ROOT_USER_REQUIRED if spawning this
 *        process failed because the user is not root and runsecure is true.
 *      - AGENTD_ERROR_CONFIG_IPC_SOCKETPAIR_FAILURE if creating a socketpair
 *        for the dataservice process failed.
 *      - AGENTD_ERROR_CONFIG_FORK_FAILURE if forking the private process
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there was a
 *        failure looking up the configured user and group for the process.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_CHROOT_FAILURE if chrooting failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_DROP_PRIVILEGES_FAILURE if dropping
 *        privileges failed.
 *      - AGENTD_ERROR_CONFIG_OPEN_CONFIG_FILE_FAILURE if opening the config
 *        file failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE if setting file descriptors
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_PRIVATE_FAILURE if executing the
 *        private command failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the process
 *        survived execution (weird!).      
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if reading data from the
 *        config stream failed.
 *      - AGENTD_ERROR_CONFIG_PROC_EXIT_FAILURE if the config proc did not
 *        properly exit.
 *      - AGENTD_ERROR_CONFIG_DEFAULTS_SET_FAILURE if setting the config
 *        defaults failed.
 */
int config_read_private_key_proc(
    const struct bootstrap_config* bconf, agent_config_t* conf,
    config_private_key_t* private_key)
{
    (void)bconf;
    (void)conf;

    /* TODO  fill out. */
    private_key->hdr.dispose = &private_key_dispose;
    private_key->found = false;

    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a private key.
 *
 * \param disp      The private key to dispose.
 */
static void private_key_dispose(void* disp)
{
    config_private_key_t* private_key = (config_private_key_t*)disp;

    /* if the private key was not found, nothing else needs to be done to clean
     * it up. */
    if (!private_key->found)
    {
        goto private_key_clear;
    }

    /* clean up key data. */
    dispose((disposable_t*)&private_key->enc_pubkey);
    dispose((disposable_t*)&private_key->enc_privkey);
    dispose((disposable_t*)&private_key->sign_pubkey);
    dispose((disposable_t*)&private_key->sign_privkey);

private_key_clear:
    memset(private_key, 0, sizeof(config_private_key_t));
}
