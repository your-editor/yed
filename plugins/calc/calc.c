#include <yed/plugin.h>
#include <yed/syntax.h>
#include <math.h>

#define PROMPT     "CALC> "
#define PROMPT_LEN (6)

#define OP_ASSOC_LEFT  (1)
#define OP_ASSOC_RIGHT (2)

#define X_OPS                                                           \
    /* op              prec,    assoc,            arity,      str */    \
    X(OP_INVALID,      0,       0,                0,          NULL)     \
    X(OP_CALL,         13,      OP_ASSOC_LEFT,    2,          "(")      \
    X(OP_EXP,          12,      OP_ASSOC_LEFT,    2,          "**")     \
    X(OP_PLUS,         9,       OP_ASSOC_LEFT,    2,          "+")      \
    X(OP_MINUS,        9,       OP_ASSOC_LEFT,    2,          "-")      \
    X(OP_MULT,         10,      OP_ASSOC_LEFT,    2,          "*")      \
    X(OP_IDIV,         10,      OP_ASSOC_LEFT,    2,          "//")     \
    X(OP_DIV,          10,      OP_ASSOC_LEFT,    2,          "/")      \
    X(OP_MOD,          10,      OP_ASSOC_LEFT,    2,          "%")      \
    X(OP_LEQ,          7,       OP_ASSOC_LEFT,    2,          "<=")     \
    X(OP_GEQ,          7,       OP_ASSOC_LEFT,    2,          ">=")     \
    X(OP_LSS,          7,       OP_ASSOC_LEFT,    2,          "<")      \
    X(OP_GTR,          7,       OP_ASSOC_LEFT,    2,          ">")      \
    X(OP_EQU,          6,       OP_ASSOC_LEFT,    2,          "==")     \
    X(OP_NEQ,          6,       OP_ASSOC_LEFT,    2,          "!=")     \
    X(OP_BSHL,         8,       OP_ASSOC_LEFT,    2,          "<<")     \
    X(OP_BSHR,         8,       OP_ASSOC_LEFT,    2,          ">>")     \
    X(OP_BAND,         5,       OP_ASSOC_LEFT,    2,          "&")      \
    X(OP_BXOR,         4,       OP_ASSOC_LEFT,    2,          "^")      \
    X(OP_BOR,          3,       OP_ASSOC_LEFT,    2,          "|")      \
    X(OP_NEG,          11,      OP_ASSOC_RIGHT,   1,          "-")      \
    X(OP_BNEG,         11,      OP_ASSOC_RIGHT,   1,          "~")

enum {
#define X(_op, _prec, _assoc, _arity, _str) _op,
    X_OPS
#undef X
    N_OPS,
};

u32 op_prec_table[] = {
#define X(_op, _prec, _assoc, _arity, _str) _prec,
    X_OPS
#undef X
};
int op_assoc_table[] = {
#define X(_op, _prec, _assoc, _arity, _str) _assoc,
    X_OPS
#undef X
};
int op_arity_table[] = {
#define X(_op, _prec, _assoc, _arity, _str) _arity,
    X_OPS
#undef X
};
const char * op_str_table[] = {
#define X(_op, _prec, _assoc, _arity, _str) _str,
    X_OPS
#undef X
};

#define OP_PREC(_op)      (op_prec_table[(_op)])
#define OP_ASSOC(_op)     (op_assoc_table[(_op)])
#define OP_IS_UNARY(_op)  (op_arity_table[(_op)] == 1)
#define OP_IS_BINARY(_op) (op_arity_table[(_op)] == 2)
#define OP_STR(_op)       (op_str_table[(_op)])
#define OP_STRLEN(_op)    (strlen(OP_STR((_op))))
#define ASSIGNMENT_PREC   (OP_PREC(OP_ASSIGN))
#define HIGHEST_BIN_PREC  (13)

enum {
    TYPE_UNKNOWN,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VAR,
};

enum {
    UNIT_UNKNOWN,

    N_UNITS,
};

typedef struct {
    union {
        i64     i;
        double  f;
        char   *name;
    };
    u32        type;
    u32        unit_id;
} calc_val_t;

enum {
    EXPR_UNKNOWN,
    EXPR_VAL,
    EXPR_OP,
};

typedef struct calc_expr {
    u32                           type;
    union {
        calc_val_t                val;
        struct {
            union {
                struct calc_expr *left;
                struct calc_expr *child;
            };
            struct calc_expr     *right;
            u32                   op;
        } op;
    };
    u32                           str_off;
} calc_expr_t;


typedef calc_val_t (*convert_t)(calc_val_t, u32 to_unit);

#include <yed/tree.h>
typedef char *varname_t;
use_tree_c(varname_t, calc_val_t, strcmp);



static convert_t                    converters[N_UNITS][N_UNITS];
static tree(varname_t, calc_val_t)  vars;
static char                        *start;
static char                        *str;
static char                        *last;
static int                          success = 1;
static char                         err_buff[512];
static int                          err_off;
static calc_val_t                   last_val;

#define CLEAR_ERR()  \
do {                 \
    err_buff[0] = 0; \
} while (0)

#define SET_ERR(_fmt, ...)                                         \
do {                                                               \
    if (strlen(err_buff) == 0) {                                   \
        snprintf(err_buff, sizeof(err_buff), _fmt, ##__VA_ARGS__); \
        err_off = str - start;                                     \
    }                                                              \
} while (0)

#define SET_ERR_AT(_off, _fmt, ...)                                \
do {                                                               \
    if (strlen(err_buff) == 0) {                                   \
        snprintf(err_buff, sizeof(err_buff), _fmt, ##__VA_ARGS__); \
        err_off = (_off);                                          \
    }                                                              \
} while (0)

static calc_expr_t *expr_val(calc_val_t val, u32 str_off) {
    calc_expr_t *expr;

    expr = malloc(sizeof(*expr));

    memset(expr, 0, sizeof(*expr));

    expr->type    = EXPR_VAL;
    expr->val     = val;
    expr->str_off = str_off;

    return expr;
}

static calc_expr_t *expr_unary(u32 op, calc_expr_t *child, u32 str_off) {
    calc_expr_t *expr;

    expr = malloc(sizeof(*expr));

    memset(expr, 0, sizeof(*expr));

    expr->type     = EXPR_OP;
    expr->op.child = child;
    expr->op.op    = op;
    expr->str_off  = str_off;

    return expr;
}

static calc_expr_t *expr_binary(u32 op, calc_expr_t *left, calc_expr_t *right, u32 str_off) {
    calc_expr_t *expr;

    expr = malloc(sizeof(*expr));

    memset(expr, 0, sizeof(*expr));

    expr->type     = EXPR_OP;
    expr->op.left  = left;
    expr->op.right = right;
    expr->op.op    = op;
    expr->str_off  = str_off;

    return expr;
}

static void free_expr(calc_expr_t *expr) {
    switch (expr->type) {
        case EXPR_VAL:
            if (expr->val.type == TYPE_VAR
            &&  expr->val.name != NULL) {
                free(expr->val.name);
            }
            free(expr);
            break;
        case EXPR_OP:
            if (expr->op.left  != NULL) { free_expr(expr->op.left);  }
            if (expr->op.right != NULL) { free_expr(expr->op.right); }
            free(expr);
            break;
    }
}


#define IS_SPACE(c)               (((c) >= 9 && (c) <= 13) || (c) == 32)
#define IS_ALPHA(c)               (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_NUM(c)                 ((c) >= '0' && (c) <= '9')
#define IS_IDENT_CHAR(c)          ((c) == '_' || IS_ALPHA(c) || IS_NUM(c))
#define IS_IN_RANGE(c, low, high) ((c) >= (low) && (c) <= (high))

#define PEEK(_search)       (strncmp(str, (_search), strlen((_search))) == 0)

#define EXPECT(_search, ...)                 \
do {                                         \
    if (!PEEK((_search))) {                  \
        SET_ERR("expected '%s'", (_search)); \
        __VA_ARGS__;                         \
    }                                        \
} while (0)

#define EAT(_len)                      \
do {                                   \
    str += (_len);                     \
    while (*str && IS_SPACE(*str)) {   \
        str += 1;                      \
    }                                  \
} while (0)

static u32 lookahead_binary_op(void) {
    u32 op;

    op = OP_INVALID;

    /*
    ** The order of these is sensative to parsing.
    ** e.g.
    **    << will parse as < if < is parsed first.
    ** So you have to check for << explicitly before <.
    ** (Obviously applies to other operators too.)
    */
         if (PEEK(OP_STR(OP_CALL)))  { op = OP_CALL;  }
    else if (PEEK(OP_STR(OP_EXP)))   { op = OP_EXP;   }
    else if (PEEK(OP_STR(OP_EQU)))   { op = OP_EQU;   }
    else if (PEEK(OP_STR(OP_NEQ)))   { op = OP_NEQ;   }
    else if (PEEK(OP_STR(OP_PLUS)))  { op = OP_PLUS;  }
    else if (PEEK(OP_STR(OP_MINUS))) { op = OP_MINUS; }
    else if (PEEK(OP_STR(OP_MULT)))  { op = OP_MULT;  }
    else if (PEEK(OP_STR(OP_IDIV)))  { op = OP_IDIV;  }
    else if (PEEK(OP_STR(OP_DIV)))   { op = OP_DIV;   }
    else if (PEEK(OP_STR(OP_MOD)))   { op = OP_MOD;   }
    else if (PEEK(OP_STR(OP_BSHL)))  { op = OP_BSHL;  }
    else if (PEEK(OP_STR(OP_BSHR)))  { op = OP_BSHR;  }
    else if (PEEK(OP_STR(OP_LEQ)))   { op = OP_LEQ;   }
    else if (PEEK(OP_STR(OP_GEQ)))   { op = OP_GEQ;   }
    else if (PEEK(OP_STR(OP_LSS)))   { op = OP_LSS;   }
    else if (PEEK(OP_STR(OP_GTR)))   { op = OP_GTR;   }
    else if (PEEK(OP_STR(OP_BXOR)))  { op = OP_BXOR;  }
    else if (PEEK(OP_STR(OP_BAND)))  { op = OP_BAND;  }
    else if (PEEK(OP_STR(OP_BOR)))   { op = OP_BOR;   }

    return op;
}

static u32 lookahead_unary_prefix_op(void) {
    u32 op;

    op = OP_INVALID;

         if (PEEK(OP_STR(OP_NEG)))  { op = OP_NEG;  }
    else if (PEEK(OP_STR(OP_BNEG))) { op = OP_BNEG; }

    return op;
}

static int parse_name(char *buff) {
    int  has_alnum;
    int  len;
    char c;

    has_alnum = 0;
    len       = 0;

    if (*str
    &&  ((c = *(str)), (c == '_' || IS_ALPHA(c)))) {
        if (c != '_') {
            has_alnum = 1;
        }
        len += 1;
    } else {
        return 0;
    }

    while (*(str + len)
    &&     IS_IDENT_CHAR((c = *(str + len)))) {
        if (c != '_') {
            has_alnum = 1;
        }
        len += 1;
    }

    if (!has_alnum) {
        if (len != 1 || *str != '_') {
            return 0;
        }
    }

    if (len > 0) {
        memcpy(buff, str, len);
        buff[len] = 0;
    }

    return len;
}

static calc_expr_t * parse_expr(void);

static calc_expr_t * parse_leaf_expr(void) {
    calc_expr_t *result;
    u32          str_off;
    calc_val_t   val;
    char         buff[256];
    char        *end;
    const char  *scan;

    result  = NULL;
    str_off = str - start;

    memset(&val, 0, sizeof(val));

    if (PEEK("(")) {
        EAT(1);
        if (PEEK(")")) {
            SET_ERR("empty set of parentheses");
            return NULL;
        }
        result = parse_expr();
        if (result == NULL) {
            SET_ERR("expected valid expression after '('");
            return NULL;
        }
        EXPECT(")", return NULL);
        EAT(1);
    } else {
        val.f = strtod(str, &end);
        if (end > str) {
            for (scan = str; scan <= end; scan += 1) {
                if (*scan == '.') {
                    val.type = TYPE_FLOAT;
                    break;
                }
            }
            if (val.type == TYPE_UNKNOWN) {
                val.type = TYPE_INT;
                val.i    = (i64)val.f;
            }
            EAT(end - str);
        } else {
            val.i = strtoll(str, &end, 0);
            if (end > str) {
                val.type = TYPE_INT;
                EAT(end - str);
            } else {
                if (parse_name(buff)) {
                    EAT(strlen(buff));
                    val.type = TYPE_VAR;
                    val.name = strdup(buff);
                } else {
                    return NULL;
                }
            }
        }

        result = expr_val(val, str_off);
    }

    return result;
}

static calc_expr_t * parse_expr_more(calc_expr_t *left, int min_prec);

static calc_expr_t * parse_operand(void) {
    u32          str_off;
    u32          op;
    calc_expr_t *child;

    str_off = str - start;
    op      = lookahead_unary_prefix_op();

    if (op == OP_INVALID) { return parse_leaf_expr(); }

    EAT(strlen(OP_STR(op)));

    if ((child = parse_operand()) == NULL) {
        SET_ERR("invalid operand to unary '%s' expression", OP_STR(op));
        return NULL;
    }

    return expr_unary(op, child, str_off);
}

static calc_expr_t * parse_expr_more(calc_expr_t *left, int min_prec) {
    /*
    ** based on algorithm found here:
    ** https://en.wikipedia.org/wiki/Operator-precedence_parser#Pseudo-code
    */

    calc_expr_t *right;
    u32          str_off;
    int          op;
    int          op_prec;
    int          lookahead_op;

    right = NULL;

    str_off = str - start;

    /*
    ** lookahead := peek next token
    ** while lookahead is a binary operator whose precedence is >= min_precedence
    **     op := lookahead
    */

    op = OP_INVALID;

    while ((op = lookahead_binary_op()) != OP_INVALID
    &&     OP_PREC(op) >= min_prec) {
        /*
        ** advance to next token
        ** rhs := parse_primary()
        */

        str_off = str - start;

        EAT(strlen(OP_STR(op)));

        if (op == OP_CALL) {
            if (PEEK(")")) {
                right = NULL;
            } else {
                right = parse_expr();
                if (right == NULL) {
                    SET_ERR("expected valid expression as function argument");
                    return NULL;
                }
                EXPECT(")", return NULL);
            }
            EAT(1);
        } else {
            if (OP_IS_BINARY(op)) {
                right = parse_operand();
                if (right == NULL) {
                    SET_ERR("missing operand to binary '%s' expression", OP_STR(op));
                    return NULL;
                }
            }
        }

        op_prec = OP_PREC(op);

        /*
        ** lookahead := peek next token
        ** while lookahead is a binary operator whose
        ** precedence is greater than op's, or a
        ** right-associative operator whose precedence is
        ** equal to op's
        */

        lookahead_op = OP_INVALID;

        while ((((lookahead_op = lookahead_binary_op()) != OP_INVALID)
            && (OP_PREC(lookahead_op) > op_prec))
        ||     ((OP_ASSOC(lookahead_op) == OP_ASSOC_RIGHT)
            && (OP_PREC(lookahead_op) == op_prec))) {

            right = parse_expr_more(right, OP_PREC(lookahead_op));
            if (right == NULL) {
                SET_ERR("missing operand to binary '%s' expression", OP_STR(lookahead_op));
                return NULL;
            }
        }

        /* lhs := the result of applying op with operands lhs and rhs */

        left = expr_binary(op, left, right, str_off);
    }

    return left;
}

static calc_expr_t * parse_expr_prec(int min_prec) {
    calc_expr_t *operand;

    operand = parse_operand();
    if (operand == NULL) { return NULL; }

    return parse_expr_more(operand, min_prec);
}

static calc_expr_t * parse_expr(void) { return parse_expr_prec(0); }

static calc_expr_t * parse_top_level_expr(void) {
    calc_expr_t *expr;

    expr = parse_expr();

    if (*str) {
        SET_ERR("unexpected '%s'", str);
        if (expr != NULL) {
            free_expr(expr);
            expr = NULL;
        }
    }

    if (expr == NULL) {
        SET_ERR("expected expression");
    }

    return expr;
}


static void set_var(char *name, calc_val_t val) {
    tree_it(varname_t, calc_val_t) it;

    it = tree_lookup(vars, name);

    if (tree_it_good(it)) {
        tree_it_val(it) = val;
    } else {
        tree_insert(vars, strdup(name), val);
    }
}

static void set_float_var(char *name, double f) {
    calc_val_t val;

    val.f       = f;
    val.type    = TYPE_FLOAT;
    val.unit_id = UNIT_UNKNOWN;

    set_var(name, val);
}

static void set_int_var(char *name, i64 i) {
    calc_val_t val;

    val.i       = i;
    val.type    = TYPE_INT;
    val.unit_id = UNIT_UNKNOWN;

    set_var(name, val);
}



static int eval_expr(calc_expr_t *expr, calc_val_t *val) {
    tree_it(varname_t, calc_val_t)   it;
    double                         (*fn)(double);
    calc_val_t                       lval;
    calc_val_t                       rval;
    i64                              li;
    i64                              ri;
    double                           lf;
    double                           rf;
    u32                              result_type;
    int                              is_compare;
    u32                              result_unit_id;
    i64                              result_i;
    double                           result_f;
    int                              cmp_i;
    int                              cmp_f;
    int                              i;

    switch (expr->type) {
        case EXPR_VAL:
            if (expr->val.type == TYPE_VAR) {
                it = tree_lookup(vars, expr->val.name);
                if (!tree_it_good(it)) {
                    SET_ERR_AT(expr->str_off, "no such variable named '%s'", expr->val.name);
                    return 0;
                }
                *val = tree_it_val(it);
            } else {
                *val = expr->val;
            }

            return 1;

        case EXPR_OP:
            if (expr->op.op == OP_CALL) {
                if (expr->op.left->type != EXPR_VAL) {
                    SET_ERR_AT(expr->op.left->str_off,
                               "attempting to call an arithmetic expression as if it were a function");
                    return 0;
                }
                if (expr->op.left->val.type != TYPE_VAR) {
                    SET_ERR_AT(expr->op.left->str_off,
                               "attempting to call %s as if it were a function",
                               expr->op.left->val.type == TYPE_INT ? "an integer" : "a float");
                    return 0;
                }

                fn = NULL;

                if      (strcmp("sin",   expr->op.left->val.name) == 0) { fn = sin;   }
                else if (strcmp("cos",   expr->op.left->val.name) == 0) { fn = cos;   }
                else if (strcmp("acos",  expr->op.left->val.name) == 0) { fn = acos;  }
                else if (strcmp("asin",  expr->op.left->val.name) == 0) { fn = asin;  }
                else if (strcmp("atan",  expr->op.left->val.name) == 0) { fn = atan;  }
                else if (strcmp("cos",   expr->op.left->val.name) == 0) { fn = cos;   }
                else if (strcmp("cosh",  expr->op.left->val.name) == 0) { fn = cosh;  }
                else if (strcmp("sin",   expr->op.left->val.name) == 0) { fn = sin;   }
                else if (strcmp("sinh",  expr->op.left->val.name) == 0) { fn = sinh;  }
                else if (strcmp("tanh",  expr->op.left->val.name) == 0) { fn = tanh;  }
                else if (strcmp("exp",   expr->op.left->val.name) == 0) { fn = exp;   }
                else if (strcmp("log",   expr->op.left->val.name) == 0) { fn = log;   }
                else if (strcmp("log2",  expr->op.left->val.name) == 0) { fn = log2;  }
                else if (strcmp("log10", expr->op.left->val.name) == 0) { fn = log10; }
                else if (strcmp("sqrt",  expr->op.left->val.name) == 0) { fn = sqrt;  }
                else if (strcmp("ceil",  expr->op.left->val.name) == 0) { fn = ceil;  }
                else if (strcmp("fabs",  expr->op.left->val.name) == 0) { fn = fabs;  }
                else if (strcmp("floor", expr->op.left->val.name) == 0) { fn = floor; }

                if (fn == NULL) {
                    SET_ERR_AT(expr->op.left->str_off,
                               "unknown function '%s'", expr->op.left->val.name);
                    return 0;
                }

                if (expr->op.right == NULL) {
                    SET_ERR_AT(expr->str_off, "missing argument to function '%s'", expr->op.left->val.name);
                    return 0;
                }

                if (!eval_expr(expr->op.right, &rval)) { return 0; }

                if (rval.type == TYPE_INT) {
                    ri = rval.i;
                    rf = (double)ri;
                } else {
                    ri = (i64)rval.f;
                    rf = rval.f;
                }

                val->f = fn(rf);

                if (rval.type == TYPE_INT
                &&  ceil(val->f) == val->f) {

                    val->i      = val->f;
                    result_type = TYPE_INT;
                } else {
                    result_type = TYPE_FLOAT;
                }

                val->type    = result_type;
                val->unit_id = rval.unit_id;

                return 1;
            }

            if (OP_IS_BINARY(expr->op.op)) {
                if (!eval_expr(expr->op.left,  &lval)) { return 0; }
                if (!eval_expr(expr->op.right, &rval)) { return 0; }
            } else if (OP_IS_UNARY(expr->op.op)) {
                if (!eval_expr(expr->op.child, &rval)) { return 0; }
            }

            if (lval.type == TYPE_INT) {
                li = lval.i;
                lf = (double)li;
            } else {
                li = (i64)lval.f;
                lf = lval.f;
            }

            if (rval.type == TYPE_INT) {
                ri = rval.i;
                rf = (double)ri;
            } else {
                ri = (i64)rval.f;
                rf = rval.f;
            }

            result_type = (lval.type == TYPE_FLOAT || rval.type == TYPE_FLOAT)
                            ? TYPE_FLOAT
                            : TYPE_INT;

            if (expr->op.op == OP_DIV || expr->op.op == OP_IDIV) {
                if ((rval.type == TYPE_INT && ri == 0) || rf == 0.0) {
                    SET_ERR_AT(expr->str_off, "divide by zero");
                    return 0;
                }
            }

            if (expr->op.op == OP_DIV
            &&  result_type == TYPE_INT
            &&  li % ri     != 0) {

                result_type = TYPE_FLOAT;
            }

            is_compare = 0;

            switch (expr->op.op) {
                case OP_LEQ:
                case OP_GEQ:
                case OP_LSS:
                case OP_GTR:
                case OP_EQU:
                case OP_NEQ:
                    is_compare = 1;
                    break;

                case OP_IDIV:
                case OP_BSHL:
                case OP_BSHR:
                case OP_BAND:
                case OP_BXOR:
                case OP_BOR:
                case OP_BNEG:
                    if (result_type == TYPE_FLOAT) {
                        SET_ERR_AT(expr->str_off, "operator '%s' invalid for floats", OP_STR(expr->op.op));
                        return 0;
                    }
                    break;
            }


            if (lval.unit_id == rval.unit_id) {
                result_unit_id = lval.unit_id;
            } else if (lval.unit_id == UNIT_UNKNOWN) {
                result_unit_id = rval.unit_id;
            } else if (rval.unit_id == UNIT_UNKNOWN) {
                result_unit_id = lval.unit_id;
            } else {
                SET_ERR_AT(expr->str_off, "operator '%s' does not apply to these units", OP_STR(expr->op.op));
                return 0;
            }

            switch (expr->op.op) {
                case OP_NEG:   result_i =     -ri;   result_f =     -rf;      break;
                case OP_PLUS:  result_i = li + ri;   result_f = lf + rf;      break;
                case OP_MINUS: result_i = li - ri;   result_f = lf - rf;      break;
                case OP_MULT:  result_i = li * ri;   result_f = lf * rf;      break;
                case OP_DIV:   result_i = li / ri;   result_f = lf / rf;      break;
                case OP_IDIV:  result_i = li / ri;                            break;
                case OP_MOD:   result_i = li % ri;   result_f = fmod(lf, rf); break;
                case OP_LEQ:   cmp_i    = li <= ri;  cmp_f    = lf <= rf;     break;
                case OP_GEQ:   cmp_i    = li >= ri;  cmp_f    = lf >= rf;     break;
                case OP_LSS:   cmp_i    = li <  ri;  cmp_f    = lf <  rf;     break;
                case OP_GTR:   cmp_i    = li >  ri;  cmp_f    = lf >  rf;     break;
                case OP_EQU:   cmp_i    = li == ri;  cmp_f    = lf == rf;     break;
                case OP_NEQ:   cmp_i    = li != ri;  cmp_f    = lf != rf;     break;
                case OP_BSHL:  result_i = li << ri;                           break;
                case OP_BSHR:  result_i = li >> ri;                           break;
                case OP_BAND:  result_i = li &  ri;                           break;
                case OP_BXOR:  result_i = li ^ ri;                            break;
                case OP_BOR:   result_i = li | ri;                            break;
                case OP_BNEG:  result_i =     ~ri;                            break;

                case OP_EXP:
                    if (result_type == TYPE_INT) {
                        result_i = 1;
                        for (i = 0; i < ri; i += 1) { result_i *= li; }
                    } else {
                        result_f = pow(lf, rf);
                    }
                    break;

                default:
                    SET_ERR_AT(expr->str_off, "unimplemented/unknown operator '%s'", OP_STR(expr->op.op));
                    return 0;
            }

            if (is_compare) {
                val->type = TYPE_INT;
                if (result_type == TYPE_INT) { val->i = cmp_i; }
                else                         { val->i = cmp_f; }
                val->unit_id = UNIT_UNKNOWN;
            } else {
                val->type = result_type;
                if (result_type == TYPE_INT) { val->i = result_i; }
                else                         { val->f = result_f; }
                val->unit_id = result_unit_id;
            }

            return 1;

        case EXPR_UNKNOWN:
            SET_ERR("unknown expr type");
            return 0;

    }

    return 0;
}


static yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*calc");

    if (buff == NULL) {
        buff = yed_create_buffer("*calc");
        buff->flags |= BUFF_SPECIAL;
        yed_buffer_add_line(buff);
        yed_buff_insert_string(buff, PROMPT, 1, 1);
    }

    return buff;
}

#define CHECK_EVENT_FRAME_AND_BUFFER(_evt)            \
do {                                                  \
    if (event->frame         == NULL                  \
    ||  event->frame->buffer == NULL                  \
    ||  event->frame->buffer != get_or_make_buff()) { \
        return;                                       \
    }                                                 \
} while (0)

static int cursor_move_handler_recursing;
static int save_cursor_col = 1;

static void cursor_pre_move_handler(yed_event *event) {
    yed_frame *f;

    if (cursor_move_handler_recursing) { return; }

    CHECK_EVENT_FRAME_AND_BUFFER(event);

    f = event->frame;

    save_cursor_col = f->cursor_col;
}

static void cursor_post_move_handler(yed_event *event) {
    yed_frame *f;

    if (cursor_move_handler_recursing) { return; }

    CHECK_EVENT_FRAME_AND_BUFFER(event);

    f = event->frame;


    if (f->cursor_line != 1) {
        get_or_make_buff()->flags |= BUFF_RD_ONLY;
    } else {
        get_or_make_buff()->flags &= ~BUFF_RD_ONLY;

        if (f->cursor_col <= PROMPT_LEN) {
            cursor_move_handler_recursing = 1;
            yed_set_cursor_within_frame(f, 1, PROMPT_LEN + 1);
            cursor_move_handler_recursing = 0;
        }
    }

}


static int disable_buffer_mod_handler = 0;

static void buffer_pre_mod_handler(yed_event *event) {
    if (disable_buffer_mod_handler
    ||  event->buffer != get_or_make_buff()) {
        return;
    }

    if (event->buff_mod_event == BUFF_MOD_ADD_LINE
    ||  event->buff_mod_event == BUFF_MOD_INSERT_LINE
    ||  event->buff_mod_event == BUFF_MOD_DELETE_LINE
    ||  event->buff_mod_event == BUFF_MOD_CLEAR_LINE
    ||  event->buff_mod_event == BUFF_MOD_SET_LINE
    ||  event->buff_mod_event == BUFF_MOD_CLEAR) {
        event->cancel = 1;
    }

    if (event->buff_mod_event == BUFF_MOD_INSERT_INTO_LINE
    ||  event->buff_mod_event == BUFF_MOD_APPEND_TO_LINE
    ||  event->buff_mod_event == BUFF_MOD_DELETE_FROM_LINE
    ||  event->buff_mod_event == BUFF_MOD_POP_FROM_LINE) {
        if (event->row != 1 || event->col <= PROMPT_LEN) {
            event->cancel = 1;
        }
    }
}

static void buffer_post_mod_handler(yed_event *event) {
    yed_line    *line;
    char         name_buff[256];
    int          var_assign;
    calc_expr_t *expr;
    calc_val_t   val;
    char         line_buff[512];
    int          i;

    if (disable_buffer_mod_handler
    ||  ys->active_frame == NULL
    ||  ys->active_frame->buffer == NULL
    ||  ys->active_frame->buffer != event->buffer
    ||  event->buffer != get_or_make_buff()) {
        return;
    }

    if (ys->active_frame->cursor_line == 1) {
        disable_buffer_mod_handler = 1;

        if (start != NULL) {
            free(start);
            start = str = NULL;
        }

        line = yed_buff_get_line(get_or_make_buff(), 1);
        array_zero_term(line->chars);
        start = str = strdup(array_data(line->chars) + PROMPT_LEN);

        EAT(0);

        CLEAR_ERR();

        if (strlen(str) > 0) {
            var_assign = 0;
            if (parse_name(name_buff)) {
                EAT(strlen(name_buff));
                if (PEEK("=")) {
                    EAT(1);
                    var_assign = 1;
                } else {
                    str = start;
                }
            }

            expr = parse_top_level_expr();

            if (expr != NULL) {
                if (!eval_expr(expr, &val)) { goto error; }

                last_val = val;

                if (val.type == TYPE_INT) {
                    sprintf(line_buff, "ans: %ld, 0x%lx", (long)val.i, (unsigned long)val.i);
                } else if (val.type == TYPE_FLOAT) {
                    sprintf(line_buff, "ans: %f", val.f);
                }

                if (var_assign) {
                    if (val.type == TYPE_INT) {
                        set_int_var(name_buff, val.i);
                    } else if (val.type == TYPE_FLOAT) {
                        set_float_var(name_buff, val.f);
                    }
                }

                yed_line_clear_no_undo(get_or_make_buff(), 2);
                yed_buff_insert_string_no_undo(get_or_make_buff(), line_buff, 2, 1);
                free_expr(expr);
                success = 1;
            } else {
error:;
                yed_line_clear_no_undo(get_or_make_buff(), 2);
                strcpy(line_buff, "error:");
                for (i = 0; i < (PROMPT_LEN + err_off) - strlen("error:"); i += 1) { strcat(line_buff, " "); }
                strcat(line_buff, "^ ");
                strcat(line_buff, err_buff);
                yed_buff_insert_string_no_undo(get_or_make_buff(), line_buff, 2, 1);
                success = 0;
            }
        } else {
            yed_line_clear_no_undo(get_or_make_buff(), 2);
            success = 1;
        }

        if (last != NULL) { free(last); }
        last = strdup(start);

        disable_buffer_mod_handler = 0;
    }
}

static void key_pressed_handler(yed_event *event) {
    char name[32];
    char buff[512];
    int  ans;

    if (ys->active_frame == NULL
    ||  ys->interactive_command
    ||  ys->active_frame->cursor_line != 1
    ||  ys->active_frame->buffer == NULL
    ||  ys->active_frame->buffer != get_or_make_buff()) {
        return;
    }

    if (event->key == ENTER) {
        if (last && strlen(last) > 0 && success) {
            disable_buffer_mod_handler = 1;
            yed_buff_insert_line_no_undo(get_or_make_buff(), 3);
            ans = yed_buff_n_lines(get_or_make_buff()) - 2;

            buff[0] = 0;
            snprintf(name, sizeof(name), "ans_%d", ans);
            if (last_val.type == TYPE_INT) {
                snprintf(buff, sizeof(buff), "%s:%s  %s  = %ld, 0x%lx", name, ans < 10 ? " " : "", last, (long)last_val.i, (unsigned long)last_val.i);
                set_int_var(name, last_val.i);
            } else if (last_val.type == TYPE_FLOAT) {
                snprintf(buff, sizeof(buff), "%s:%s  %s  = %f", name, ans < 10 ? " " : "", last, last_val.f);
                set_float_var(name, last_val.f);
            }

            yed_buff_insert_string_no_undo(get_or_make_buff(), buff, 3, 1);
            yed_set_cursor_within_frame(ys->active_frame, 1, 1);
            yed_line_clear_no_undo(get_or_make_buff(), 1);
            yed_buff_insert_string(get_or_make_buff(), PROMPT, 1, 1);
            yed_set_cursor_within_frame(ys->active_frame, 1, 1);
            yed_line_clear_no_undo(get_or_make_buff(), 2);
            free(last);
            last = NULL;
            disable_buffer_mod_handler = 0;
        }
        event->cancel = 1;
    }
}

static void line_pre_draw_handler(yed_event *event) {
    yed_attrs                       a;
    yed_attrs                      *ait;
    tree_it(varname_t, calc_val_t)  it;

    if (event->frame         == NULL
    ||  event->frame->buffer == NULL
    ||  event->frame->buffer != get_or_make_buff()) {
        return;
    }

    if (event->row == 2 && start != NULL && !success) {
        a = yed_active_style_get_attention();
        array_traverse(event->line_attrs, ait) {
            yed_combine_attrs(ait, &a);
        }
    }

#ifdef __APPLE__
#define WB "[[:>:]]"
#else
#define WB "\\b"
#endif

    yed_syntax syn;

    yed_syntax_start(&syn);
        yed_syntax_attr_push(&syn, "&code-number");
            yed_syntax_regex_sub(&syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?)"WB, 2);
            yed_syntax_regex_sub(&syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
            yed_syntax_regex_sub(&syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+)"WB, 2);
        yed_syntax_attr_pop(&syn);

        yed_syntax_attr_push(&syn, "&code-fn-call");
            yed_syntax_regex_sub(&syn, "([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
        yed_syntax_attr_pop(&syn);

        yed_syntax_attr_push(&syn, "&code-keyword");
            tree_traverse(vars, it) {
                yed_syntax_kwd(&syn, tree_it_key(it));
            }
        yed_syntax_attr_pop(&syn);
    yed_syntax_end(&syn);

    yed_syntax_line_event(&syn, event);

    yed_syntax_free(&syn);
}

static void free_vars(void) {
    tree_it(varname_t, calc_val_t)  it;
    char                           *key;

    while (tree_len(vars)) {
        it  = tree_begin(vars);
        key = tree_it_key(it);
        tree_delete(vars, key);
        free(key);
    }

    tree_free(vars);
}

static void unload(yed_plugin *self) {
    yed_buffer *buff;

    buff = yed_get_buffer("*calc");
    if (buff != NULL) {
        yed_free_buffer(buff);
    }

    free_vars();

    if (start != NULL) {
        free(start);
        start = str = NULL;
    }
}

void calc_open(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    get_or_make_buff();
    YEXE("special-buffer-prepare-focus", "*calc");
    YEXE("buffer", "*calc");
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    vars = tree_make(varname_t, calc_val_t);

    set_float_var("pi", M_PI);
    set_float_var("e",  M_E);
    set_int_var("page_size", 4096);
    set_int_var("NULL", 0);

    get_or_make_buff();

    yed_plugin_set_command(self, "calc", calc_open);

    h.kind = EVENT_CURSOR_PRE_MOVE;
    h.fn   = cursor_pre_move_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_CURSOR_POST_MOVE;
    h.fn   = cursor_post_move_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_BUFFER_PRE_MOD;
    h.fn   = buffer_pre_mod_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_BUFFER_POST_MOD;
    h.fn   = buffer_post_mod_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = key_pressed_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = line_pre_draw_handler;
    yed_plugin_add_event_handler(self, h);

    (void)converters;

    return 0;
}
