/**
 * \file readers/config_read_private_key_proc.c
 *
 * \brief Spawn a process as the blockchain user/group to read the private key
 * file.
 *
 * \copyright 2020-2023 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vpr/allocator/malloc_allocator.h>

/* forward decls. */
static int config_private_key_file_send(
    int clientsock, const config_private_key_entry_t* entry);
static int config_private_key_read(
    int clientsock, allocator_options_t* alloc_opts,
    config_private_key_t* entry);
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
 * \param alloc_opts    The allocator options to use for this operation.
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
    allocator_options_t* alloc_opts, config_private_key_t* private_key)
{
    int retval = 1;
    int clientsock = -1, serversock = -1;
    int procid = 0;
    uid_t uid;
    gid_t gid;

    /* first, do we have a private key file specified? */
    if (NULL == conf->private_key)
    {
        /* no.  Mark the key as "not found." */
        memset(private_key, 0, sizeof(config_private_key_t));
        private_key->hdr.dispose = &private_key_dispose;
        private_key->found = false;
        retval = AGENTD_STATUS_SUCCESS;
        goto done;
    }

    /* verify that this process is running as root. */
    if (0 != geteuid())
    {
        retval = AGENTD_ERROR_READER_PROC_RUNSECURE_ROOT_USER_REQUIRED;
        goto done;
    }

    /* create a socketpair for communication. */
    retval = ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &clientsock, &serversock);
    if (0 != retval)
    {
        perror("ipc_socketpair");
        retval = AGENTD_ERROR_READER_IPC_SOCKETPAIR_FAILURE;
        goto done;
    }

    /* fork the process into parent and child. */
    procid = fork();
    if (procid < 0)
    {
        perror("fork");
        retval = AGENTD_ERROR_READER_FORK_FAILURE;
        goto done;
    }

    /* child */
    if (0 == procid)
    {
        /* close parent's end of the socket pair. */
        close(clientsock);
        clientsock = -1;

        /* get the user and group IDs. */
        retval =
            privsep_lookup_usergroup(
                conf->usergroup->user, conf->usergroup->group, &uid, &gid);
        if (0 != retval)
        {
            perror("privsep_lookup_usergroup");
            retval = AGENTD_ERROR_READER_PRIVSEP_LOOKUP_USERGROUP_FAILURE;
            goto done;
        }

        /* change into the prefix directory. */
        retval = privsep_chroot(bconf->prefix_dir);
        if (0 != retval)
        {
            perror("privsep_chroot");
            retval = AGENTD_ERROR_READER_PRIVSEP_CHROOT_FAILURE;
            goto done;
        }

        /* set the user ID and group ID. */
        retval = privsep_drop_privileges(uid, gid);
        if (0 != retval)
        {
            perror("privsep_drop_privileges");
            retval = AGENTD_ERROR_READER_PRIVSEP_DROP_PRIVILEGES_FAILURE;
            goto done;
        }

        /* move the fds out of the way. */
        if (AGENTD_STATUS_SUCCESS !=
            privsep_protect_descriptors(&serversock, NULL))
        {
            retval = AGENTD_ERROR_READER_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors */
        retval = privsep_close_standard_fds();
        if (0 != retval)
        {
            perror("privsep_close_standard_fds");
            retval = AGENTD_ERROR_READER_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* reset the fds. */
        retval =
            privsep_setfds(
                serversock, /* ==> */ AGENTD_FD_READER_CONTROL,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            retval = AGENTD_ERROR_READER_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close any socket above the given value. */
        retval =
            privsep_close_other_fds(AGENTD_FD_READER_CONTROL);
        if (0 != retval)
        {
            perror("privsep_close_other_fds");
            retval = AGENTD_ERROR_READER_PRIVSEP_CLOSE_OTHER_FDS;
            goto done;
        }

        /* spawn the child process (this does not return if successful. */
        retval = privsep_exec_private(bconf, "read_private_key");
        if (0 != retval)
        {
            perror("privsep_exec_private");
            retval = AGENTD_ERROR_READER_PRIVSEP_EXEC_PRIVATE_FAILURE;
            goto done;
        }

        printf("Should never get here.\n");
        retval = AGENTD_ERROR_READER_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS;
        goto done;
    }
    /* parent */
    else
    {
        int pidstatus;
        close(serversock);
        serversock = -1;

        /* send the private key to open / read. */
        if (0 != config_private_key_file_send(clientsock, conf->private_key))
        {
            retval = AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE;
            goto done;
        }

        /* read back the response. */
        if (0 != config_private_key_read(clientsock, alloc_opts, private_key))
        {
            retval = AGENTD_ERROR_READER_IPC_READ_DATA_FAILURE;
            goto done;
        }

        /* we're done with the reader proc, so close the socket. */
        close(clientsock);
        clientsock = -1;

        /* wait on the child process to complete. */
        waitpid(procid, &pidstatus, 0);

        /* Use the return value of the child process as our return value. */
        if (WIFEXITED(pidstatus))
        {
            retval = WEXITSTATUS(pidstatus);
            if (0 != retval)
            {
                retval = AGENTD_ERROR_READER_PROC_EXIT_FAILURE;
            }
        }
        else
        {
            retval = AGENTD_ERROR_READER_PROC_EXIT_FAILURE;
        }

        /* make sure to clean up the private key if we fail. */
        if (0 != retval)
            goto cleanup_private_key;
        else
            goto done;
    }

cleanup_private_key:
    dispose((disposable_t*)private_key);

done:
    /* clean up clientsock. */
    if (clientsock >= 0)
        close(clientsock);

    /* clean up serversock. */
    if (serversock >= 0)
        close(serversock);

    return retval;
}

/**
 * \brief Send a private key file to the reader proc.
 *
 * \param clientsock        Socket connection to the reader proc.
 * \param entry             The private key entry to send.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int config_private_key_file_send(
    int clientsock, const config_private_key_entry_t* entry)
{
    return ipc_write_string_block(clientsock, entry->filename);
}

/**
 * \brief Read a private key from the reader proc.
 *
 * \param clientsock        The reader process socket.
 * \param alloc_opts        The allocator to use for this operation.
 * \param entry             The entry to read. On success, it is initialized.
 *                          The caller is responsible for disposing it.;
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int config_private_key_read(
    int clientsock, allocator_options_t* alloc_opts,
    config_private_key_t* entry)
{
    int retval;
    uint8_t type;

    /* first, read the begin of message marker. */
    retval = ipc_read_uint8_block(clientsock, &type);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* verify that this is the beginning of a message. */
    if (CONFIG_STREAM_TYPE_BOM != type)
    {
        retval = AGENTD_ERROR_READER_INVALID_STREAM;
        goto done;
    }

    /* read the uuid. */
    uint8_t* uuid_data;
    uint32_t uuid_size;
    retval = ipc_read_data_block(clientsock, (void**)&uuid_data, &uuid_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* read the encryption public key. */
    uint8_t* enc_pubdata;
    uint32_t enc_pubsize;
    retval =
        ipc_read_data_block(clientsock, (void**)&enc_pubdata, &enc_pubsize);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuid_data;
    }

    /* read the encryption private key. */
    uint8_t* enc_privdata;
    uint32_t enc_privsize;
    retval =
        ipc_read_data_block(clientsock, (void**)&enc_privdata, &enc_privsize);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubdata;
    }

    /* read the signature public key. */
    uint8_t* sign_pubdata;
    uint32_t sign_pubsize;
    retval =
        ipc_read_data_block(clientsock, (void**)&sign_pubdata, &sign_pubsize);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_privdata;
    }

    /* read the signature private key. */
    uint8_t* sign_privdata;
    uint32_t sign_privsize;
    retval =
        ipc_read_data_block(clientsock, (void**)&sign_privdata, &sign_privsize);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_sign_pubdata;
    }

    /* finally, read the end of message marker. */
    retval = ipc_read_uint8_block(clientsock, &type);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_sign_privdata;
    }

    /* verify that this is the end of a message. */
    if (CONFIG_STREAM_TYPE_EOM != type)
    {
        retval = AGENTD_ERROR_READER_INVALID_STREAM;
        goto cleanup_sign_privdata;
    }

    /* verify data sizes. */
    if (16 != uuid_size)
    {
        retval = AGENTD_ERROR_CONFIG_INVALID_STREAM;
        goto cleanup_sign_privdata;
    }

    /* set up private key structure. */
    memset(entry, 0, sizeof(config_private_key_t));
    entry->hdr.dispose = &private_key_dispose;
    memcpy(entry->id, uuid_data, uuid_size);
    /* change to true when ownership transfers to caller. */
    entry->found = false;

    /* initialize buffer for the encryption public key. */
    retval =
        vccrypt_buffer_init(&entry->enc_pubkey, alloc_opts, enc_pubsize);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto clear_entry;
    }

    /* copy public encryption key data to this buffer. */
    memcpy(entry->enc_pubkey.data, enc_pubdata, enc_pubsize);

    /* initialize buffer for the encryption private key. */
    retval =
        vccrypt_buffer_init(&entry->enc_privkey, alloc_opts, enc_privsize);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubkey;
    }

    /* copy private encryption key data to this buffer. */
    memcpy(entry->enc_privkey.data, enc_privdata, enc_privsize);

    /* initialize buffer for the signature public key. */
    retval =
        vccrypt_buffer_init(&entry->sign_pubkey, alloc_opts, sign_pubsize);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_privkey;
    }

    /* copy public signature key data to this buffer. */
    memcpy(entry->sign_pubkey.data, sign_pubdata, sign_pubsize);

    /* initialize buffer for the signature private key. */
    retval =
        vccrypt_buffer_init(&entry->sign_privkey, alloc_opts, sign_privsize);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_sign_pubkey;
    }

    /* copy private signature key data to this buffer. */
    memcpy(entry->sign_privkey.data, sign_privdata, sign_privsize);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    /* transfer ownership to caller. */
    entry->found = true;
    /* entry is owned by caller on success, so skip its cleanup. */
    goto cleanup_sign_privdata;

cleanup_sign_pubkey:
    dispose((disposable_t*)&entry->sign_pubkey);

cleanup_enc_privkey:
    dispose((disposable_t*)&entry->enc_privkey);

cleanup_enc_pubkey:
    dispose((disposable_t*)&entry->enc_pubkey);

clear_entry:
    memset(entry, 0, sizeof(config_private_key_t));

cleanup_sign_privdata:
    memset(sign_privdata, 0, sign_privsize);
    free(sign_privdata);

cleanup_sign_pubdata:
    memset(sign_pubdata, 0, sign_pubsize);
    free(sign_pubdata);

cleanup_enc_privdata:
    memset(enc_privdata, 0, enc_privsize);
    free(enc_privdata);

cleanup_enc_pubdata:
    memset(enc_pubdata, 0, enc_pubsize);
    free(enc_pubdata);

cleanup_uuid_data:
    memset(uuid_data, 0, uuid_size);
    free(uuid_data);

done:
    return retval;
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
