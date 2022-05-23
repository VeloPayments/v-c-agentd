/**
 * \file readers/config_read_public_entities_proc.c
 *
 * \brief Spawn a process as the blockchain user/group to read public entity
 * files.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vpr/allocator/malloc_allocator.h>

/* forward decls */
static int config_public_file_send_endorser_flag(
    int clientsock, bool is_endorser);
static int config_public_file_send(
    int clientsock, const char* filename);
static int config_entity_read(
    int clientsock, config_public_entity_node_t** entry);
static void public_entity_dispose(void* disp);

/**
 * \brief Spawn a process to read the public entities, populating the provided
 * public entities structure.
 *
 * On success, a public entities structure is initialized with data from the
 * public entity reader process. This is owned by the caller and must be
 * disposed by calling \ref dispose() when no longer needed.
 *
 * \param bconf         The bootstrap configuration used to spawn the process.
 * \param conf          The config structure used to spawn the process.
 * \param endorser      The \ref config_public_entity_node_t of the endorser.
 * \param entities      The \ref config_public_entity_node_t list to populate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_READER_PROC_RUNSECURE_ROOT_USER_REQUIRED if spawning this
 *        process failed because the user is not root and runsecure is true.
 *      - AGENTD_ERROR_READER_IPC_SOCKETPAIR_FAILURE if creating a socketpair
 *        for the dataservice process failed.
 *      - AGENTD_ERROR_READER_FORK_FAILURE if forking the private process
 *        failed.
 *      - AGENTD_ERROR_READER_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there was a
 *        failure looking up the configured user and group for the process.
 *      - AGENTD_ERROR_READER_PRIVSEP_CHROOT_FAILURE if chrooting failed.
 *      - AGENTD_ERROR_READER_PRIVSEP_DROP_PRIVILEGES_FAILURE if dropping
 *        privileges failed.
 *      - AGENTD_ERROR_READER_PRIVSEP_SETFDS_FAILURE if setting file descriptors
 *        failed.
 *      - AGENTD_ERROR_READER_PRIVSEP_EXEC_PRIVATE_FAILURE if executing the
 *        private command failed.
 *      - AGENTD_ERROR_READER_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the process
 *        survived execution (weird!).      
 *      - AGENTD_ERROR_READER_IPC_READ_DATA_FAILURE if reading data from the
 *        config stream failed.
 *      - AGENTD_ERROR_READER_PROC_EXIT_FAILURE if the config proc did not
 *        properly exit.
 */
int config_read_public_entities_proc(
    const struct bootstrap_config* bconf, agent_config_t* conf,
    config_public_entity_node_t** endorser,
    config_public_entity_node_t** entities)
{
    int retval = 1;
    int clientsock = -1, serversock = -1;
    int procid = 0;
    uid_t uid;
    gid_t gid;

    /* verify that this process is running as root. */
    if (0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
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
        retval = privsep_exec_private(bconf, "read_public_entities");
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

        /* start with no entries. */
        *entities = NULL;

        /* send the endorser public key entry if set. */
        *endorser = NULL;
        if (NULL != conf->endorser_key)
        {
            /* send a flag indicating that the first entity is the endorser. */
            if (0 != config_public_file_send_endorser_flag(clientsock, true))
            {
                retval = AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE;
                goto cleanup_entities;
            }

            /* send the file to open / read. */
            if (0 !=
                config_public_file_send(clientsock,
                    conf->endorser_key->filename))
            {
                retval = AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE;
                goto cleanup_entities;
            }

            /* read back the response. */
            config_public_entity_node_t* entry = NULL;
            if (0 != config_entity_read(clientsock, &entry))
            {
                retval = AGENTD_ERROR_READER_IPC_READ_DATA_FAILURE;
                goto cleanup_entities;
            }

            /* save as the endorser entity. */
            *endorser = entry;
        }
        else
        {
            /* The first entity is NOT the endorser. */
            if (0 != config_public_file_send_endorser_flag(clientsock, false))
            {
                retval = AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE;
                goto cleanup_entities;
            }
        }

        /* for each file to read... */
        config_public_key_entry_t* tmp = conf->public_key_head;
        while (NULL != tmp)
        {
            /* send the file to open / read. */
            if (0 != config_public_file_send(clientsock, tmp->filename))
            {
                retval = AGENTD_ERROR_READER_IPC_WRITE_DATA_FAILURE;
                goto cleanup_entities;
            }

            /* read back the response. */
            config_public_entity_node_t* entry = NULL;
            if (0 != config_entity_read(clientsock, &entry))
            {
                retval = AGENTD_ERROR_READER_IPC_READ_DATA_FAILURE;
                goto cleanup_entities;
            }

            /* add to entities. */
            entry->hdr.next = (config_disposable_list_node_t*)*entities;
            *entities = entry;

            /* skip to the next entry. */
            tmp = (config_public_key_entry_t*)tmp->hdr.next;
        }

        /* we're done with the reader proc, so send EOM and close the socket. */
        ipc_write_uint8_block(clientsock, CONFIG_STREAM_TYPE_EOM);
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

        /* make sure to clean up the entities if we fail. */
        if (0 != retval)
            goto cleanup_entities;
        else
            goto done;
    }

cleanup_entities:
    if (NULL != *endorser)
    {
        dispose((disposable_t*)*endorser);
        free(*endorser);
        *endorser = NULL;
    }

    while (NULL != *entities)
    {
        config_public_entity_node_t* tmp =
            (config_public_entity_node_t*)(*entities)->hdr.next;
        dispose((disposable_t*)*entities);
        free(*entities);
        *entities = tmp;
    }

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
 * \brief Send a flag indicating whether the next entity is the endorser.
 *
 * \param clientsock        Socket connection to the reader proc.
 * \param is_endorser       A flag to indicate whether the next entity is the
 *                          endorser.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int config_public_file_send_endorser_flag(
    int clientsock, bool is_endorser)
{
    uint8_t endorser_flag;

    if (is_endorser)
    {
        endorser_flag = 1;
    }
    else
    {
        endorser_flag = 0;
    }

    return ipc_write_uint8_block(clientsock, endorser_flag);
}

/**
 * \brief Send a public entity file to the reader proc.
 *
 * \param clientsock        Socket connection to the reader proc.
 * \param filename          The public key entry filename to send.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int config_public_file_send(
    int clientsock, const char* filename)
{
    return ipc_write_string_block(clientsock, filename);
}

/**
 * \brief Read a public entity from the reader proc.
 *
 * \param clientsock        The reader process socket.
 * \param entry             The entry to read. On success, it is allocated and
 *                          populated. The caller is responsible for disposing
 *                          and freeing it.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int config_entity_read(
    int clientsock, config_public_entity_node_t** entry)
{
    int retval;
    uint8_t type;
    allocator_options_t alloc_opts;

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

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
    uint8_t* enc_data;
    uint32_t enc_size;
    retval = ipc_read_data_block(clientsock, (void**)&enc_data, &enc_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuid_data;
    }

    /* read the signing public key. */
    uint8_t* sign_data;
    uint32_t sign_size;
    retval = ipc_read_data_block(clientsock, (void**)&sign_data, &sign_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_data;
    }

    /* finally, read the end of message marker. */
    retval = ipc_read_uint8_block(clientsock, &type);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_sign_data;
    }

    /* verify that this is the end of a message. */
    if (CONFIG_STREAM_TYPE_EOM != type)
    {
        retval = AGENTD_ERROR_READER_INVALID_STREAM;
        goto cleanup_sign_data;
    }

    /* verify data sizes. */
    if (16 != uuid_size)
    {
        retval = AGENTD_ERROR_CONFIG_INVALID_STREAM;
        goto cleanup_sign_data;
    }

    /* allocate memory for the entry. */
    *entry = (config_public_entity_node_t*)
        malloc(sizeof(config_public_entity_node_t));
    if (NULL == *entry)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_sign_data;
    }

    /* clear structure. */
    memset(*entry, 0, sizeof(config_public_entity_node_t));

    /* set disposer and uuid. */
    (*entry)->hdr.hdr.dispose = &public_entity_dispose;
    memcpy((*entry)->id, uuid_data, uuid_size);

    /* initialize buffer for the encryption public key. */
    retval = vccrypt_buffer_init(&(*entry)->enc_pubkey, &alloc_opts, enc_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto free_entry;
    }

    /* copy public encryption key data to this buffer. */
    memcpy((*entry)->enc_pubkey.data, enc_data, enc_size);

    /* initialize buffer for the signing public key. */
    retval =
        vccrypt_buffer_init(&(*entry)->sign_pubkey, &alloc_opts, sign_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_enc_pubkey;
    }

    /* copy public signing key data to this buffer. */
    memcpy((*entry)->sign_pubkey.data, sign_data, sign_size);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    /* entry is owned by caller on success, so skip its cleanup. */
    goto cleanup_sign_data;

cleanup_enc_pubkey:
    dispose((disposable_t*)&(*entry)->enc_pubkey);

free_entry:
    memset(*entry, 0, sizeof(config_public_entity_node_t));
    free(*entry);
    *entry = NULL;

cleanup_sign_data:
    memset(sign_data, 0, sign_size);
    free(sign_data);

cleanup_enc_data:
    memset(enc_data, 0, enc_size);
    free(enc_data);

cleanup_uuid_data:
    memset(uuid_data, 0, uuid_size);
    free(uuid_data);

done:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}

/**
 * \brief Dispose of a public entity node.
 *
 * \param disp          The public entity node to dispose.
 */
static void public_entity_dispose(void* disp)
{
    config_public_entity_node_t* node = (config_public_entity_node_t*)disp;

    /* dispose of the public keys. */
    dispose((disposable_t*)&node->enc_pubkey);
    dispose((disposable_t*)&node->sign_pubkey);

    /* clear out capabilities. */
    while (NULL != node->cap_head)
    {
        config_public_entity_capability_node_t* tmp =
            (config_public_entity_capability_node_t*)node->cap_head->hdr.next;
        dispose((disposable_t*)node->cap_head);
        free(node->cap_head);
        node->cap_head = tmp;
    }

    /* clear out the structure. */
    memset(node, 0, sizeof(config_public_entity_node_t));
}
