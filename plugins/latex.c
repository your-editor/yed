#include "plugin.h"

void latex_compile_current_file(int n_args, char **args);
void latex_view_current_file(int n_args, char **args);
void latex_syntax_line_handler(yed_event *event);

void latex_syntax_highlight(yed_event *event);

void add_commands(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = latex_syntax_line_handler;

    yed_plugin_add_event_handler(self, line);
    add_commands(self);

    ys->redraw = 1;

    return 0;
}

void add_commands(yed_plugin *self) {
    yed_plugin_set_command(self, "latex-compile-current-file", latex_compile_current_file);
    yed_plugin_set_command(self, "latex-view-current-file",    latex_view_current_file);
}

void latex_syntax_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_LATEX) {
        return;
    }

    latex_syntax_highlight(event);
}

void latex_compile_current_file(int n_args, char **args) {
    char       *comp_prg,
               *path;
    char        cmd_buff[256];
    yed_frame  *frame;
    yed_buffer *buff;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_append_text_to_cmd_buff("[!] buffer is not a file");
        return;
    }

    path = buff->path;

    comp_prg = yed_get_var("latex-comp-prg");

    if (!comp_prg) {
        yed_append_text_to_cmd_buff("[!] 'latex-comp-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    strcat(cmd_buff, comp_prg);
    strcat(cmd_buff, " ");
    strcat(cmd_buff, path);
    strcat(cmd_buff, " | less");

    YEXE("sh", cmd_buff);
}

void latex_view_current_file(int n_args, char **args) {
    char       *view_prg,
               *path;
    char        cmd_buff[256];
    yed_frame  *frame;
    yed_buffer *buff;
    int         err;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_append_text_to_cmd_buff("[!] buffer is not a file");
        return;
    }

    path = buff->path;

    view_prg = yed_get_var("latex-view-prg");

    if (!view_prg) {
        yed_append_text_to_cmd_buff("[!] 'latex-view-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    path = path_without_ext(path);

    strcat(cmd_buff, view_prg);
    strcat(cmd_buff, " ");
    strcat(cmd_buff, path);
    strcat(cmd_buff, ".pdf");
    free(path);

    err = system(cmd_buff);
    if (err != 0) {
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(cmd_buff);
        yed_append_text_to_cmd_buff("' exited with non-zero status ");
        yed_append_int_to_cmd_buff(err);
    }
}

void latex_syntax_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, pp, cal, num, con, cha, str;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_backslash;
    char       c, *word;

    #if 0
              *kwds[][8] = {
                  { "do",       "if",       NULL,       NULL,       NULL,     NULL,     NULL,     NULL     }, /* 2 */
                  { "int",      "for",      "asm",      NULL,       NULL,     NULL,     NULL,     NULL     }, /* 3 */
                  { "long",     "char",     "bool",     "void",     "enum",   "else",   "case",   "goto"   }, /* 4 */
                  { "const",    "short",    "float",    "while",    "break",  "union",  NULL,     NULL     }, /* 5 */
                  { "static",   "size_t",   "double",   "struct",   "switch", "return", "sizeof", "inline" }, /* 6 */
                  { "ssize_t",  "typedef",  "default",  "__asm__",  NULL,     NULL,     NULL,     NULL     }, /* 7 */
                  { "unsigned", "continue", "volatile", "restrict", NULL,     NULL,     NULL,     NULL     }  /* 8 */
              },
             *pp_kwds[][3] = {
                  { "if",      NULL,     NULL     }, /* 2 */
                  { NULL,      NULL,     NULL     }, /* 3 */
                  { "else",    NULL,     NULL     }, /* 4 */
                  { "undef",   "endif",  "ifdef"  }, /* 5 */
                  { "define",  "ifndef", "pragma" }, /* 6 */
                  { "include", NULL,     NULL     }  /* 7 */
             };
    #endif

    #define MIN_KWD_LEN   (2)
    #define MAX_KWD_LEN   (8)
    #define MIN_PPKWD_LEN (2)
    #define MAX_PPKWD_LEN (7)

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    (void)j;

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    pp  = yed_active_style_get_code_preprocessor();
    cal = yed_active_style_get_code_fn_call();
    num = yed_active_style_get_code_number();
    con = yed_active_style_get_code_constant();
    cha = yed_active_style_get_code_character();
    str = yed_active_style_get_code_string();

    (void)cha;
    (void)con;
    (void)num;
    (void)pp;
    (void)key;

    /*
     * Highlight math environments.
     */
    i = 0;
    while (i < array_len(line->chars) - 1) {
        word = array_item(line->chars, i);

        if (*word == '$') {
            j = 1;

            while (i + j < array_len(line->chars)) {
                c = word[j];

                if (c == '$') {
                    match = 1;
                    break;
                } else if (c == '\\') {
                    j += 1;
                }
                j += 1;
            }

            if (match) {
                for (k = i; k <= i + j; k += 1) {
                    attr         = array_item(event->line_attrs, k);
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= str.flags;
                    attr->fg     = str.fg;
                }
            }

            i += j + 1;
        } else {
            i += 1;
        }
    }


    col   = 1;

    last_was_backslash  = 0;
    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        spaces   = 0;
        match    = 0;
        c        = yed_line_col_to_char(line, col);

        if (isalnum(c) || c == '_') {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_char(line, col);

                if (!isalnum(c) && c != '_') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_char(line, col);

                if (isalnum(c) || c == '_' || isspace(c)) {
                    break;
                }
            }
        }

        if (isspace(c)) {
            while (col <= line->visual_width) {
                spaces += 1;
                col    += 1;
                c       = yed_line_col_to_char(line, col);

                if (!isspace(c)) {
                    break;
                }
            }
        }

        if (last_was_backslash) {
            attr         = array_item(event->line_attrs, old_col - 2);
            attr->flags &= ~(ATTR_BOLD);
            attr->flags |= cal.flags;
            attr->fg     = cal.fg;

            for (i = 0; i < word_len; i += 1) {
                attr         = array_item(event->line_attrs, old_col + i - 1);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= cal.flags;
                attr->fg     = cal.fg;
            }
        }

        last_was_backslash = !spaces && word[word_len - 1] == '\\';
    }

    match = 0;

    /*
     * Highlight comments.
     */
    i = 0;
    while (i < array_len(line->chars)) {
        word = array_item(line->chars, i);

        if (*word == '%') {
            for (k = i; k < array_len(line->chars); k += 1) {
                attr         = array_item(event->line_attrs, k);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= com.flags;
                attr->fg     = com.fg;
            }
            break;

        } else {
            i += 1;
        }
    }
}
