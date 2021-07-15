#include <yed/plugin.h>

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

#define OP_ASSOC_LEFT  (1)
#define OP_ASSOC_RIGHT (2)

#define X_OPS                                                           \
    /* op              prec,    assoc,            arity,      str */    \
    X(OP_INVALID,      0,       0,                0,          NULL)     \
    X(OP_CALL,         12,      OP_ASSOC_LEFT,    2,          "(")      \
    X(OP_PLUS,         9,       OP_ASSOC_LEFT,    2,          "+")      \
    X(OP_MINUS,        9,       OP_ASSOC_LEFT,    2,          "-")      \
    X(OP_MULT,         10,      OP_ASSOC_LEFT,    2,          "*")      \
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
#define HIGHEST_BIN_PREC  (12)

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
} calc_expr_t;


typedef calc_val_t (*convert_t)(calc_val_t, u32 to_unit);

#define inline static inline
#include <yed/tree.h>
typedef char *varname_t;
use_tree(varname_t, calc_val_t);
#undef inline



convert_t converters[N_UNITS][N_UNITS];
tree(varname_t, calc_val_t) vars;
const char *str;


static calc_expr_t *expr_val(calc_val_t val) {
    calc_expr_t *expr;

    expr       = malloc(sizeof(*expr));
    expr->type = EXPR_VAL;
    expr->val  = val;

    return expr;
}

static calc_expr_t *expr_unary(u32 op, calc_expr_t *child) {
    calc_expr_t *expr;

    expr           = malloc(sizeof(*expr));
    expr->type     = EXPR_OP;
    expr->op.child = child;
    expr->op.op    = op;

    return expr;
}

static calc_expr_t *expr_binary(u32 op, calc_expr_t *left, calc_expr_t *right) {
    calc_expr_t *expr;

    expr           = malloc(sizeof(*expr));
    expr->type     = EXPR_OP;
    expr->op.left  = left;
    expr->op.right = right;
    expr->op.op    = op;

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

#define EXPECT(_search, ...)           \
do {                                   \
    if (!PEEK((_search))) {            \
        /* @error */                   \
        __VA_ARGS__;                   \
    }                                  \
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
    else if (PEEK(OP_STR(OP_EQU)))   { op = OP_EQU;   }
    else if (PEEK(OP_STR(OP_NEQ)))   { op = OP_NEQ;   }
    else if (PEEK(OP_STR(OP_PLUS)))  { op = OP_PLUS;  }
    else if (PEEK(OP_STR(OP_MINUS))) { op = OP_MINUS; }
    else if (PEEK(OP_STR(OP_MULT)))  { op = OP_MULT;  }
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
    }

    return len;
}

static calc_expr_t * parse_expr(void);

static calc_expr_t * parse_leaf_expr(void) {
    calc_expr_t *result;
    calc_val_t   val;
    char         buff[256];
    char        *end;
    const char  *scan;

    result = NULL;

    memset(&val, 0, sizeof(val));

    if (PEEK("(")) {
        EAT(1);
        result = parse_expr();
        EXPECT(")", return NULL);
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

        result = expr_val(val);
    }

    return result;
}

static calc_expr_t * parse_expr_more(calc_expr_t *left, int min_prec);

static calc_expr_t * parse_operand() {
    u32          op;
    calc_expr_t *child;

    op = lookahead_unary_prefix_op();

    if (op == OP_INVALID) { return parse_leaf_expr(); }

    EAT(strlen(OP_STR(op)));

    if ((child = parse_operand()) == NULL) {
        /* @error */
/*         report_loc_err(GET_BEG_POINT(str), "invalid operand to '%s' expression", OP_STR(op)); */
        return NULL;
    }

    return expr_unary(op, child);
}

static calc_expr_t * parse_expr_more(calc_expr_t *left, int min_prec) {
    /*
    ** based on algorithm found here:
    ** https://en.wikipedia.org/wiki/Operator-precedence_parser#Pseudo-code
    */

    calc_expr_t *right;
    int          op;
    int          op_prec;
    int          lookahead_op;

    right = NULL;

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

        EAT(strlen(OP_STR(op)));

        if (op == OP_CALL) {
            right = parse_expr();
            if (right == NULL) { return NULL; }
            EXPECT(")", return NULL);
        } else {
            if (OP_IS_BINARY(op)) {
                right = parse_operand();
                if (right == NULL) {
                    /* @error */
/*                     report_loc_err(GET_BEG_POINT(str), "missing operand to binary '%s' expression", OP_STR(op)); */
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
                /* @error */
/*                 report_loc_err(GET_BEG_POINT(str), "missing operand to binary '%s' expression", OP_STR(lookahead_op)); */
                return NULL;
            }
        }

        /* lhs := the result of applying op with operands lhs and rhs */

        left = expr_binary(op, left, right);
    }

    return left;
}

static calc_expr_t * parse_expr_prec(int min_prec) {
    calc_expr_t *operand;

    operand = parse_operand();
    if (operand == NULL) { return NULL; }

    return parse_expr_more(operand, min_prec);
}

static calc_expr_t * parse_expr() {
    calc_expr_t *expr;
    expr = parse_expr_prec(0);

    if (*str) {
        /* @error */
        if (expr != NULL) {
            free_expr(expr);
        }
        return NULL;
    }

    return expr;
}



static yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*calc");

    if (buff == NULL) {
        buff = yed_create_buffer("*calc");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}

static void test_expr(int n_args, char **args) {
    yed_line    *line;
    calc_expr_t *expr;

    line = yed_buff_get_line(ys->active_frame->buffer, ys->active_frame->cursor_line);
    array_zero_term(line->chars);
    str = array_data(line->chars);

    EAT(0);

    expr = parse_expr();

    if (expr == NULL) {
        yed_cerr("bad expr");
        return;
    }

    yed_cprint("nice expr");
}

static void set_cursor_top_line(yed_frame *f) {
    LOG_FN_ENTER();
    yed_log("nice");
    LOG_EXIT();
}

#define CHECK_EVENT_FRAME_AND_BUFFER(_evt)            \
do {                                                  \
    if (event->frame         == NULL                  \
    ||  event->frame->buffer == NULL                  \
    ||  event->frame->buffer != get_or_make_buff()) { \
        return;                                       \
    }                                                 \
} while (0)

static void frame_post_set_buffer_handler(yed_event *event) {
    CHECK_EVENT_FRAME_AND_BUFFER(event);

    set_cursor_top_line(event->frame);
}

static void frame_activated_handler(yed_event *event) {
    CHECK_EVENT_FRAME_AND_BUFFER(event);

    set_cursor_top_line(event->frame);
}

static void cursor_moved_handler(yed_event *event) {
    CHECK_EVENT_FRAME_AND_BUFFER(event);

    if (frame->cursor_line != 1) {

    }
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    vars = tree_make_c(varname_t, calc_val_t, strcmp);

    get_or_make_buff();

    yed_plugin_set_command(self, "calc-expr", test_expr);

    h.kind = EVENT_FRAME_ACTIVATED;
    h.fn   = frame_activated_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_FRAME_POST_SET_BUFFER;
    h.fn   = frame_post_set_buffer_handler;
    yed_plugin_add_event_handler(self, h);

    return 0;
}
