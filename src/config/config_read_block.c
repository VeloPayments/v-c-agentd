/**
 * \file config/config_read_block.c
 *
 * \brief Read a config structure from the given stream.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

/* forward decls */
static int config_read_logdir(int s, agent_config_t* conf);
static int config_read_loglevel(int s, agent_config_t* conf);
static int config_read_block_max_milliseconds(int s, agent_config_t* conf);
static int config_read_block_max_transactions(int s, agent_config_t* conf);
static int config_read_secret(int s, agent_config_t* conf);
static int config_read_rootblock(int s, agent_config_t* conf);
static int config_read_datastore(int s, agent_config_t* conf);
static int config_read_chroot(int s, agent_config_t* conf);
static int config_read_usergroup(int s, agent_config_t* conf);
static int config_read_listen_addr(int s, agent_config_t* conf);
static int config_read_private_key(int s, agent_config_t* conf);
static int config_read_endorser_key(int s, agent_config_t* conf);
static int config_read_public_key(int s, agent_config_t* conf);
void private_key_dispose(void*);
void endorser_key_dispose(void*);
void public_key_dispose(void*);

/**
 * \brief Initialize and read an agent config structure from a blocking stream.
 *
 * On success, a config structure is initialized with data from the blocking
 * stream.  This is owned by the caller and must be disposed by calling \ref
 * dispose() when no longer needed.
 *
 * \param s             The socket descriptor to read.
 * \param conf          The config structure to read.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 *      - AGENTD_ERROR_CONFIG_INET_PTON_FAILURE if converting an address to a
 *        network address failed.
 */
int config_read_block(int s, agent_config_t* conf)
{
    int retval = 0;
    uint8_t type;

    /* initialize this config structure. */
    memset(conf, 0, sizeof(agent_config_t));
    conf->hdr.dispose = &config_dispose;

    /* get the BOM to start this stream. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_uint8_block(s, &type))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* verify that we have the beginning of this stream. */
    if (CONFIG_STREAM_TYPE_BOM != type)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* read all config data. */
    while (AGENTD_STATUS_SUCCESS == ipc_read_uint8_block(s, &type))
    {
        /* handle different config fields. */
        switch (type)
        {
            /* end of stream.  Success. */
            case CONFIG_STREAM_TYPE_EOM:
                return AGENTD_STATUS_SUCCESS;

            /* logdir */
            case CONFIG_STREAM_TYPE_LOGDIR:
                /* attempt to read the logdir from the stream. */
                retval = config_read_logdir(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* loglevel */
            case CONFIG_STREAM_TYPE_LOGLEVEL:
                /* attempt to read the loglevel from the stream. */
                retval = config_read_loglevel(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* secret */
            case CONFIG_STREAM_TYPE_SECRET:
                /* attempt to read the secret from the stream. */
                retval = config_read_secret(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* rootblock */
            case CONFIG_STREAM_TYPE_ROOTBLOCK:
                /* attempt to read the rootblock from the stream. */
                retval = config_read_rootblock(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* datastore */
            case CONFIG_STREAM_TYPE_DATASTORE:
                /* attempt to read the datastore from the stream. */
                retval = config_read_datastore(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* listen address */
            case CONFIG_STREAM_TYPE_LISTEN_ADDR:
                /* attempt to read a listen address. */
                retval = config_read_listen_addr(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* chroot */
            case CONFIG_STREAM_TYPE_CHROOT:
                /* attempt to read the chroot from the stream. */
                retval = config_read_chroot(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* usergroup */
            case CONFIG_STREAM_TYPE_USERGROUP:
                /* attempt to read the usergroup from the stream. */
                retval = config_read_usergroup(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* block max milliseconds */
            case CONFIG_STREAM_TYPE_BLOCK_MAX_MILLISECONDS:
                /* attempt to read the block max milliseconds from stream. */
                retval = config_read_block_max_milliseconds(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* block max transactions */
            case CONFIG_STREAM_TYPE_BLOCK_MAX_TRANSACTIONS:
                /* attempt to read the block max txns from the stream. */
                retval = config_read_block_max_transactions(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* private key */
            case CONFIG_STREAM_TYPE_PRIVATE_KEY:
                /* attempt to read the private key from the stream. */
                retval = config_read_private_key(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* endorser key */
            case CONFIG_STREAM_TYPE_ENDORSER_KEY:
                /* attempt to read the endorser key from the stream. */
                retval = config_read_endorser_key(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* public key */
            case CONFIG_STREAM_TYPE_PUBLIC_KEY:
                /* attempt to read the public key from the stream. */
                retval = config_read_public_key(s, conf);
                if (AGENTD_STATUS_SUCCESS != retval)
                    return retval;
                break;

            /* unknown data */
            default:
                /* return error. */
                return AGENTD_ERROR_CONFIG_INVALID_STREAM;
        }
    }

    /* if we make it here, something has gone wrong. */
    return AGENTD_ERROR_CONFIG_INVALID_STREAM;
}

/**
 * \brief Read the logdir from the config stream.
 *
 * \param s             The socket from which the logdir is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_logdir(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->logdir)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the logdir. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&conf->logdir))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the loglevel from the config stream.
 *
 * \param s             The socket from which the loglevel is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_loglevel(int s, agent_config_t* conf)
{
    /* it's an error to set the loglevel more than once. */
    if (conf->loglevel_set)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the loglevel. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_int64_block(s, &conf->loglevel))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* loglevel must be between 0 and 9. */
    if (conf->loglevel < 0 || conf->loglevel > 9)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* loglevel has been set. */
    conf->loglevel_set = true;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the block max milliseconds from the config stream.
 *
 * \param s             The socket from which this value is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_block_max_milliseconds(int s, agent_config_t* conf)
{
    /* it's an error to set the block max milliseconds more than once. */
    if (conf->block_max_milliseconds_set)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the value. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_int64_block(s, &conf->block_max_milliseconds))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* block max milliseconds must be between 0 / BLOCK_MILLISECONDS_MAXIMUM. */
    if (conf->block_max_milliseconds < 0 || conf->block_max_milliseconds > BLOCK_MILLISECONDS_MAXIMUM)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* loglevel has been set. */
    conf->block_max_milliseconds_set = true;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the block max transactions from the config stream.
 *
 * \param s             The socket from which this value is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_block_max_transactions(int s, agent_config_t* conf)
{
    /* it's an error to set the block max transactions more than once. */
    if (conf->block_max_transactions_set)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the value. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_int64_block(s, &conf->block_max_transactions))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* block max txns must be between 0 and BLOCK_TRANSACTIONS_MAXIMUM. */
    if (conf->block_max_transactions < 0 || conf->block_max_transactions > BLOCK_TRANSACTIONS_MAXIMUM)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* block_max_transactions has been set. */
    conf->block_max_transactions_set = true;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the secret from the config stream.
 *
 * \param s             The socket from which the secret is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_secret(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->secret)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the secret. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&conf->secret))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the rootblock from the config stream.
 *
 * \param s             The socket from which the rootblock is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_rootblock(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->rootblock)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the rootblock. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&conf->rootblock))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the datastore from the config stream.
 *
 * \param s             The socket from which the datastore is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_datastore(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->datastore)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the datastore. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&conf->datastore))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the chroot from the config stream.
 *
 * \param s             The socket from which the chroot is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_chroot(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->chroot)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* attempt to read the chroot. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&conf->chroot))
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the user/group from the config stream.
 *
 * \param s             The socket from which the user/group is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_usergroup(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->usergroup)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* allocate a usergroup structure. */
    config_user_group_t* usergroup =
        (config_user_group_t*)malloc(sizeof(config_user_group_t));
    if (NULL == usergroup)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* clear this structure. */
    memset(usergroup, 0, sizeof(config_user_group_t));

    /* attempt to read the user. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&usergroup->user))
    {
        free(usergroup);
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
    }

    /* attempt to read the group. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&usergroup->group))
    {
        free((char*)usergroup->user);
        free(usergroup);
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
    }

    /* set the usergroup. */
    conf->usergroup = usergroup;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read a listen address from the config stream.
 *
 * \param s             The socket from which the listen address is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 *      - AGENTD_ERROR_CONFIG_INET_PTON_FAILURE if converting an address to a
 *        network address failed.
 */
static int config_read_listen_addr(int s, agent_config_t* conf)
{
    int retval = 1;
    char* paddr = NULL;
    config_listen_address_t* ptr = NULL;

    /* allocate a listen_addr structure. */
    ptr = (config_listen_address_t*)malloc(sizeof(config_listen_address_t));
    if (NULL == ptr)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* clear this structure. */
    memset(ptr, 0, sizeof(config_listen_address_t));

    /* attempt to read the address. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_string_block(s, &paddr))
    {
        retval = AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
        goto cleanup;
    }

    /* allocate an addr structure. */
    ptr->addr = (struct in_addr*)malloc(sizeof(struct in_addr));
    if (NULL == ptr->addr)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup;
    }

    /* convert to a network address. */
    if (1 != inet_pton(AF_INET, paddr, ptr->addr))
    {
        retval = AGENTD_ERROR_CONFIG_INET_PTON_FAILURE;
        goto cleanup;
    }

    /* clean up. */
    free(paddr);
    paddr = NULL;

    /* attempt to write the listen port. */
    uint64_t port = 0U;
    if (AGENTD_STATUS_SUCCESS != ipc_read_uint64_block(s, &port))
    {
        retval = AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
        goto cleanup;
    }

    /* set port. */
    ptr->port = port;

    /* append the address. */
    ptr->hdr.next = (config_list_node_t*)conf->listen_head;
    conf->listen_head = ptr;
    ptr = NULL;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

cleanup:

    /* clean up this failed node. */
    if (NULL != ptr)
    {
        /* clean up addr */
        if (NULL != ptr->addr)
            free(ptr->addr);

        free(ptr);
    }

    /* clean up presentation address. */
    if (NULL != paddr)
        free(paddr);

done:
    return retval;
}

/**
 * \brief Read the private key from the config stream.
 *
 * \param s             The socket from which the private key is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_private_key(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->private_key)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* allocate a private key structure. */
    config_private_key_entry_t* private_key =
        (config_private_key_entry_t*)malloc(sizeof(config_private_key_entry_t));
    if (NULL == private_key)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* clear this structure. */
    memset(private_key, 0, sizeof(config_private_key_entry_t));

    /* set the dispose method. */
    private_key->hdr.dispose = &private_key_dispose;

    /* attempt to read the private key filename. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&private_key->filename))
    {
        free(private_key);
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
    }

    /* set the private key. */
    conf->private_key = private_key;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read the endorser key from the config stream.
 *
 * \param s             The socket from which the endorser key is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_endorser_key(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->endorser_key)
        return AGENTD_ERROR_CONFIG_INVALID_STREAM;

    /* allocate a endorser key structure. */
    config_endorser_key_entry_t* endorser_key =
        (config_endorser_key_entry_t*)malloc(
            sizeof(config_endorser_key_entry_t));
    if (NULL == endorser_key)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* clear this structure. */
    memset(endorser_key, 0, sizeof(config_endorser_key_entry_t));

    /* set the dispose method. */
    endorser_key->hdr.dispose = &endorser_key_dispose;

    /* attempt to read the endorser key filename. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&endorser_key->filename))
    {
        free(endorser_key);
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
    }

    /* set the endorser key. */
    conf->endorser_key = endorser_key;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Read a public key from the config stream.
 *
 * \param s             The socket from which the public key is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 */
static int config_read_public_key(int s, agent_config_t* conf)
{
    /* allocate a public key structure. */
    config_public_key_entry_t* public_key =
        (config_public_key_entry_t*)malloc(sizeof(config_public_key_entry_t));
    if (NULL == public_key)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* clear this structure. */
    memset(public_key, 0, sizeof(config_public_key_entry_t));

    /* set the dispose method. */
    public_key->hdr.hdr.dispose = &public_key_dispose;

    /* attempt to read the public key filename. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_read_string_block(s, (char**)&public_key->filename))
    {
        free(public_key);
        return AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
    }

    /* save the previous public key head. */
    public_key->hdr.next =
        (config_disposable_list_node_t*)conf->public_key_head;

    /* set the public key. */
    conf->public_key_head = public_key;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
