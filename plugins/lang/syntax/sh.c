#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo1;
highlight_info hinfo2;

typedef struct {
    char       close;
    yed_attrs *attrs;
    int        is_arith;
    int        paren_balance;
} sh_hl_cxt;

void unload(yed_plugin *self);
void syntax_sh_line_handler(yed_event *event);
void syntax_sh_frame_handler(yed_event *event);
void syntax_sh_buff_mod_pre_handler(yed_event *event);
void syntax_sh_buff_mod_post_handler(yed_event *event);
void syntax_sh_highlight_strings_and_expansions(yed_line *line, array_t line_attrs);


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;
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


    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_sh_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_sh_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_sh_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_sh_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);


    highlight_info_make(&hinfo1);

    ARRAY_LOOP(kwds)
        highlight_add_kwd(&hinfo1, *it, HL_KEY);
    ARRAY_LOOP(control_flow)
        highlight_add_kwd(&hinfo1, *it, HL_CF);
    ARRAY_LOOP(builtins)
        highlight_add_kwd(&hinfo1, *it, HL_CALL);
    highlight_numbers(&hinfo1);

    highlight_info_make(&hinfo2);

    highlight_within_multiline(&hinfo2, "${", "}", 0, HL_IGNORE);
    highlight_within(&hinfo2, "$", "#", 0, 0, HL_IGNORE);
    highlight_to_eol_from(&hinfo2, "#", HL_COMMENT);
    highlight_within_multiline(&hinfo2, "\"", "\"", '\\', HL_IGNORE);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo2);
    highlight_info_free(&hinfo1);
    ys->redraw = 1;
}

void syntax_sh_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Shell")) {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo1, event);
}

void syntax_sh_line_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Shell")) {
        return;
    }

    highlight_line(&hinfo1, event);
    highlight_line(&hinfo2, event);

    line = yed_buff_get_line(event->frame->buffer, event->row);

    syntax_sh_highlight_strings_and_expansions(line, event->line_attrs);
}

void syntax_sh_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Shell")) {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo1, event);
}

void syntax_sh_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Shell")) {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo1, event);
}

void syntax_sh_highlight_strings_and_expansions(yed_line *line, array_t line_attrs) {
    int        col;
    array_t    stack;
    sh_hl_cxt *cxt, new_cxt;
    yed_attrs  str, con, num;
    yed_glyph *g;

    if (line->visual_width == 0) {
        return;
    }

    stack = array_make(sh_hl_cxt);
    str   = yed_active_style_get_code_string();
    con   = yed_active_style_get_code_constant();
    num   = yed_active_style_get_code_number();
    cxt   = NULL;
    for (col = 1; col <= line->visual_width; col += 1) {
        g = yed_line_col_to_glyph(line, col);

        if (!cxt && g->c == '#') { goto cleanup; }

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
                            &&     g->c != ')') {
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
    }

cleanup:
    array_free(stack);
}
