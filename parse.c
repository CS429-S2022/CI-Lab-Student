#include "ci.h"

extern lptr_t this_token, next_token;
extern void init_lexer(void);
extern void advance_lexer(void);

static const char *VALID_FMTS = "dxXbB";
static const int NUM_RESERVED_IDS = 2;
static const struct {
    char *id;
    token_t t;
} reserved_ids[] = {
    {"true", TOK_TRUE},
    {"false", TOK_FALSE}
};

bool is_binop(token_t t) {
    return t >= TOK_PLUS && t <= TOK_EQ;
}

bool is_unop(token_t t) {
    return t >= TOK_UMINUS && t <= TOK_NOT;
}

bool id_is_fmt_spec(char *s) {
    return strlen(s) == 1 && strspn(s, VALID_FMTS) == 1;
}

static token_t check_reserved_ids(char *s) {
    for (int i = 0; i < NUM_RESERVED_IDS; i++)
        if (strcmp(reserved_ids[i].id, s) == 0) return reserved_ids[i].t;
    return TOK_INVALID;
}

static node_t *build_leaf(void) {
    node_t *ret = calloc(1, sizeof(node_t));
    if (! ret) {
        logging(LOG_FATAL, "failed to allocate node");
        return NULL;
    }
    ret->node_type = NT_LEAF;
    ret->type = NO_TYPE;
    ret->tok = this_token->ttype;
    switch (this_token->ttype) {
        case TOK_NUM:
            ret->val.ival = atoi(this_token->repr);
            ret->type = INT_TYPE;
            break;
        case TOK_TRUE:
        case TOK_FALSE:
            ret->val.bval = this_token->ttype == TOK_TRUE;
            ret->type = BOOL_TYPE;
            break;
        case TOK_STR:
            ret->val.sval = (char *) malloc(strlen(this_token->repr) + 1);
            if (! ret->val.sval) {
                free(ret);
                logging(LOG_FATAL, "failed to allocate string");
                return NULL;
            }
            strcpy(ret->val.sval, this_token->repr);
            ret->type = STRING_TYPE;
            break;
        case TOK_FMT_SPEC:
            ret->val.fval = this_token->repr[0];
            ret->type = FMT_TYPE;
            break;
        case TOK_ID:
            ret->val.sval = (char *) malloc(strlen(this_token->repr) + 1);
            if (! ret->val.sval) {
                free(ret);
                logging(LOG_FATAL, "failed to allocate string");
                return NULL;
            }
            strcpy(ret->val.sval, this_token->repr);
            ret->type = ID_TYPE;
            break;
        default:
            logging(LOG_ERROR, "unknown token type");
            free(ret); ret = NULL;
            break;
    }
    return ret;
}

static node_t *build_exp(void) {
    // check running status
    if (terminate || ignore_input) return NULL;

    token_t t;

    if (this_token->ttype == TOK_NUM)               // P5 -> P23
        return build_leaf();
    if (this_token->ttype == TOK_STR)               // P5 -> P??
        return build_leaf();
    if (this_token->ttype == TOK_ID) {         // P24, P25
        if ((t = check_reserved_ids(this_token->repr)) != TOK_INVALID) {
            this_token->ttype = t;
        }
        return build_leaf();
    } else {
        if (this_token->ttype != TOK_LPAREN) {
            handle_error(ERR_SYNTAX);
            return NULL;
        }
        node_t *ret = calloc(1, sizeof(node_t));
        if (! ret) {
            logging(LOG_FATAL, "failed to allocate node");
            return NULL;
        }
        ret->node_type = NT_INTERNAL;
        ret->type = NO_TYPE;
        advance_lexer();
        if (is_unop(this_token->ttype)) {           // P9
            ret->tok = this_token->ttype;
            advance_lexer();
            ret->children[0] = build_exp();
            if (next_token->ttype != TOK_RPAREN) {
                handle_error(ERR_SYNTAX);
                return ret;
            }
            advance_lexer();
        }
        else {
            ret->children[0] = build_exp();
            if (is_binop(next_token->ttype)) {      // P8
                ret->tok = next_token->ttype;
                advance_lexer();
                advance_lexer();
                ret->children[1] = build_exp();
                if (next_token->ttype != TOK_RPAREN) {
                    handle_error(ERR_SYNTAX);
                    return ret;
                }
                advance_lexer();
            }
            else if (next_token->ttype == TOK_QUESTION) {   // P7
                ret->tok = next_token->ttype;
                advance_lexer();
                advance_lexer();
                ret->children[1] = build_exp();
                if (next_token->ttype != TOK_COLON) {
                    handle_error(ERR_SYNTAX);
                    return ret;
                }
                advance_lexer();
                advance_lexer();
                ret->children[2] = build_exp();
                if (next_token->ttype != TOK_RPAREN) {
                    handle_error(ERR_SYNTAX);
                    return ret;
                }
                advance_lexer();
            }
            else {                                  // P10
                if (next_token->ttype != TOK_RPAREN) {
                    handle_error(ERR_SYNTAX);
                    return ret;
                }
                ret->tok = TOK_IDENTITY;
                advance_lexer();
            }
        }
        return ret;
    }
}

static node_t *build_root(void) {
    // check running status
    if (terminate || ignore_input) return NULL;

    node_t *ret = calloc(1, sizeof(node_t));
    if (! ret) {
        logging(LOG_FATAL, "failed to allocate node");
        return NULL;
    }
    ret->node_type = NT_ROOT;
    ret->type = NO_TYPE;
    // check for assignment
    if (this_token->ttype == TOK_ID && next_token->ttype == TOK_ASSIGN) {
        if (check_reserved_ids(this_token->repr) != TOK_INVALID) {
            logging(LOG_ERROR, "variable name is reserved");
            return ret;
        }
        ret->type = ID_TYPE;
        ret->children[0] = build_leaf();
        advance_lexer();
        advance_lexer();
        ret->children[1] = build_exp();
        if (next_token->ttype != TOK_EOL) {
            handle_error(ERR_SYNTAX);
        }
        return ret;
    }
    
    ret->children[0] = build_exp();
    if (next_token->ttype == TOK_EOL)               // P2
        return ret;
    else {                                          // P3
        if (next_token->ttype != TOK_SEP) {
            handle_error(ERR_SYNTAX);
            return ret;
        }
        advance_lexer();
        if (next_token->ttype != TOK_ID) {
            handle_error(ERR_SYNTAX);
            return ret;
        }
        if (id_is_fmt_spec(next_token->repr))
            next_token->ttype = TOK_FMT_SPEC;
        if (next_token->ttype != TOK_FMT_SPEC) {
            handle_error(ERR_SYNTAX);
            return ret;
        }
        advance_lexer();
        ret->children[1] = build_leaf();
        if (next_token->ttype != TOK_EOL) {
            handle_error(ERR_SYNTAX);
            return ret;
        }
        return ret;
    }
    handle_error(ERR_SYNTAX);
    return ret;
}

node_t *read_and_parse(void) {
    init_lexer();
    return build_root();
}