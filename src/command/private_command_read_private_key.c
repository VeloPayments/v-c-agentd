/**
 * \file command/private_command_read_private_key.c
 *
 * \brief Read public entity certificates.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vccert/fields.h>
#include <vccert/parser.h>
#include <vccrypt/suite.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

/* forward decls */
static void read_private_key(
    int controlfd, vccert_parser_options_t* parser_opts);

/**
 * \brief Read private key.
 */
void private_command_read_private_key(bootstrap_config_t* UNUSED(bconf))
{
    int retval;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    vccert_parser_options_t parser_opts;

    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* create a malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    retval =
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_alloc_opts;
    }

    /* initialize the parser options. */
    retval =
        vccert_parser_options_simple_init(&parser_opts, &alloc_opts, &suite);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_suite;
    }

    /* read the private key. */
    read_private_key(AGENTD_FD_READER_CONTROL, &parser_opts);

    /* success. */
    goto cleanup_parser_opts;

cleanup_parser_opts:
    dispose((disposable_t*)&parser_opts);

cleanup_suite:
    dispose((disposable_t*)&suite);

cleanup_alloc_opts:
    dispose((disposable_t*)&alloc_opts);
}

/**
 * \brief Read the private key file send data back over the control socket.
 *
 * \param controlfd         Descriptor for the control socket.
 * \param parser_opts       The parser options to use to parse a public entity
 *                          certificate.
 */
static void read_private_key(
    int controlfd, vccert_parser_options_t* parser_opts)
{
    int retval, fd;
    struct stat st;
    vccrypt_buffer_t cert_buffer;
    vccert_parser_context_t parser;

    /* attempt to read a filename from the control stream. */
    char* filename;
    retval = ipc_read_string_block(controlfd, &filename);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* attempt to open the filename for read. */
    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        retval = AGENTD_ERROR_READER_FILE_OPEN;
        goto cleanup_filename;
    }

    /* attempt to stat the file. */
    retval = fstat(fd, &st);
    if (0 != retval)
    {
        retval = AGENTD_ERROR_READER_FILE_STAT;
        goto close_fd;
    }

    /* initialize the certificate buffer. */
    retval =
        vccrypt_buffer_init(&cert_buffer, parser_opts->alloc_opts, st.st_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto close_fd;
    }

    /* read the certificate from the file. */
    ssize_t read_bytes = read(fd, cert_buffer.data, cert_buffer.size);
    if (read_bytes < (ssize_t)cert_buffer.size)
    {
        retval = AGENTD_ERROR_READER_FILE_READ;
        goto cleanup_cert_buffer;
    }

    /* create a parser instance, backed by this buffer. */
    retval =
        vccert_parser_init(
            parser_opts, &parser, cert_buffer.data, cert_buffer.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_cert_buffer;
    }

    /* read the artifact uuid. */
    const uint8_t* artifact_id;
    size_t artifact_id_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_ARTIFACT_ID, &artifact_id,
            &artifact_id_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* read the public encryption key. */
    const uint8_t* enc_pubkey;
    size_t enc_pubkey_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PUBLIC_ENCRYPTION_KEY, &enc_pubkey,
            &enc_pubkey_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* read the private encryption key. */
    const uint8_t* enc_privkey;
    size_t enc_privkey_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PRIVATE_ENCRYPTION_KEY, &enc_privkey,
            &enc_privkey_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* read the public signing key. */
    const uint8_t* sign_pubkey;
    size_t sign_pubkey_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PUBLIC_SIGNING_KEY, &sign_pubkey,
            &sign_pubkey_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* read the private signing key. */
    const uint8_t* sign_privkey;
    size_t sign_privkey_size;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PRIVATE_SIGNING_KEY, &sign_privkey,
            &sign_privkey_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the BOM value. */
    retval = ipc_write_uint8_block(controlfd, CONFIG_STREAM_TYPE_BOM);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }
 
    /* write the entity id. */
    retval = ipc_write_data_block(controlfd, artifact_id, artifact_id_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the public encryption key. */
    retval = ipc_write_data_block(controlfd, enc_pubkey, enc_pubkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the private encryption key. */
    retval = ipc_write_data_block(controlfd, enc_privkey, enc_privkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the public signing key. */
    retval = ipc_write_data_block(controlfd, sign_pubkey, sign_pubkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the private signing key. */
    retval = ipc_write_data_block(controlfd, sign_privkey, sign_privkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the EOM value. */
    retval = ipc_write_uint8_block(controlfd, CONFIG_STREAM_TYPE_EOM);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* success. */
    goto cleanup_parser;

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_cert_buffer:
    dispose((disposable_t*)&cert_buffer);

close_fd:
    close(fd);

cleanup_filename:
    free(filename);

done:
    return;
}
