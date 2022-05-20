/**
 * \file config/agentd.y
 *
 * \brief Parser for block configuration files.
 *
 * \copyright 2018-2021 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <agentd/config.h>
#include <vpr/parameters.h>

/**
 * \brief Helper macro for passing an error condition to the caller and breaking
 * out of the parse.
 */
#define CONFIG_ERROR(s) \
    context->set_error(context, (s)); \
    return NULL

/**
 * \brief Helper macro for breaking out of the parse if a NULL pointer is
 * returned by the helper method.
 */
#define MAYBE_ASSIGN(rhs, lhs) \
    do { \
        typeof (rhs) x = (lhs); \
        if (NULL == x) YYACCEPT; \
        (rhs) = x; \
    } while (false)

/* forward decls */
int yylex();
int yyerror(
    yyscan_t scanner, config_context_t* context, const char*);
static agent_config_t* new_config(config_context_t*);
static agent_config_t* add_logdir(
    config_context_t*, agent_config_t*, const char*);
static agent_config_t* add_loglevel(
    config_context_t*, agent_config_t*, int64_t);
static agent_config_t* add_datasize(
    config_context_t*, agent_config_t*, int64_t);
static agent_config_t* add_secret(
    config_context_t*, agent_config_t*, const char*);
static agent_config_t* add_rootblock(
    config_context_t*, agent_config_t*, const char*);
static agent_config_t* add_datastore(
    config_context_t*, agent_config_t*, const char*);
static agent_config_t* add_listen(
    agent_config_t*, config_listen_address_t*);
static agent_config_t* add_chroot(
    config_context_t*, agent_config_t*, const char*);
static agent_config_t* add_usergroup(
    config_context_t*, agent_config_t*, config_user_group_t*);
static config_user_group_t* create_user_group(
    config_context_t*, const char*, const char*);
static config_listen_address_t* create_listen_address(
    config_context_t*, struct in_addr*, int64_t);
static config_canonization_t* new_canonization(
    config_context_t*);
void config_dispose(void* disp);
static agent_config_t* fold_canonization(
    config_context_t*, agent_config_t*, config_canonization_t*);
static config_canonization_t* add_max_milliseconds(
    config_context_t*, config_canonization_t*, int64_t);
static config_canonization_t* add_max_transactions(
    config_context_t*, config_canonization_t*, int64_t);
void canonization_dispose(void* disp);
static agent_config_t* fold_view(
    config_context_t*, agent_config_t*, config_materialized_view_t*);
static config_materialized_view_t* view_new(
    config_context_t*);
static config_materialized_view_t* view_add_name(
    config_context_t*, config_materialized_view_t*, const char*);
static config_materialized_view_t* view_add_artifact(
    config_context_t*, config_materialized_view_t*,
    config_materialized_artifact_type_t*);
void view_dispose(void* disp);
static config_materialized_artifact_type_t* view_artifact_new(
    config_context_t*);
static config_materialized_artifact_type_t* view_artifact_add_uuid(
    config_context_t*, config_materialized_artifact_type_t*, vpr_uuid*);
static config_materialized_artifact_type_t* view_artifact_add_transaction(
    config_context_t*, config_materialized_artifact_type_t*,
    config_materialized_transaction_type_t*);
void view_artifact_dispose(void* disp);
static config_materialized_transaction_type_t* view_transaction_new(
    config_context_t*);
static config_materialized_transaction_type_t* view_transaction_add_uuid(
    config_context_t*, config_materialized_transaction_type_t*, vpr_uuid*);
static config_materialized_transaction_type_t* view_transaction_add_crud(
    config_context_t*, config_materialized_transaction_type_t*, int64_t);
static config_materialized_transaction_type_t* view_transaction_add_field(
    config_context_t*, config_materialized_transaction_type_t*,
    config_materialized_field_type_t*);
void view_transaction_dispose(void* disp);
static config_materialized_field_type_t* view_field_new(
    config_context_t*);
static config_materialized_field_type_t* view_field_add_uuid(
    config_context_t*, config_materialized_field_type_t*, vpr_uuid*);
void view_field_dispose(void* disp);
static agent_config_t* fold_private_key(
    config_context_t* context, agent_config_t* cfg,
    config_private_key_entry_t* private_key);
static config_private_key_entry_t* private_key_new(
    config_context_t* context, const char*);
void private_key_dispose(void* disp);
static agent_config_t* fold_endorser_key(
    config_context_t* context, agent_config_t* cfg,
    config_endorser_key_entry_t* private_key);
static config_endorser_key_entry_t* endorser_key_new(
    config_context_t* context, const char*);
void endorser_key_dispose(void* disp);
static agent_config_t* fold_public_key(
    config_context_t* context, agent_config_t* cfg,
    config_public_key_entry_t* public_key);
static config_public_key_entry_t* public_key_add_path(
    config_context_t* context, config_public_key_entry_t* head,
    const char* path);
void public_key_dispose(void* disp);
%}

/* use the full pure API for Bison. */
%define api.pure full
/* We pass the scanner to the lexer. */
%lex-param {yyscan_t scanner}
/* We pass the scanner to Bison. */
%parse-param {yyscan_t scanner}
/* We pass our config context structure to Bison. */
%parse-param {config_context_t* context}

/* Tokens. */
%token <string> APPEND
%token <string> AUTHORIZED
%token <string> ARTIFACT
%token <string> CANONIZATION
%token <string> CHROOT
%token <string> COLON
%token <string> COMMA
%token <string> CREATE
%token <string> DATASTORE
%token <string> DELETE
%token <string> ENDORSER
%token <string> ENTITIES
%token <string> FIELD
%token <string> IDENTIFIER
%token <addr> IP
%token <string> INVALID
%token <string> INVALID_IP
%token <string> KEY
%token <string> LBRACE
%token <string> LISTEN
%token <string> LOGDIR
%token <string> LOGLEVEL
%token <string> MATERIALIZED
%token <string> MAX
%token <number> NUMBER
%token <string> PATH
%token <string> PRIVATE
%token <string> RBRACE
%token <string> ROOTBLOCK
%token <string> MILLISECONDS
%token <string> SECRET
%token <string> SIZE
%token <string> TRANSACTION
%token <string> TRANSACTIONS
%token <string> TYPE
%token <string> UPDATE
%token <string> USERGROUP
%token <id> UUID
%token <id> UUID_INVALID
%token <string> VIEW

/* Types for branch nodes.. */
%type <config> conf
%type <string> chroot
%type <canonization> canonization
%type <canonization> canonization_block
%type <number> datasize
%type <string> datastore
%type <listenaddr> listen
%type <string> logdir
%type <number> loglevel
%type <private_key> private_key
%type <endorser_key> endorser_key
%type <public_key> public_key
%type <public_key> public_key_block
%type <string> rootblock
%type <string> secret
%type <usergroup> usergroup
%type <view> view
%type <view> view_block
%type <view_artifact> view_artifact
%type <view_artifact> view_artifact_block
%type <view_transaction> view_transaction
%type <view_transaction> view_transaction_block
%type <number> view_artifact_crud
%type <number> view_artifact_crud_block
%type <view_field> view_field
%type <view_field> view_field_block

%%

/* Base config rule.  Fold all config values into the empty config structure. */
conf : {
            /* create a new config. */
            MAYBE_ASSIGN($$, new_config(context));
            context->val_callback(context, $$); }
    | conf logdir {
            /* fold in logdir. */
            MAYBE_ASSIGN($$, add_logdir(context, $1, $2)); }
    | conf loglevel {
            /* fold in loglevel. */
            MAYBE_ASSIGN($$, add_loglevel(context, $1, $2)); }
    | conf datasize {
            /* fold in datasize. */
            MAYBE_ASSIGN($$, add_datasize(context, $1, $2)); }
    | conf secret {
            /* fold in secret. */
            MAYBE_ASSIGN($$, add_secret(context, $1, $2)); }
    | conf rootblock {
            /* fold in rootblock. */
            MAYBE_ASSIGN($$, add_rootblock(context, $1, $2)); }
    | conf datastore {
            /* fold in datastore. */
            MAYBE_ASSIGN($$, add_datastore(context, $1, $2)); }
    | conf listen {
            /* fold in listen address. */
            MAYBE_ASSIGN($$, add_listen($1, $2)); }
    | conf chroot {
            /* fold in chroot. */
            MAYBE_ASSIGN($$, add_chroot(context, $1, $2)); }
    | conf usergroup {
            /* fold in usergroup. */
            MAYBE_ASSIGN($$, add_usergroup(context, $1, $2)); }
    | conf canonization {
            /* fold in canonization data. */
            MAYBE_ASSIGN($$, fold_canonization(context, $1, $2)); }
    | conf view {
            /* fold in a materialized view. */
            MAYBE_ASSIGN($$, fold_view(context, $1, $2)); }
    | conf private_key {
            /* fold in a private key. */
            MAYBE_ASSIGN($$, fold_private_key(context, $1, $2)); }
    | conf endorser_key {
            /* fold in an endorser key. */
            MAYBE_ASSIGN($$, fold_endorser_key(context, $1, $2)); }
    | conf public_key {
            /* fold in public key. */
            MAYBE_ASSIGN($$, fold_public_key(context, $1, $2)); }
    ;

/* Provide a log directory that is either a simple identifier or a path. */
logdir
    : LOGDIR PATH {
            /* ownership is forwarded. */
            $$ = $2; }
    | LOGDIR IDENTIFIER {
            /* ownership is forwarded. */
            $$ = $2; }
    ;

loglevel
    : LOGLEVEL NUMBER {
            $$ = $2; }
    ;

datasize
    : MAX DATASTORE SIZE NUMBER {
            $$ = $4; }

/* Provide a secret file that is either a simple identifier or a path. */
secret
    : SECRET PATH {
            /* ownership is forwarded. */
            $$ = $2; }
    | SECRET IDENTIFIER {
            /* ownership is forwarded. */
            $$ = $2; }
    ;

/* Provide a root block file that is either a simple identifier or a path. */
rootblock
    : ROOTBLOCK PATH {
            /* ownership is forwarded. */
            $$ = $2; }
    | ROOTBLOCK IDENTIFIER {
            /* ownership is forwarded. */
            $$ = $2; }
    ;

/* Provide a datastore dir that is either a simple identifier or a path. */
datastore
    : DATASTORE PATH {
            /* ownership is forwarded. */
            $$ = $2; }
    | DATASTORE IDENTIFIER {
            /* ownership is forwarded. */
            $$ = $2; }

/* Provide a chroot dir that is either a simple identifier or a path. */
chroot
    : CHROOT PATH {
            /* ownership is forwarded. */
            $$ = $2; }
    | CHROOT IDENTIFIER {
            /* ownership is forwarded. */
            $$ = $2; }

/* Provide a listen address and port. */
listen
    : LISTEN IP COLON NUMBER {
            /* create listen param. */
            MAYBE_ASSIGN($$, create_listen_address(context, $2, $4)); }
    ;

/* Provide a user and group. */
usergroup
    : USERGROUP IDENTIFIER COLON IDENTIFIER {
            /* create usergroup param. */
            MAYBE_ASSIGN($$, create_user_group(context, $2, $4)); }
    ;

/* Provide a canonization block. */
canonization
    : CANONIZATION LBRACE canonization_block RBRACE {
            /* ownership is forwarded. */
            $$ = $3; }
    ;

canonization_block
    : {
            /* create a new canonization block. */
            MAYBE_ASSIGN($$, new_canonization(context)); }
    | canonization_block MAX MILLISECONDS NUMBER {
            /* override the max milliseconds. */
            MAYBE_ASSIGN($$, add_max_milliseconds(context, $$, $4)); }
    | canonization_block MAX TRANSACTIONS NUMBER {
            /* override the max transactions. */
            MAYBE_ASSIGN($$, add_max_transactions(context, $$, $4)); }
    ;

/* handle materialized view. */
view
    : MATERIALIZED VIEW IDENTIFIER LBRACE view_block RBRACE {
        /* ownership is forwarded. */
        MAYBE_ASSIGN($$, view_add_name(context, $5, $3)); }
    ;

view_block
    : {
            /* create a new view block. */
            MAYBE_ASSIGN($$, view_new(context)); }
    | view_block view_artifact {
            /* add artifact type to this view. */
            MAYBE_ASSIGN($$, view_add_artifact(context, $1, $2)); }
    ;

/* handle materialized view artifact. */
view_artifact
    : ARTIFACT TYPE UUID LBRACE view_artifact_block RBRACE {
            /* ownership is forwarded. */
            MAYBE_ASSIGN($$, view_artifact_add_uuid(context, $5, &$3)); }
    ;

view_artifact_block
    : {
            /* create a new artifact block. */
            MAYBE_ASSIGN($$, view_artifact_new(context)); }
    | view_artifact_block view_transaction {
            /* add transaction type to artifact type. */
            MAYBE_ASSIGN($$, view_artifact_add_transaction(context, $1, $2)); }
    ;

/* handle materialized view transaction. */
view_transaction
    : TRANSACTION TYPE UUID LBRACE view_transaction_block RBRACE {
            /* ownership is forwarded. */
            MAYBE_ASSIGN($$, view_transaction_add_uuid(context, $5, &$3)); }
    ;

view_transaction_block
    : {
            /* create a new transaction block. */
            MAYBE_ASSIGN($$, view_transaction_new(context)); }
    | view_transaction_block view_artifact_crud {
            /* add artifact detail to transaction block. */
            MAYBE_ASSIGN($$, view_transaction_add_crud(context, $1, $2)); }
    | view_transaction_block view_field {
            /* add field type to transaction block. */
            MAYBE_ASSIGN($$, view_transaction_add_field(context, $1, $2)); }
    ;

/* handle artifact level CRUD flags. */
view_artifact_crud
    : ARTIFACT LBRACE view_artifact_crud_block RBRACE {
            /* ownership is forwarded. */
            $$ = $3; }
    ;

view_artifact_crud_block
    : {
            /* create a new set of crud flags. */
            $$ = 0; }
    | view_artifact_crud_block CREATE {
            /* add CREATE crud flag. */
            $$ |= MATERIALIZED_VIEW_CRUD_CREATE; }
    | view_artifact_crud_block UPDATE {
            /* add an UPDATE crud flag. */
            $$ |= MATERIALIZED_VIEW_CRUD_UPDATE; }
    | view_artifact_crud_block APPEND {
            /* add an APPEND crud flag. */
            $$ |= MATERIALIZED_VIEW_CRUD_APPEND; }
    | view_artifact_crud_block DELETE {
            /* add a DELETE crud flag. */
            $$ |= MATERIALIZED_VIEW_CRUD_DELETE; }
    ;

/* handle materialized view field. */
view_field
    : FIELD TYPE UUID LBRACE view_field_block RBRACE {
            /* ownership is forwarded. */
            MAYBE_ASSIGN($$, view_field_add_uuid(context, $5, &$3)); }
    ;

view_field_block
    : {
            /* create a new field block. */
            MAYBE_ASSIGN($$, view_field_new(context)); }
    | view_field_block CREATE {
            /* add CREATE crud flag. */
            $$->field_crud_flags |= MATERIALIZED_VIEW_CRUD_CREATE; }
    | view_field_block UPDATE {
            /* add UPDATE crud flag. */
            $$->field_crud_flags |= MATERIALIZED_VIEW_CRUD_UPDATE; }
    | view_field_block APPEND {
            /* add APPEND crud flag. */
            $$->field_crud_flags |= MATERIALIZED_VIEW_CRUD_APPEND; }
    | view_field_block DELETE {
            /* add DELETE crud flag. */
            $$->field_crud_flags |= MATERIALIZED_VIEW_CRUD_DELETE; }
    ;

/* handle private key. */
private_key
    : PRIVATE KEY PATH {
        /* create a new private_key. */
        MAYBE_ASSIGN($$, private_key_new(context, $3)); }
    ;

/* handle endorser key. */
endorser_key
    : ENDORSER KEY PATH {
        /* create a new endorser key. */
        MAYBE_ASSIGN($$, endorser_key_new(context, $3)); }

/* handle public key. */
public_key
    : AUTHORIZED ENTITIES LBRACE public_key_block RBRACE {
        /* ownership is forwarded. */
        $$ = $4; }
    ;

public_key_block
    : {
        /* start with a null block. */
        $$ = NULL; }
    | public_key_block PATH {
        MAYBE_ASSIGN($$, public_key_add_path(context, $1, $2)); }
    ;
%%

/**
 * \brief Create a new configuration structure.
 */
static agent_config_t* new_config(config_context_t* context)
{
    agent_config_t* ret = (agent_config_t*)malloc(sizeof(agent_config_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in new_config().");
    }

    memset(ret, 0, sizeof(agent_config_t));
    ret->hdr.dispose = &config_dispose;

    return ret;
}

/**
 * \brief Add a log directory to the config structure.
 */
static agent_config_t* add_logdir(
    config_context_t* context, agent_config_t* cfg, const char* logdir)
{
    if (NULL != cfg->logdir)
    {
        CONFIG_ERROR("Duplicate logdir settings.");
    }

    cfg->logdir = logdir;

    return cfg;
}

/**
 * \brief Add a log level to the config structure.
 */
static agent_config_t* add_loglevel(
    config_context_t* context, agent_config_t* cfg, int64_t loglevel)
{
    if (cfg->loglevel_set)
    {
        CONFIG_ERROR("Duplicate loglevel settings.");
    }

    if (loglevel > 9 || loglevel < 0)
    {
        CONFIG_ERROR("Bad loglevel range.");
    }

    cfg->loglevel_set = true;
    cfg->loglevel = loglevel;

    return cfg;
}

/**
 * \brief Add a data size to the config structure.
 */
static agent_config_t* add_datasize(
    config_context_t* context, agent_config_t* cfg, int64_t datasize)
{
    if (cfg->database_max_size_set)
    {
        CONFIG_ERROR("Duplicate max database size settings.");
    }

    if (datasize <= 0)
    {
        CONFIG_ERROR("Bad max database size value.");
    }

    cfg->database_max_size_set = true;
    cfg->database_max_size = datasize;

    return cfg;
}

/**
 * \brief Add a secret to the config structure.
 */
static agent_config_t* add_secret(
    config_context_t* context, agent_config_t* cfg, const char* secret)
{
    if (NULL != cfg->secret)
    {
        CONFIG_ERROR("Duplicate secrets set.");
    }

    cfg->secret = secret;

    return cfg;
}

/**
 * \brief Add a root block to the config structure.
 */
static agent_config_t* add_rootblock(
    config_context_t* context, agent_config_t* cfg, const char* rootblock)
{
    if (NULL != cfg->rootblock)
    {
        CONFIG_ERROR("Duplicate rootblocks set.");
    }

    cfg->rootblock = rootblock;

    return cfg;
}

/**
 * \brief Add a datastore to the config structure.
 */
static agent_config_t* add_datastore(
    config_context_t* context, agent_config_t* cfg, const char* datastore)
{
    if (NULL != cfg->datastore)
    {
        CONFIG_ERROR("Duplicate datastores set.");
    }

    cfg->datastore = datastore;

    return cfg;
}

/**
 * \brief Add a listen address / port to the config structure.
 */
static agent_config_t* add_listen(
    agent_config_t* cfg, config_listen_address_t* listen)
{
    /* cons this element onto the list. */
    listen->hdr.next = (config_list_node_t*)cfg->listen_head;
    cfg->listen_head = listen;

    return cfg;
}

/**
 * \brief Add a chroot directory to the config structure.
 */
static agent_config_t* add_chroot(
    config_context_t* context, agent_config_t* cfg, const char* chroot)
{
    if (NULL != cfg->chroot)
    {
        CONFIG_ERROR("Duplicate chroots set.");
    }

    cfg->chroot = chroot;

    return cfg;
}

/**
 * \brief Add a user and group to the config structure.
 */
static agent_config_t*
add_usergroup(
    config_context_t* context, agent_config_t* cfg,
    config_user_group_t* usergroup)
{
    if (NULL != cfg->usergroup)
    {
        CONFIG_ERROR("Duplicate usergroups set.");
    }

    cfg->usergroup = usergroup;

    return cfg;
}

/**
 * \brief Create a user and group from strings.
 */
static config_user_group_t*
create_user_group(
    config_context_t* context, const char* user, const char* group)
{
    config_user_group_t* usergroup =
        (config_user_group_t*)malloc(sizeof(config_user_group_t));
    if (NULL == usergroup)
    {
        CONFIG_ERROR("Out of memory in create_user_group().");
    }

    usergroup->user = user;
    usergroup->group = group;

    return usergroup;
}

/**
 * \brief Create a listen address from strings.
 */
static config_listen_address_t*
create_listen_address(
    config_context_t* context, struct in_addr* addr, int64_t port)
{
    config_listen_address_t* listen =
        (config_listen_address_t*)malloc(sizeof(config_listen_address_t));
    if (NULL == listen)
    {
        CONFIG_ERROR("Out of memory in create_listen_address().");
    }

    listen->addr = addr;
    listen->port = (in_port_t)port;

    return listen;
}

/**
 * \brief dispose of a config structure.
 */
void config_dispose(void* disp)
{
    agent_config_t* cfg = (agent_config_t*)disp;

    if (NULL != cfg->logdir)
        free((char*)cfg->logdir);
    if (NULL != cfg->secret)
        free((char*)cfg->secret);
    if (NULL != cfg->rootblock)
        free((char*)cfg->rootblock);
    if (NULL != cfg->datastore)
        free((char*)cfg->datastore);
    if (NULL != cfg->chroot)
        free((char*)cfg->chroot);

    while (NULL != cfg->listen_head)
    {
        config_listen_address_t* tmp =
            (config_listen_address_t*)cfg->listen_head->hdr.next;
        free(cfg->listen_head->addr);
        free(cfg->listen_head);
        cfg->listen_head = tmp;
    }

    if (NULL != cfg->usergroup)
    {
        free((char*)cfg->usergroup->user);
        free((char*)cfg->usergroup->group);
        free(cfg->usergroup);
    }

    while (NULL != cfg->view_head)
    {
        config_materialized_view_t* tmp =
            (config_materialized_view_t*)cfg->view_head->hdr.next;
        cfg->view_head->hdr.next = NULL;
        dispose((disposable_t*)cfg->view_head);
        free(cfg->view_head);
        cfg->view_head = tmp;
    }

    if (NULL != cfg->private_key)
    {
        dispose((disposable_t*)cfg->private_key);
        free(cfg->private_key);
    }

    if (NULL != cfg->endorser_key)
    {
        dispose((disposable_t*)cfg->endorser_key);
        free(cfg->endorser_key);
    }

    while (NULL != cfg->public_key_head)
    {
        config_public_key_entry_t* tmp =
            (config_public_key_entry_t*)cfg->public_key_head->hdr.next;
        cfg->public_key_head->hdr.next = NULL;
        dispose((disposable_t*)cfg->public_key_head);
        free(cfg->public_key_head);
        cfg->public_key_head = tmp;
    }
}

/**
 * \brief Create a new canonization structure.
 */
static config_canonization_t* new_canonization(config_context_t* context)
{
    config_canonization_t* ret =
        (config_canonization_t*)malloc(sizeof(config_canonization_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in new_canonization().");
    }

    memset(ret, 0, sizeof(config_canonization_t));
    ret->hdr.dispose = &canonization_dispose;

    return ret;
}

/**
 * \brief Add the maximum milliseconds to the canonization config.
 */
static config_canonization_t* add_max_milliseconds(
    config_context_t* context, config_canonization_t* canonization,
    int64_t milliseconds)
{
    if (canonization->block_max_milliseconds_set)
    {
        CONFIG_ERROR("Duplicate max milliseconds setting.");
    }

    if (milliseconds < 0 || milliseconds > BLOCK_MILLISECONDS_MAXIMUM)
    {
        CONFIG_ERROR("Invalid milliseconds range.");
    }

    canonization->block_max_milliseconds_set = true;
    canonization->block_max_milliseconds = milliseconds;

    return canonization;
}

/**
 * \brief Add the maximum transactions to the canonization config.
 */
static config_canonization_t* add_max_transactions(
    config_context_t* context, config_canonization_t* canonization,
    int64_t transactions)
{
    if (canonization->block_max_transactions_set)
    {
        CONFIG_ERROR("Duplicate max transactions setting.");
    }

    if (transactions < 0 || transactions > BLOCK_TRANSACTIONS_MAXIMUM)
    {
        CONFIG_ERROR("Invalid transactions range.");
    }

    canonization->block_max_transactions_set = true;
    canonization->block_max_transactions = transactions;

    return canonization;
}

/**
 * \brief Fold canonization data into the config structure.
 */
static agent_config_t* fold_canonization(
    config_context_t* context, agent_config_t* cfg,
    config_canonization_t* canonization)
{
    /* only allow the max milliseconds to be set once. */
    if (cfg->block_max_milliseconds_set
     && canonization->block_max_milliseconds_set)
    {
        CONFIG_ERROR("Duplicate canonization max milliseconds settings.");
    }

    /* assign max milliseconds if set. */
    cfg->block_max_milliseconds_set = canonization->block_max_milliseconds_set;
    if (canonization->block_max_milliseconds_set)
    {
        cfg->block_max_milliseconds = canonization->block_max_milliseconds;
    }

    /* only allow the max transactions to be set once. */
    if (cfg->block_max_transactions_set
     && canonization->block_max_transactions_set)
    {
        CONFIG_ERROR("Duplicate canonization max transactions settings.");
    }

    /* assign max transactions if set. */
    cfg->block_max_transactions_set = canonization->block_max_transactions_set;
    if (canonization->block_max_transactions_set)
    {
        cfg->block_max_transactions = canonization->block_max_transactions;
    }

    /* dispose of the canonization structure. */
    dispose((disposable_t*)canonization);
    /* free the canonization structure. */
    free(canonization);

    return cfg;
}

/**
 * \brief dispose of a canonization structure.
 */
void canonization_dispose(void* disp)
{
    config_canonization_t* cfg = (config_canonization_t*)disp;

    /* nothing to do here yet, as it currently contains just ints and bools.  */
    (void)cfg;
}

/**
 * \brief Fold materialized view data into the config.
 */
static agent_config_t* fold_view(
    config_context_t* context, agent_config_t* cfg,
    config_materialized_view_t* view)
{
    /* scan the list for a view matching this name. */
    config_materialized_view_t* tmp = cfg->view_head;
    while (NULL != tmp)
    {
        if (!strcmp(tmp->name, view->name))
        {
            CONFIG_ERROR("Duplicate materialized view names.");
        }

        tmp = (config_materialized_view_t*)tmp->hdr.next;
    }

    /* if there are no duplicates, add this to our list. */
    view->hdr.next = &cfg->view_head->hdr;
    cfg->view_head = view;

    return cfg;
}

/**
 * \brief Create a new view structure.
 */
static config_materialized_view_t* view_new(config_context_t* context)
{
    config_materialized_view_t* ret =
        (config_materialized_view_t*)malloc(sizeof(config_materialized_view_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in view_new().");
    }

    memset(ret, 0, sizeof(config_materialized_view_t));
    ret->hdr.hdr.dispose = &view_dispose;

    return ret;
}

/**
 * \brief dispose of a view structure.
 */
void view_dispose(void* disp)
{
    config_materialized_view_t* view = (config_materialized_view_t*)disp;

    if (NULL != view->name)
    {
        free((char*)view->name);
    }

    while (NULL != view->artifact_head)
    {
        config_materialized_artifact_type_t* tmp =
            (config_materialized_artifact_type_t*)view->artifact_head->hdr.next;
        view->artifact_head->hdr.next = NULL;
        dispose((disposable_t*)view->artifact_head);
        free(view->artifact_head);
        view->artifact_head = tmp;
    }
}

/**
 * \brief Add a name to the view.
 */
static config_materialized_view_t* view_add_name(
    config_context_t* UNUSED(context), config_materialized_view_t* view,
    const char* name)
{
    view->name = name;

    return view;
}

/**
 * \brief Add an artifact type to a view.
 */
static config_materialized_view_t* view_add_artifact(
    config_context_t* context, config_materialized_view_t* view,
    config_materialized_artifact_type_t* artifact)
{
    /* scan the list for an artifact matching this type. */
    config_materialized_artifact_type_t* tmp = view->artifact_head;
    while (NULL != tmp)
    {
        if (
            !memcmp(
                &tmp->artifact_type, &artifact->artifact_type,
                sizeof(vpr_uuid)))
        {
            CONFIG_ERROR("Duplicate artifact types.");
        }

        tmp = (config_materialized_artifact_type_t*)tmp->hdr.next;
    }

    /* if there are no duplicaes, add this to our list. */
    artifact->hdr.next = &view->artifact_head->hdr;
    view->artifact_head = artifact;

    return view;
}

/**
 * \brief Create a new artifact view structure.
 */
static config_materialized_artifact_type_t* view_artifact_new(
    config_context_t* context)
{
    config_materialized_artifact_type_t* ret =
        (config_materialized_artifact_type_t*)malloc(
            sizeof(config_materialized_artifact_type_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in view_artifact_new().");
    }

    memset(ret, 0, sizeof(config_materialized_artifact_type_t));
    ret->hdr.hdr.dispose = &view_artifact_dispose;

    return ret;
}

/**
 * \brief dispose of an artifact view structure.
 */
void view_artifact_dispose(void* disp)
{
    config_materialized_artifact_type_t* artifact =
        (config_materialized_artifact_type_t*)disp;

    while (NULL != artifact->transaction_head)
    {
        config_materialized_transaction_type_t* tmp =
            (config_materialized_transaction_type_t*)
            artifact->transaction_head->hdr.next;
        artifact->transaction_head->hdr.next = NULL;
        dispose((disposable_t*)artifact->transaction_head);
        free(artifact->transaction_head);
        artifact->transaction_head = tmp;
    }
}

/**
 * \brief Add the uuid to an artifact type.
 */
static config_materialized_artifact_type_t* view_artifact_add_uuid(
    config_context_t* UNUSED(context),
    config_materialized_artifact_type_t* artifact, vpr_uuid* uuid)
{
    memcpy(&artifact->artifact_type, uuid, sizeof(vpr_uuid));

    return artifact;
}

/**
 * \brief Add a transaction type to an artifact type.
 */
static config_materialized_artifact_type_t* view_artifact_add_transaction(
    config_context_t* context, config_materialized_artifact_type_t* artifact,
    config_materialized_transaction_type_t* transaction)
{
    /* scan the list for a transaction matching this type. */
    config_materialized_transaction_type_t* tmp = artifact->transaction_head;
    while (NULL != tmp)
    {
        if (
            !memcmp(
                &tmp->transaction_type, &transaction->transaction_type,
                sizeof(vpr_uuid)))
        {
            CONFIG_ERROR("Duplicate transaction types.");
        }

        tmp = (config_materialized_transaction_type_t*)tmp->hdr.next;
    }

    /* if there are no duplicates, add this to our list. */
    transaction->hdr.next = &artifact->transaction_head->hdr;
    artifact->transaction_head = transaction;

    return artifact;
}

/**
 * \brief Create a new transaction view structure.
 */
static config_materialized_transaction_type_t* view_transaction_new(
    config_context_t* context)
{
    config_materialized_transaction_type_t* ret =
        (config_materialized_transaction_type_t*)malloc(
            sizeof(config_materialized_transaction_type_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in view_transaction_new().");
    }

    memset(ret, 0, sizeof(config_materialized_transaction_type_t));
    ret->hdr.hdr.dispose = &view_transaction_dispose;

    return ret;
}

/**
 * \brief dispose of a transaction view structure.
 */
void view_transaction_dispose(void* disp)
{
    config_materialized_transaction_type_t* transaction =
        (config_materialized_transaction_type_t*)disp;

    while (NULL != transaction->field_head)
    {
        config_materialized_field_type_t* tmp =
            (config_materialized_field_type_t*)
            transaction->field_head->hdr.next;
        transaction->field_head->hdr.next = NULL;
        dispose((disposable_t*)transaction->field_head);
        free(transaction->field_head);
        transaction->field_head = tmp;
    }
}

/**
 * \brief Add the uuid to a transaction type.
 */
static config_materialized_transaction_type_t* view_transaction_add_uuid(
    config_context_t* UNUSED(context),
    config_materialized_transaction_type_t* transaction, vpr_uuid* uuid)
{
    memcpy(&transaction->transaction_type, uuid, sizeof(vpr_uuid));

    return transaction;
}

/**
 * \brief Add crud flags to this transaction type.
 */
static config_materialized_transaction_type_t* view_transaction_add_crud(
    config_context_t* UNUSED(context),
    config_materialized_transaction_type_t* transaction, int64_t flags)
{
    transaction->artifact_crud_flags = (uint32_t)flags;

    return transaction;
}

/**
 * \brief Add a field to a transaction type.
 */
static config_materialized_transaction_type_t* view_transaction_add_field(
    config_context_t* context,
    config_materialized_transaction_type_t* transaction,
    config_materialized_field_type_t* field)
{
    /* scan the list for a field matching this type. */
    config_materialized_field_type_t* tmp = transaction->field_head;
    while (NULL != tmp)
    {
        if (!memcmp(&tmp->field_code, &field->field_code, sizeof(vpr_uuid)))
        {
            CONFIG_ERROR("Duplicate field types.");
        }

        tmp = (config_materialized_field_type_t*)tmp->hdr.next;
    }

    /* if there are no duplicates, add this to our list. */
    field->hdr.next = &transaction->field_head->hdr;
    transaction->field_head = field;

    return transaction;
}

/**
 * \brief Create a new field view structure.
 */
static config_materialized_field_type_t* view_field_new(
    config_context_t* context)
{
    config_materialized_field_type_t* ret =
        (config_materialized_field_type_t*)malloc(
            sizeof(config_materialized_field_type_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in view_field_new().");
    }

    memset(ret, 0, sizeof(config_materialized_field_type_t));
    ret->hdr.hdr.dispose = &view_field_dispose;

    return ret;
}

/**
 * \brief dispose of a field view structure.
 */
void view_field_dispose(void* UNUSED(disp))
{
    /* nothing needs to be done here. */
}

/**
 * \brief Add the uuid to a field type.
 */
static config_materialized_field_type_t* view_field_add_uuid(
    config_context_t* UNUSED(context), config_materialized_field_type_t* field,
    vpr_uuid* uuid)
{
    memcpy(&field->field_code, uuid, sizeof(vpr_uuid));

    return field;
}

/**
 * \brief Fold private key into the config.
 */
static agent_config_t* fold_private_key(
    config_context_t* context, agent_config_t* cfg,
    config_private_key_entry_t* private_key)
{
    /* has a private key already been set? */
    if (NULL != cfg->private_key)
    {
        CONFIG_ERROR("Duplicate private key entry in config.");
    }

    /* if this is the only private key, then add it. */
    cfg->private_key = private_key;

    return cfg;
}

/**
 * \brief Create a new private key entry with the given path.
 */
static config_private_key_entry_t* private_key_new(
    config_context_t* context, const char* path)
{
    config_private_key_entry_t* ret =
        (config_private_key_entry_t*)malloc(sizeof(config_private_key_entry_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in private_key_new().");
    }

    memset(ret, 0, sizeof(config_private_key_entry_t));
    ret->hdr.dispose = &private_key_dispose;
    ret->filename = strdup(path);

    return ret;
}

/**
 * \brief Dispose of a private key entry.
 */
void private_key_dispose(void* disp)
{
    config_private_key_entry_t* private_key = (config_private_key_entry_t*)disp;

    free((char*)private_key->filename);
}

/**
 * \brief Fold endorser key into the config.
 */
static agent_config_t* fold_endorser_key(
    config_context_t* context, agent_config_t* cfg,
    config_endorser_key_entry_t* endorser_key)
{
    /* has an endorser key already been set? */
    if (NULL != cfg->endorser_key)
    {
        CONFIG_ERROR("Duplicate endorser key entry in config.");
    }

    /* if this is the only endorser key, then add it. */
    cfg->endorser_key = endorser_key;

    return cfg;
}

/**
 * \brief Create a new endorser key entry with the given path.
 */
static config_endorser_key_entry_t* endorser_key_new(
    config_context_t* context, const char* path)
{
    config_endorser_key_entry_t* ret =
        (config_endorser_key_entry_t*)malloc(
            sizeof(config_endorser_key_entry_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in endorser_key_new().");
    }

    memset(ret, 0, sizeof(config_endorser_key_entry_t));
    ret->hdr.dispose = &endorser_key_dispose;
    ret->filename = strdup(path);

    return ret;
}

/**
 * \brief Dispose of an endorser key entry.
 */
void endorser_key_dispose(void* disp)
{
    config_endorser_key_entry_t* endorser_key =
        (config_endorser_key_entry_t*)disp;

    free((char*)endorser_key->filename);
}

/**
 * \brief Fold public key into the config.
 */
static agent_config_t* fold_public_key(
    config_context_t* UNUSED(context), agent_config_t* cfg,
    config_public_key_entry_t* public_key)
{
    /* if there are no entries, exit. */
    if (NULL == public_key)
    {
        return cfg;
    }

    /* if there are no entries, add them. */
    if (NULL == cfg->public_key_head)
    {
        cfg->public_key_head = public_key;
    }
    /* otherwise, iterate through the entries to find the last entry, and fix up
     * the head with these entries. This preserves the list in reverse order. */
    else
    {
        config_public_key_entry_t* tmp = public_key;

        /* get to the last new entry. */
        while (NULL != tmp->hdr.next)
        {
            tmp = (config_public_key_entry_t*)tmp->hdr.next;
        }

        /* set the head of this entry to our current head. */
        tmp->hdr.next = (config_disposable_list_node_t*)cfg->public_key_head;

        /* set the public key head to the first entry. */
        cfg->public_key_head = public_key;
    }

    return cfg;
}

/**
 * \brief Add a public key file path to the config. */
static config_public_key_entry_t* public_key_add_path(
    config_context_t* context, config_public_key_entry_t* head,
    const char* path)
{
    config_public_key_entry_t* ret =
        (config_public_key_entry_t*)malloc(sizeof(config_public_key_entry_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in public_key_new().");
    }

    memset(ret, 0, sizeof(config_public_key_entry_t));
    ret->hdr.hdr.dispose = &public_key_dispose;
    ret->hdr.next = (config_disposable_list_node_t*)head;
    ret->filename = strdup(path);

    return ret;
}

/**
 * \brief Dispose of a public key entry.
 */
void public_key_dispose(void* disp)
{
    config_public_key_entry_t* public_key = (config_public_key_entry_t*)disp;

    free((char*)public_key->filename);
}

/**
 * \brief Set the error for the config structure.
 */
int yyerror(
    yyscan_t UNUSED(scanner), config_context_t* context, const char* msg)
{
    context->set_error(context, msg);

    return 1;
}
