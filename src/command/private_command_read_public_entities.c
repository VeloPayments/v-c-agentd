/**
 * \file command/private_command_read_public_entities.c
 *
 * \brief Read public entity certificates.
 *
 * \copyright 2020-2022 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <fcntl.h>
#include <rcpr/allocator.h>
#include <rcpr/resource.h>
#include <rcpr/resource/protected.h>
#include <rcpr/uuid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vccert/fields.h>
#include <vccert/parser.h>
#include <vccrypt/compare.h>
#include <vccrypt/suite.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

RCPR_IMPORT_allocator_as(rcpr);
RCPR_IMPORT_resource;
RCPR_IMPORT_uuid;

/* forward decls */
typedef struct parser_callback_context parser_callback_context;
static bool dummy_txn_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    const uint8_t* txn_id, vccrypt_buffer_t* output_buffer, bool* trusted);
static int32_t dummy_artifact_state_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    vccrypt_buffer_t* txn_id);
static int basic_contract_resolver(
    void* options, void* parser, const uint8_t* type_id,
    const uint8_t* artifact_id,
    vccert_contract_closure_t* closure);
static void basic_contract_disposer(void* disp);
static bool always_pass_contract(vccert_parser_context_t* parser, void* ctx);
static bool endorser_key_resolver(
    void* options, void* parser, uint64_t height, const uint8_t* entity_id,
    vccrypt_buffer_t* pubenckey_buffer, vccrypt_buffer_t* pubsignkey_buffer);
static void read_public_entities(
    int controlfd, vccert_parser_options_t* parser_opts,
    parser_callback_context* ctx);
static int read_public_entity(
    int controlfd, vccert_parser_options_t* parser_opts, const char* filename,
    parser_callback_context* ctx, bool is_endorser);
static status parser_callback_context_create(
    parser_callback_context** ctx, rcpr_allocator* alloc,
    vccrypt_suite_options_t* suite);
static status parser_callback_context_resource_release(resource* r);
static status entity_get_capabilities_count(
    uint64_t* count, vccert_parser_context_t* parser);

struct parser_callback_context
{
    resource hdr;
    rcpr_allocator* alloc;
    bool endorser_set;
    rcpr_uuid endorser_id;
    vccrypt_buffer_t endorser_cipher_key;
    vccrypt_buffer_t endorser_signing_key;
};

/**
 * \brief Read public entities.
 */
void private_command_read_public_entities(bootstrap_config_t* UNUSED(bconf))
{
    int retval, release_retval;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    vccert_parser_options_t parser_opts;
    rcpr_allocator* alloc;
    parser_callback_context* ctx;

    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* create a malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create an RCPR malloc allocator. */
    retval = rcpr_malloc_allocator_create(&alloc);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_alloc_opts;
    }

    /* initialize the crypto suite. */
    retval =
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_alloc;
    }

    /* create the parser context structure. */
    retval = parser_callback_context_create(&ctx, alloc, &suite);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_suite;
    }

    /* initialize the parser options. */
    retval =
        vccert_parser_options_init(
            &parser_opts, &alloc_opts, &suite, &dummy_txn_resolver,
            &dummy_artifact_state_resolver, &basic_contract_resolver,
            &endorser_key_resolver, ctx);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_ctx;
    }

    /* read the public entities. */
    read_public_entities(AGENTD_FD_READER_CONTROL, &parser_opts, ctx);

    /* success. */
    goto cleanup_parser_opts;

cleanup_parser_opts:
    dispose((disposable_t*)&parser_opts);

cleanup_ctx:
    release_retval = resource_release(&ctx->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_suite:
    dispose((disposable_t*)&suite);

cleanup_alloc:
    release_retval = resource_release(rcpr_allocator_resource_handle(alloc));
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

cleanup_alloc_opts:
    dispose((disposable_t*)&alloc_opts);
}

/**
 * \brief Read public entity files and send data back over the control socket.
 *
 * \param controlfd         Descriptor for the control socket.
 * \param parser_opts       The parser options to use to parse a public entity
 *                          certificate.
 * \param ctx               The parser callback context.
 */
static void read_public_entities(
    int controlfd, vccert_parser_options_t* parser_opts,
    parser_callback_context* ctx)
{
    int retval;
    bool stream_okay = true;
    uint8_t is_endorser = 0;

    /* read a flag indicating whether the first entity is the endorser. */
    retval = ipc_read_uint8_block(controlfd, &is_endorser);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        stream_okay = false;
    }

    while (stream_okay)
    {
        /* attempt to read a filename from the control stream. */
        char* filename;
        retval = ipc_read_string_block(controlfd, &filename);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            stream_okay = false;
            continue;
        }

        /* process this file. */
        retval =
            read_public_entity(
                controlfd, parser_opts, filename, ctx, is_endorser);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            stream_okay = false;
        }

        /* clean up the filename. */
        free(filename);

        /* reset the endorser flag. */
        is_endorser = 0;
    }
}

/**
 * \brief Read a public entity file and send data back over the control socket.
 *
 * \param controlfd         Descriptor for the control socket.
 * \param parser_opts       The parser options to use to parse a public entity
 *                          certificate.
 * \param filename          The name of the file to read.
 * \param ctx               The parser callback context.
 * \param is_endorser       Set to true if this is the endorser certificate
 *                          file.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int read_public_entity(
    int controlfd, vccert_parser_options_t* parser_opts, const char* filename,
    parser_callback_context* ctx, bool is_endorser)
{
    int retval, fd;
    struct stat st;
    vccrypt_buffer_t cert_buffer;
    vccert_parser_context_t parser;

    /* attempt to open the filename for read. */
    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        retval = AGENTD_ERROR_READER_FILE_OPEN;
        goto done;
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

    /* if the endorser is set, run attestation on the certificate. */
    if (ctx->endorser_set)
    {
        retval =
            vccert_parser_attest(&parser, 0, false);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }
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

    /* verify the artifact id size. */
    if (artifact_id_size != 16)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
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

    /* verify the encryption pubkey size. */
    if (
        enc_pubkey_size
            != parser_opts->crypto_suite->key_cipher_opts.public_key_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
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

    /* verify the signing pubkey size. */
    if (sign_pubkey_size
            != parser_opts->crypto_suite->sign_opts.public_key_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_parser;
    }

    /* if this is the endorser, copy the keys. */
    if (is_endorser)
    {
        /* copy the endorser cipher key. */
        retval =
            vccrypt_buffer_read_data(
                &ctx->endorser_cipher_key, enc_pubkey, enc_pubkey_size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }

        /* copy the endorser signing key. */
        retval =
            vccrypt_buffer_read_data(
                &ctx->endorser_signing_key, sign_pubkey, sign_pubkey_size);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }

        /* the endorser is now valid. */
        ctx->endorser_set = true;
        memcpy(ctx->endorser_id.data, artifact_id, artifact_id_size);
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

    /* write the encryption key. */
    retval = ipc_write_data_block(controlfd, enc_pubkey, enc_pubkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* write the signing key. */
    retval = ipc_write_data_block(controlfd, sign_pubkey, sign_pubkey_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* if the endorser is set, retrieve and send the caps. */
    if (ctx->endorser_set && !is_endorser)
    {
        /* get the capabilities count. */
        uint64_t count = 0;
        retval = entity_get_capabilities_count(&count, &parser);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }

        /* emit the number of capabilities. */
        retval = ipc_write_uint64_block(controlfd, count);
        if (STATUS_SUCCESS != retval)
        {
            goto cleanup_parser;
        }

        /* get the first capability. */
        const uint8_t* endorsebuf;
        size_t endorsebuf_size;
        retval =
            vccert_parser_find_short(
                &parser, VCCERT_FIELD_TYPE_VELO_ENDORSEMENT,
                &endorsebuf, &endorsebuf_size);
        if (STATUS_SUCCESS != retval)
        {
            /* were we expecting to find more fields? */
            if (count)
            {
                goto cleanup_parser;
            }
            else
            {
                retval = STATUS_SUCCESS;
                goto message_write_eom;
            }
        }

        /* if the field size is incorrect, it's an error. */
        if (endorsebuf_size < 48)
        {
            retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
            goto cleanup_parser;
        }

        /* iterate through all of the caps. */
        while (count--)
        {
            /* write the BOM value. */
            retval = ipc_write_uint8_block(controlfd, CONFIG_STREAM_TYPE_BOM);
            if (AGENTD_STATUS_SUCCESS != retval)
            {
                goto cleanup_parser;
            }
 
            /* emit the subject. */
            retval = ipc_write_data_block(controlfd, endorsebuf, 16);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_parser;
            }

            /* emit the verb. */
            retval = ipc_write_data_block(controlfd, endorsebuf + 16, 16);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_parser;
            }

            /* emit the object. */
            retval = ipc_write_data_block(controlfd, endorsebuf + 32, 16);
            if (STATUS_SUCCESS != retval)
            {
                goto cleanup_parser;
            }

            /* write the EOM value. */
            retval = ipc_write_uint8_block(controlfd, CONFIG_STREAM_TYPE_EOM);
            if (AGENTD_STATUS_SUCCESS != retval)
            {
                goto cleanup_parser;
            }

            /* get the next field. */
            retval =
                vccert_parser_find_next(
                    &parser, &endorsebuf, &endorsebuf_size);
            if (STATUS_SUCCESS != retval)
            {
                /* were we expecting to find more fields? */
                if (count)
                {
                    goto cleanup_parser;
                }
                else
                {
                    retval = STATUS_SUCCESS;
                    goto message_write_eom;
                }
            }

            /* if the field size is incorrect, it's an error. */
            if (endorsebuf_size < 48)
            {
                retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
                goto cleanup_parser;
            }
        }
    }

message_write_eom:
    /* write the EOM value. */
    retval = ipc_write_uint8_block(controlfd, CONFIG_STREAM_TYPE_EOM);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_parser;

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_cert_buffer:
    dispose((disposable_t*)&cert_buffer);

close_fd:
    close(fd);

done:
    return retval;
}

/**
 * \brief Dummy transaction resolver.
 */
static bool dummy_txn_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id),
    const uint8_t* UNUSED(txn_id), vccrypt_buffer_t* UNUSED(output_buffer),
    bool* UNUSED(trusted))
{
    return false;
}

/**
 * \brief Dummy artifact state resolver.
 */
static int32_t dummy_artifact_state_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id), vccrypt_buffer_t* UNUSED(txn_id))
{
    return 0;
}

/**
 * \brief Basic contract resolver that does nothing.
 */
static int basic_contract_resolver(
    void* UNUSED(options), void* UNUSED(parser), const uint8_t* UNUSED(type_id),
    const uint8_t* UNUSED(artifact_id),
    vccert_contract_closure_t* closure)
{
    dispose_init(&closure->hdr, &basic_contract_disposer);
    closure->contract_fn = &always_pass_contract;
    closure->context = NULL;
    return STATUS_SUCCESS;
}

/**
 * \brief Basic contract disposer.
 *
 * \param disp The value to be disposed.
 */
static void basic_contract_disposer(void* UNUSED(disp))
{
    /* do nothing. */
}

/**
 * \brief This contract always passes.
 */
static bool always_pass_contract(
    vccert_parser_context_t* UNUSED(parser), void* UNUSED(ctx))
{
    return true;
}

/**
 * \brief Key resolver for the endorser.
 */
static bool endorser_key_resolver(
    void* options, void* UNUSED(parser), uint64_t UNUSED(height),
    const uint8_t* entity_id,
    vccrypt_buffer_t* pubenckey_buffer,
    vccrypt_buffer_t* pubsignkey_buffer)
{
    status retval;
    vccert_parser_options_t* opts = (vccert_parser_options_t*)options;
    parser_callback_context* ctx = (parser_callback_context*)opts->context;

    /* verify that the endorser is set. */
    if (!ctx->endorser_set)
    {
        return false;
    }

    /* verify that the endorser id matches. */
    if (crypto_memcmp(entity_id, ctx->endorser_id.data, 16))
    {
        return false;
    }

    /* verify that the buffer sizes match our buffer sizes. */
    if (pubenckey_buffer->size != ctx->endorser_cipher_key.size
     || pubsignkey_buffer->size != ctx->endorser_signing_key.size)
    {
        return false;
    }

    /* copy the endorser public encryption key. */
    retval = vccrypt_buffer_copy(pubenckey_buffer, &ctx->endorser_cipher_key);
    if (STATUS_SUCCESS != retval)
    {
        return false;
    }

    /* copy the endorser public signing key. */
    retval = vccrypt_buffer_copy(pubsignkey_buffer, &ctx->endorser_signing_key);
    if (STATUS_SUCCESS != retval)
    {
        return false;
    }

    /* success. */
    return true;
}

/**
 * \brief Create a parser callback context to be used by this utility.
 *
 * \param ctx           Pointer to receive the created context.
 * \param alloc         The allocator to use for this operation.
 * \param suite         The cryptographic suite to use for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status parser_callback_context_create(
    parser_callback_context** ctx, rcpr_allocator* alloc,
    vccrypt_suite_options_t* suite)
{
    status retval, release_retval;
    parser_callback_context* tmp;

    /* allocate a buffer for this callback structure. */
    retval = rcpr_allocator_allocate(alloc, (void**)&tmp, sizeof(*tmp));
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* clear memory. */
    memset(tmp, 0, sizeof(*tmp));

    /* set basics. */
    tmp->alloc = alloc;

    /* initialize resource. */
    resource_init(&tmp->hdr, &parser_callback_context_resource_release);

    /* create the encryption public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            suite, &tmp->endorser_cipher_key);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* create the signing public key buffer. */
    retval =
        vccrypt_suite_buffer_init_for_signature_public_key(
            suite, &tmp->endorser_signing_key);
    if (STATUS_SUCCESS != retval)
    {
        goto cleanup_tmp;
    }

    /* currently, the endorser is NOT set. */
    tmp->endorser_set = false;

    /* success. */
    *ctx = tmp;
    retval = STATUS_SUCCESS;
    goto done;

cleanup_tmp:
    release_retval = resource_release(&tmp->hdr);
    if (STATUS_SUCCESS != release_retval)
    {
        retval = release_retval;
    }

done:
    return retval;
}

/**
 * \brief Release a parser callback context resource.
 *
 * \param r             The resource to be released.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status parser_callback_context_resource_release(resource* r)
{
    parser_callback_context* ctx = (parser_callback_context*)r;

    /* cache the allocator. */
    rcpr_allocator* alloc = ctx->alloc;

    /* if the cipher key buffer has been created, dispose it. */
    if (NULL != ctx->endorser_cipher_key.data)
    {
        dispose((disposable_t*)&ctx->endorser_cipher_key);
    }

    /* if the signing key buffer has been created, dispose it. */
    if (NULL != ctx->endorser_signing_key.data)
    {
        dispose((disposable_t*)&ctx->endorser_signing_key);
    }

    /* clear the structure. */
    memset(ctx, 0, sizeof(*ctx));

    /* reclaim the buffer. */
    return
        rcpr_allocator_reclaim(alloc, ctx);
}

/**
 * \brief Get the count of capabilities in the current parser instance.
 *
 * \param count             Pointer to receive this count.
 * \param parser            The parser instance.
 *
 * \returns a status code indicating success or failure.
 *      - STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static status entity_get_capabilities_count(
    uint64_t* count, vccert_parser_context_t* parser)
{
    status retval;

    /* the count starts at 0. */
    *count = 0;

    /* get the first capability. */
    const uint8_t* endorsebuf;
    size_t endorsebuf_size;
    retval =
        vccert_parser_find_short(
            parser, VCCERT_FIELD_TYPE_VELO_ENDORSEMENT, &endorsebuf,
            &endorsebuf_size);
    if (STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* increment count. */
    ++(*count);

    /* loop until we reach the end. */
    for (;;)
    {
        /* get the next field. */
        retval =
            vccert_parser_find_next(parser, &endorsebuf, &endorsebuf_size);
        if (STATUS_SUCCESS != retval)
        {
            goto done;
        }

        /* increment count. */
        ++(*count);
    }

done:
    return STATUS_SUCCESS;
}
