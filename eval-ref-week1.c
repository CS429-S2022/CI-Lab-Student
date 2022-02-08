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

    switch (nptr->node_type) {
        // For each week, you will also need to include error checking for each type.
        // Week 2 TODO: Extend type inference to handle operators on string types.
        // Week 3 TODO: Implement type evaluation for variables.
        // You will need to modify or extend some of the cases here.
        case NT_INTERNAL:
            switch (nptr->tok) {   
                // For reference, the identity (do nothing) operator is implemented for you.
                case TOK_IDENTITY:
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_PLUS:
                    if (nptr->children[0]->type != INT_TYPE || nptr->children[1]->type != INT_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type;
                    break;
                case TOK_BMINUS:
                case TOK_DIV:
                case TOK_MOD:
                    if (nptr->children[0]->type != INT_TYPE || nptr->children[1]->type != INT_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type;
                    break;
                case TOK_TIMES:
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
                    if (nptr->children[0]->type == BOOL_TYPE) {
                        handle_error(ERR_TYPE);
                        return;
                    }
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_NOT:
                    if (nptr->children[0]->type != BOOL_TYPE) {
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
            // Week 2 TODO: Extend evaluation to handle operators on string types.
            // You will need to modify some cases for this.
            if (is_unop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_UMINUS:
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
                        nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival; break;
                    case TOK_BMINUS:
                        nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival; break;
                    case TOK_TIMES:
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
                        nptr->val.bval = nptr->children[0]->val.ival < nptr->children[1]->val.ival; break;
                    case TOK_GT:
                        nptr->val.bval = nptr->children[0]->val.ival > nptr->children[1]->val.ival; break;
                    case TOK_EQ:
                        nptr->val.bval = nptr->children[0]->val.ival == nptr->children[1]->val.ival; break;
                    default:
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
                    nptr->val.ival = nptr->children[1]->val.ival;
                } else {
                    eval_node(nptr->children[2]);
                    if (terminate || ignore_input) return;
                    nptr->val.ival = nptr->children[2]->val.ival;
                }
            }
            if (nptr->tok == TOK_IDENTITY) {
                nptr->val.ival = nptr->children[0]->val.ival;
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
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    // Week 2 TODO: Implement copying and reversing the string.
    return NULL;
}

/* cleanup() - frees the space allocated to the AST
 * Parameter: The node to free.
 */
void cleanup(node_t *nptr) {
    // Week 2 TODO: Recursively free each node in the AST
    return;
}