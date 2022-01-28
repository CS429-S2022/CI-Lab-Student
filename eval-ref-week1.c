#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);

static void infer_type(node_t *nptr) {
    // check running status
    if (terminate || ignore_input) return;
    
    if (nptr == NULL) return;

    for (int i = 0; i < 3; ++i) {
        infer_type(nptr->children[i]);
    }

    switch (nptr->node_type) {
        case NT_INTERNAL:
            switch (nptr->tok) {
                // Week 2 TODO: Implement type inference for unary and ternary operators
                // Week 2 TODO: Implement error checking and handling.
                case TOK_IDENTITY:
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_PLUS:
                case TOK_BMINUS:
                case TOK_DIV:
                case TOK_MOD:
                case TOK_TIMES:
                case TOK_AND:
                case TOK_OR:
                    nptr->type = nptr->children[0]->type; 
                    break;
                case TOK_LT:
                case TOK_GT:
                case TOK_EQ:
                    nptr->type = BOOL_TYPE; 
                    break;
                default:
                    break;
            }
        case NT_LEAF:
            // Week 3 TODO: Implement tpye evaluation for variables
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
    // check running status
    if (terminate || ignore_input) return;

    if (nptr == NULL) return;
    
    // Week 2 TODO: You'll need to slightly change this to account for ternary operators.
    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    
    switch (nptr->node_type) {
        // Week 2 TODO: Implement unary operation evaluation.
        case NT_INTERNAL:
            if (is_unop(nptr->tok)) {
                switch (nptr->tok) {
                    default:
                        break;
                }
            }
            if (is_binop(nptr->tok)) {
                switch (nptr->tok) {
                    case TOK_PLUS:
                        if (nptr->type == STRING_TYPE) {
                            // Week 2 TODO: Implement string addition.
                            break;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival; break;
                    case TOK_BMINUS:
                        nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival; break;
                    case TOK_TIMES:
                        if (nptr->type == STRING_TYPE) {
                            // Week 2 TODO: Implement string multiplication.
                            break;
                        }
                        nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival; break;
                    case TOK_DIV:
                        // Week 2 TODO: Implement error checking for zero division
                        nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival; break;
                    case TOK_MOD:
                        // Week 2 TODO: Implement error checking for zero division
                        nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival; break;
                    case TOK_AND:
                        nptr->val.bval = nptr->children[0]->val.bval && nptr->children[1]->val.bval; break;
                    case TOK_OR:
                        nptr->val.bval = nptr->children[0]->val.bval || nptr->children[1]->val.bval; break;
                    case TOK_LT:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            // Week 2 TODO: Implement string comparison.
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival < nptr->children[1]->val.ival; break;
                    case TOK_GT:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            // Week 2 TODO: Implement string comparison.
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival > nptr->children[1]->val.ival; break;
                    case TOK_EQ:
                        if (nptr->children[0]->type == STRING_TYPE) {
                            // Week 2 TODO: Implement string comparison.
                            break;
                        }
                        nptr->val.bval = nptr->children[0]->val.ival == nptr->children[1]->val.ival; break;
                    default:
                        break;
                }
            }
            if (nptr->tok == TOK_QUESTION) {
                // Week 2 TODO: Implement ternary operations.
            }
            if (nptr->tok == TOK_IDENTITY) {
                if (nptr->type == STRING_TYPE) {
                    // Week 2 TODO: Make a copy of the string.
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

char *strrev(char *str) {
    return NULL;
}
