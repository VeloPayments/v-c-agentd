/**
 * \file config/agentd.y
 *
 * \brief Parser for block configuration files.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
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
static config_consensus_t* new_consensus(
    config_context_t*);
void config_dispose(void* disp);
static agent_config_t* fold_consensus(
    config_context_t*, agent_config_t*, config_consensus_t*);
static config_consensus_t* add_max_milliseconds(
    config_context_t*, config_consensus_t*, int64_t);
static config_consensus_t* add_max_transactions(
    config_context_t*, config_consensus_t*, int64_t);
void consensus_dispose(void* disp);
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
%token <string> CHROOT
%token <string> COLON
%token <string> CONSENSUS
%token <string> DATASTORE
%token <string> IDENTIFIER
%token <addr> IP
%token <string> INVALID
%token <string> INVALID_IP
%token <string> LBRACE
%token <string> LISTEN
%token <string> LOGDIR
%token <string> LOGLEVEL
%token <string> MAX
%token <number> NUMBER
%token <string> PATH
%token <string> RBRACE
%token <string> ROOTBLOCK
%token <string> MILLISECONDS
%token <string> SECRET
%token <string> TRANSACTIONS
%token <string> USERGROUP

/* Types for branch nodes.. */
%type <config> conf
%type <string> chroot
%type <consensus> consensus
%type <consensus> consensus_block
%type <string> datastore
%type <listenaddr> listen
%type <string> logdir
%type <number> loglevel
%type <string> rootblock
%type <string> secret
%type <usergroup> usergroup

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
    | conf consensus {
            /* fold in consensus data. */
            MAYBE_ASSIGN($$, fold_consensus(context, $1, $2)); }
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

/* Provide a consensus block. */
consensus
    : CONSENSUS LBRACE consensus_block RBRACE {
            /* ownership is forwarded. */
            $$ = $3; }
    ;

consensus_block
    : {
            /* create a new consensus block. */
            MAYBE_ASSIGN($$, new_consensus(context)); }
    | consensus_block MAX MILLISECONDS NUMBER {
            /* override the max milliseconds. */
            MAYBE_ASSIGN($$, add_max_milliseconds(context, $$, $4)); }
    | consensus_block MAX TRANSACTIONS NUMBER {
            /* override the max transactions. */
            MAYBE_ASSIGN($$, add_max_transactions(context, $$, $4)); }
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
}

/**
 * \brief Create a new consensus structure.
 */
static config_consensus_t* new_consensus(config_context_t* context)
{
    config_consensus_t* ret =
        (config_consensus_t*)malloc(sizeof(config_consensus_t));
    if (NULL == ret)
    {
        CONFIG_ERROR("Out of memory in new_consensus().");
    }

    memset(ret, 0, sizeof(config_consensus_t));
    ret->hdr.dispose = &consensus_dispose;

    return ret;
}

/**
 * \brief Add the maximum milliseconds to the consensus config.
 */
static config_consensus_t* add_max_milliseconds(
    config_context_t* context, config_consensus_t* consensus,
    int64_t milliseconds)
{
    if (consensus->block_max_milliseconds_set)
    {
        CONFIG_ERROR("Duplicate max milliseconds setting.");
    }

    if (milliseconds < 0 || milliseconds > BLOCK_MILLISECONDS_MAXIMUM)
    {
        CONFIG_ERROR("Invalid milliseconds range.");
    }

    consensus->block_max_milliseconds_set = true;
    consensus->block_max_milliseconds = milliseconds;

    return consensus;
}

/**
 * \brief Add the maximum transactions to the consensus config.
 */
static config_consensus_t* add_max_transactions(
    config_context_t* context, config_consensus_t* consensus,
    int64_t transactions)
{
    if (consensus->block_max_transactions_set)
    {
        CONFIG_ERROR("Duplicate max transactions setting.");
    }

    if (transactions < 0 || transactions > BLOCK_TRANSACTIONS_MAXIMUM)
    {
        CONFIG_ERROR("Invalid transactions range.");
    }

    consensus->block_max_transactions_set = true;
    consensus->block_max_transactions = transactions;

    return consensus;
}

/**
 * \brief Fold consensus data into the config structure.
 */
static agent_config_t* fold_consensus(
    config_context_t* context, agent_config_t* cfg,
    config_consensus_t* consensus)
{
    /* only allow the max milliseconds to be set once. */
    if (cfg->block_max_milliseconds_set
     && consensus->block_max_milliseconds_set)
    {
        CONFIG_ERROR("Duplicate consensus max milliseconds settings.");
    }

    /* assign max milliseconds if set. */
    cfg->block_max_milliseconds_set = consensus->block_max_milliseconds_set;
    if (consensus->block_max_milliseconds_set)
    {
        cfg->block_max_milliseconds = consensus->block_max_milliseconds;
    }

    /* only allow the max transactions to be set once. */
    if (cfg->block_max_transactions_set
     && consensus->block_max_transactions_set)
    {
        CONFIG_ERROR("Duplicate consensus max transactions settings.");
    }

    /* assign max transactions if set. */
    cfg->block_max_transactions_set = consensus->block_max_transactions_set;
    if (consensus->block_max_transactions_set)
    {
        cfg->block_max_transactions = consensus->block_max_transactions;
    }

    /* dispose of the consensus structure. */
    dispose((disposable_t*)consensus);
    /* free the consensus structure. */
    free(consensus);

    return cfg;
}

/**
 * \brief dispose of a consensus structure.
 */
void consensus_dispose(void* disp)
{
    config_consensus_t* cfg = (config_consensus_t*)disp;

    /* nothing to do here yet, as it currently contains just ints and bools.  */
    (void)cfg;
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
