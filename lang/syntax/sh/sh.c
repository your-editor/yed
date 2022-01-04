#include <yed/plugin.h>
#include <yed/syntax.h>

static yed_syntax syn;

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

#define _CHECK(x, r)                                                      \
do {                                                                      \
    if (x) {                                                              \
        LOG_FN_ENTER();                                                   \
        yed_log("[!] " __FILE__ ":%d regex error for '%s': %s", __LINE__, \
                r,                                                        \
                yed_syntax_get_regex_err(&syn));                          \
        LOG_EXIT();                                                       \
    }                                                                     \
} while (0)

#define SYN()          yed_syntax_start(&syn)
#define ENDSYN()       yed_syntax_end(&syn)
#define APUSH(s)       yed_syntax_attr_push(&syn, s)
#define APOP(s)        yed_syntax_attr_pop(&syn)
#define RANGE(r)       _CHECK(yed_syntax_range_start(&syn, r), r)
#define ONELINE()      yed_syntax_range_one_line(&syn)
#define SKIP(r)        _CHECK(yed_syntax_range_skip(&syn, r), r)
#define ENDRANGE(r)    _CHECK(yed_syntax_range_end(&syn, r), r)
#define REGEX(r)       _CHECK(yed_syntax_regex(&syn, r), r)
#define REGEXSUB(r, g) _CHECK(yed_syntax_regex_sub(&syn, r, g), r)
#define KWD(k)         yed_syntax_kwd(&syn, k)

#ifdef __APPLE__
#define WB "[[:>:]]"
#else
#define WB "\\b"
#endif

typedef struct {
    char       close;
    yed_attrs *attrs;
    int        is_arith;
    int        paren_balance;
} sh_hl_cxt;

void syntax_sh_highlight_strings_and_expansions(yed_line *line, array_t line_attrs);

void estyle(yed_event *event)   { yed_syntax_style_event(&syn, event);         }
void ebuffdel(yed_event *event) { yed_syntax_buffer_delete_event(&syn, event); }
void ebuffmod(yed_event *event) { yed_syntax_buffer_mod_event(&syn, event);    }
void eline(yed_event *event)  {
    yed_frame *frame;
    yed_line  *line;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Shell")) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, event->row);
    if (line != NULL) {
        syntax_sh_highlight_strings_and_expansions(line, event->line_attrs);
    }

    yed_syntax_line_event(&syn, event);
}


void unload(yed_plugin *self) {
    yed_syntax_free(&syn);
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler style;
    yed_event_handler buffdel;
    yed_event_handler buffmod;
    yed_event_handler line;

    char              *kwds[] = {
        "time", "shift",
        "source",   "select",   "export",
        "function",
    };
    char              *control_flow[] = {
        "if",       "fi",       "do",       "in",
        "for", "done",     "else",     "elif",     "then",     "wait",   "case",   "esac",
        "while",    "until", "break", "continue",
    };
    char              *builtins[] = {
        "bg",        "cd",       "fc",       "fg",
        "let",       "pwd",      "set",
        "bind",      "dirs",     "echo",     "eval",    "exec",    "exit",    "false",   "hash",    "help",    "jobs",    "kill",    "popd", "read", "test", "trap", "true", "type", "wait",
        "alias",     "break",    "local",    "pushd",   "shopt",   "times",   "umask",   "unset",
        "caller",    "disown",   "enable",   "logout",  "printf",  "return",  "ulimit",
        "builtin",   "command",  "compgen",  "compopt", "declare", "getopts", "history", "mapfile", "suspend", "typeset", "unalias",
        "complete",  "readonly",
        "readarray",
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    style.kind = EVENT_STYLE_CHANGE;
    style.fn   = estyle;
    yed_plugin_add_event_handler(self, style);

    buffdel.kind = EVENT_BUFFER_PRE_DELETE;
    buffdel.fn   = ebuffdel;
    yed_plugin_add_event_handler(self, buffdel);

    buffmod.kind = EVENT_BUFFER_POST_MOD;
    buffmod.fn   = ebuffmod;
    yed_plugin_add_event_handler(self, buffmod);

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = eline;
    yed_plugin_add_event_handler(self, line);


    SYN();
        APUSH("");
            REGEX("\\$\\{?#");
            RANGE("\""); SKIP("\\\\\""); ENDRANGE("\"");
            RANGE("'");  SKIP("\\\\'");  ENDRANGE("'");
        APOP();

        APUSH("&code-comment");
            RANGE("#"); ONELINE(); ENDRANGE("$");
        APOP();


        APUSH("&code-number");
            REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+)))"WB, 2);
            REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
        APOP();

        APUSH("&code-keyword");
            ARRAY_LOOP(kwds) KWD(*it);
        APOP();

        APUSH("&code-control-flow");
            ARRAY_LOOP(control_flow) KWD(*it);
        APOP();

        APUSH("&code-fn-call");
            ARRAY_LOOP(builtins) KWD(*it);
        APOP();
    ENDSYN();

    return 0;
}

void syntax_sh_highlight_strings_and_expansions(yed_line *line, array_t line_attrs) {
    int        col;
    array_t    stack;
    sh_hl_cxt *cxt, new_cxt;
    yed_attrs  str, con, num;
    yed_glyph  last;
    yed_glyph *g;

    if (line->visual_width == 0) {
        return;
    }

    stack = array_make(sh_hl_cxt);
    str   = yed_active_style_get_code_string();
    con   = yed_active_style_get_code_constant();
    num   = yed_active_style_get_code_number();
    cxt   = NULL;
    last  = G(0);
    for (col = 1; col <= line->visual_width; col += 1) {
        g = yed_line_col_to_glyph(line, col);

        if (!cxt && g->c == '#') { goto cleanup; }

        if (last.c == '\\') {
            if ((cxt = array_last(stack))) {
                yed_combine_attrs(array_item(line_attrs, col - 1), cxt->attrs);
            }
            goto next;
        }

        if ((cxt = array_last(stack))
        &&  g->c == cxt->close) {
            switch (g->c) {
                case '"':
                    yed_combine_attrs(array_item(line_attrs, col - 1), cxt->attrs);
                    break;
                case '\'':
                    yed_combine_attrs(array_item(line_attrs, col - 1), cxt->attrs);
                    break;
                case ')':
                    if (cxt->is_arith) {
                        if (cxt->paren_balance != 0) {
                            cxt->paren_balance -= 1;
                            goto dont_pop;
                        } else if (col < line->visual_width) {
                            if (yed_line_col_to_glyph(line, col + 1)->c == ')') {
                                yed_combine_attrs(array_item(line_attrs, col - 1), &num);
                                yed_combine_attrs(array_item(line_attrs, col), &num);
                                col += 1;
                            }
                        }
                        break;
                    } /* else fall through */
                case '}':
                    yed_combine_attrs(array_item(line_attrs, col - 1), &con);
                    break;
            }
            array_pop(stack);
            goto next;
dont_pop:;
        } else {
            switch (g->c) {
                case '"':
                    new_cxt.close    = '"';
                    new_cxt.attrs    = &str;
                    new_cxt.is_arith = 0;
                    array_push(stack, new_cxt);
                    break;
                case '\'':
                    new_cxt.close    = '\'';
                    new_cxt.attrs    = &str;
                    new_cxt.is_arith = 0;
                    array_push(stack, new_cxt);
                    break;
                case '(':
                    if (cxt && cxt->is_arith) {
                        cxt->paren_balance += 1;
                    } else if (col < line->visual_width) {
                        if (yed_line_col_to_glyph(line, col + 1)->c == '(') {
                            yed_combine_attrs(array_item(line_attrs, col - 1), &num);
                            yed_combine_attrs(array_item(line_attrs, col), &num);
                            new_cxt.close         = ')';
                            new_cxt.attrs         = NULL;
                            new_cxt.is_arith      = 1;
                            new_cxt.paren_balance = -1;
                            array_push(stack, new_cxt);
                        }
                    }
                    break;
                case '$':
                    if (cxt && cxt->close == '\'') {
                        yed_combine_attrs(array_item(line_attrs, col - 1), cxt->attrs);
                        goto next;
                    }

                    if (col < line->visual_width) {
                        if (yed_line_col_to_glyph(line, col + 1)->c == '(') {
                            if (col < line->visual_width - 1
                            &&  yed_line_col_to_glyph(line, col + 2)->c == '(') {
                                yed_combine_attrs(array_item(line_attrs, col - 1), &con);
                                goto next;
                            } else {
                                yed_combine_attrs(array_item(line_attrs, col - 1), &con);
                                yed_combine_attrs(array_item(line_attrs, col), &con);
                                new_cxt.close    = ')';
                                new_cxt.attrs    = NULL;
                                new_cxt.is_arith = 0;
                                array_push(stack, new_cxt);
                            }
                        } else if (yed_line_col_to_glyph(line, col + 1)->c == '{') {
                            new_cxt.close    = '}';
                            new_cxt.attrs    = &con;
                            new_cxt.is_arith = 0;
                            array_push(stack, new_cxt);
                        } else {
                            yed_combine_attrs(array_item(line_attrs, col - 1), &con);
                            while (col + 1 <= line->visual_width
                            &&     !isspace((g = yed_line_col_to_glyph(line, col + 1))->c)
                            &&     g->c != '\''
                            &&     g->c != '"'
                            &&     g->c != '('
                            &&     g->c != ')'
                            &&     g->c != '['
                            &&     g->c != ']') {
                                yed_combine_attrs(array_item(line_attrs, col), &con);
                                col += 1;
                            }
                            goto next;
                        }
                    }
                    break;
            }
        }

        cxt = array_last(stack);

        if (cxt && cxt->attrs) {
            yed_combine_attrs(array_item(line_attrs, col - 1), cxt->attrs);
        }
next:;
        last = *g;
    }

cleanup:
    array_free(stack);
}
