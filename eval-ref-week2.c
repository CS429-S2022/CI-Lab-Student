#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);

static void infer_type(node_t *nptr) {
    if (nptr == NULL) return;

    for (int i = 0; i < 3; ++i) {
        infer_type(nptr->children[i]);
    }
    // check running status
    if (terminate || ignore_input) return;

    // Week 3 TODO: Implement type inference for variables.
    switch (nptr->node_type) {
        case NT_INTERNAL:
            switch (nptr->tok) {
                case TOK_PLUS:
                case TOK_BMINUS:
                case TOK_DIV:
                case TOK_MOD:
                    if (nptr->tok == TOK_PLUS && nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
                        nptr->type = nptr->children[0]->type;
                        break;
                    }
                    if (nptr->children[0]->type != INT_TYPE || nptr->children[1]->type != INT_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type;
                    break;
                case TOK_TIMES:
                    if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == INT_TYPE) {
                        nptr->type = STRING_TYPE;
                        break;
                    }
                    if (nptr->children[0]->type != INT_TYPE || nptr->children[1]->type != INT_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_AND:
                case TOK_OR:
                    if (nptr->children[0]->type != BOOL_TYPE || nptr->children[1]->type != BOOL_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_LT:
                case TOK_GT:
                case TOK_EQ:
                    if (nptr->children[0]->type != nptr->children[1]->type ||
                            nptr->children[0]->type == BOOL_TYPE || nptr->children[1]->type == BOOL_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = BOOL_TYPE; 
                    break;
                case TOK_UMINUS:
                case TOK_NOT:
                case TOK_IDENTITY:
                    if ((nptr->tok == TOK_UMINUS && nptr->children[0]->type == BOOL_TYPE) ||
                            (nptr->tok == TOK_NOT && nptr->children[0]->type != BOOL_TYPE)) {
                        handle_error(ERR_TYPE);
                        return;
                        }
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_QUESTION:
                    if (nptr->children[0]->type != BOOL_TYPE || (nptr->children[1]->type != nptr->children[2]->type)) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[1]->type; 
                    break;
                default:
                    logging(LOG_ERROR, "unsupported token type for internal node");
                    break;
            }
        case NT_LEAF:
            break;
        default:
            break;
    }
    return;
}

static void infer_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        infer_type(nptr->children[1]);
    } else {
        for (int i = 0; i < 3; ++i) {
            infer_type(nptr->children[i]);
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

static void eval_node(node_t *nptr) {
    if (nptr == NULL) return;
    
    // ternary expression has unique eval priority
    if (nptr->tok != TOK_QUESTION) {
        for (int i = 0; i < 2; ++i) {
            eval_node(nptr->children[i]);
        }
    }
    // check running status
    if (terminate || ignore_input) return;
    
    switch (nptr->node_type) {
        case NT_INTERNAL:
            if (is_unop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_UMINUS:
                        if (nptr->type == STRING_TYPE) {
                            (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
                            if (! nptr->val.sval) {
                                logging(LOG_FATAL, "failed to allocate string");
                                return;
                            }
                            strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                            strrev(nptr->val.sval);
                            break;
                        }
                        nptr->val.ival = - nptr->children[0]->val.ival; break;
                    case TOK_NOT:
                        nptr->val.bval = ! nptr->children[0]->val.bval; break;
                    default:
                        break;
                }
            }
            if (is_binop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_PLUS:
                        if (nptr->type == STRING_TYPE) {
                            nptr->val.sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + strlen(nptr->children[1]->val.sval) + 1);
                            if (! nptr->val.sval) {
                                logging(LOG_FATAL, "failed to allocate string");
                                return;
                            }
                            strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                            strcat(nptr->val.sval, nptr->children[1]->val.sval);
                            break;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival; break;
                    case TOK_BMINUS:
                        nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival; break;
                    case TOK_TIMES:
                        if (nptr->type == STRING_TYPE) {
                            // check string * negative integer
                            if (nptr->children[1]->val.ival < 0) {
                                handle_error(ERR_EVAL);
                                return;
                            }
                            nptr->val.sval = (char *) malloc(strlen(nptr->children[0]->val.sval) * nptr->children[1]->val.ival + 1);
                            if (! nptr->val.sval) {
                                logging(LOG_FATAL, "failed to allocate string");
                                return;
                            }
                            nptr->val.sval[0] = '\0';
                            for (int i = 0; i < nptr->children[1]->val.ival; ++i) {
                                strcat(nptr->val.sval, nptr->children[0]->val.sval);
                            }
                            break;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival; break;
                    case TOK_DIV:
                        // check divide by zero
                        if (nptr->children[1]->val.ival == 0) {
                            handle_error(ERR_EVAL);
                            return;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival; break;
                    case TOK_MOD:
                        // check divide by zero
                        if (nptr->children[1]->val.ival == 0) {
                            handle_error(ERR_EVAL);
                            return;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival; break;
                    case TOK_AND:
                        nptr->val.bval = nptr->children[0]->val.bval && nptr->children[1]->val.bval; break;
                    case TOK_OR:
                        nptr->val.bval = nptr->children[0]->val.bval || nptr->children[1]->val.bval; break;
                    case TOK_LT:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) < 0);
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival < nptr->children[1]->val.ival; break;
                    case TOK_GT:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) > 0);
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival > nptr->children[1]->val.ival; break;
                    case TOK_EQ:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            nptr->val.bval = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) == 0);
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival == nptr->children[1]->val.ival; break;
                    default:
                        logging(LOG_ERROR, "unsupported token type for binary operator");
                        break;
                }
            }
            if (nptr->tok == TOK_QUESTION) {
                // short circuit eval strategy
                eval_node(nptr->children[0]);
                if (terminate || ignore_input) return;

                if (nptr->children[0]->val.bval) {
                    eval_node(nptr->children[1]);
                    if (terminate || ignore_input) return;
                    if (nptr->children[1]->type == STRING_TYPE) {
                        nptr->val.sval = (char *) malloc(strlen(nptr->children[1]->val.sval) + 1);
                        if (! nptr->val.sval) {
                            logging(LOG_FATAL, "failed to allocate string");
                            return;
                        }
                        strcpy(nptr->val.sval, nptr->children[1]->val.sval);
                        break;
                    }
                    nptr->val.ival = nptr->children[1]->val.ival;
                } else {
                    eval_node(nptr->children[2]);
                    if (terminate || ignore_input) return;
                    if (nptr->children[2]->type == STRING_TYPE) {
                        nptr->val.sval = (char *) malloc(strlen(nptr->children[2]->val.sval) + 1);
                        if (! nptr->val.sval) {
                            logging(LOG_FATAL, "failed to allocate string");
                            return;
                        }
                        strcpy(nptr->val.sval, nptr->children[2]->val.sval);
                        break;
                    }
                    nptr->val.ival = nptr->children[2]->val.ival;
                }
            }
            if (nptr->tok == TOK_IDENTITY) {
                if (nptr->type == STRING_TYPE) {
                    (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
                    if (! nptr->val.sval) {
                        logging(LOG_FATAL, "failed to allocate string");
                        return;
                    }
                    strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                } else {
                    nptr->val.ival = nptr->children[0]->val.ival;
                }
            }
            break;
        case NT_LEAF:
            break;
        default:
            break;
    }
    return;
}

void eval_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input) return;
        
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input) return;
    
    if (nptr->type == STRING_TYPE) {
        (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (! nptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    } else {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

void infer_and_eval(node_t *nptr) {
    infer_root(nptr);
    eval_root(nptr);
    return;
}


/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 */
char *strrev(char *str) {
    char *p1, *p2;

    if (! str || ! *str)
        return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

/* cleanup() - frees the space allocated to the AST
 * Parameter: The node to free.
 * (STUDENT TODO)
 */
void cleanup(node_t *nptr) {
    // Week 3 TODO: Recursively free each node in the AST
    return;
}