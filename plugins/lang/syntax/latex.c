#include <yed/plugin.h>

void latex_syntax_line_handler(yed_event *event);
void latex_syntax_highlight(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = latex_syntax_line_handler;

    yed_plugin_add_event_handler(self, line);

    ys->redraw = 1;

    return 0;
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
                    attr = array_item(event->line_attrs, k);
                    yed_combine_attrs(attr, &str);
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
            attr = array_item(event->line_attrs, old_col - 2);
            yed_combine_attrs(attr, &cal);

            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &cal);
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
                attr = array_item(event->line_attrs, k);
                yed_combine_attrs(attr, &com);
            }
            break;

        } else {
            i += 1;
        }
    }
}
