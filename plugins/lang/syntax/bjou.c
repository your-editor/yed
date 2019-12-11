#include "plugin.h"

void syntax_bjou_line_handler(yed_event *event);
void syntax_bjou_highlight(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_bjou_line_handler;

    yed_plugin_add_event_handler(self, h);

    ys->redraw = 1;

    return 0;
}

void syntax_bjou_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_BJOU) {
        return;
    }

    syntax_bjou_highlight(event);
}

void syntax_bjou_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, pp, cal, num, con, cha, str;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_backslash, last_was_minus;
    char       c, *word,
              *kwds[][17] = {
                { "as",            "do",       "i8",      "if",      "in",     "or",     "u8",     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 2  */
                { "and",           "bor",      "f32",     "f64",     "for",    "i16",    "i32",    "i64",  "int",  "new",  "not",  "ref",  "u16",  "u32",  "u64",  NULL,   NULL   }, /* 3  */
                { "Type",          "band",     "bneg",    "bool",    "bshl",   "bshr",   "bxor",   "char", "else", "enum", "f128", "long", "proc", "some", "true", "type", "void" }, /* 4  */
                { "break",         "false",    "float",   "print",   "short",  "using",  "while",  NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 5  */
                { "delete",        "double",   "extern",  "import",  "module", "return", "sizeof", NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 6  */
                { "extends",       "foreach",  "include", "nothing", NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 7  */
                { "abstract",      "continue", NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 8  */
                { NULL,            NULL,       NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 9  */
                { "__inline__",    NULL,       NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 10 */
                { NULL,            NULL,       NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 11 */
                { NULL,            NULL,       NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 12 */
                { "__no_mangle__", NULL,       NULL,      NULL,      NULL,     NULL,     NULL,     NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 13 */
              };

    #define MIN_KWD_LEN   (2)
    #define MAX_KWD_LEN   (13)

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    pp  = yed_active_style_get_code_preprocessor();
    cal = yed_active_style_get_code_fn_call();
    num = yed_active_style_get_code_number();
    con = yed_active_style_get_code_constant();
    cha = yed_active_style_get_code_character();
    str = yed_active_style_get_code_string();

    col = 1;

    last_was_backslash  = 0;
    last_was_minus      = 0;
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
            yed_combine_attrs(attr, &pp);

            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &pp);
            }

            match = 1;
        }


        /*
         * Try to match keywords.
         */
        if (!match) {
            if (word_len >= MIN_KWD_LEN && word_len <= MAX_KWD_LEN) {
                for (i = 0; i < sizeof(kwds[word_len - MIN_KWD_LEN]) / sizeof(const char*); i += 1) {
                    if (!kwds[word_len - MIN_KWD_LEN][i])    { break; }

                    match = (strncmp(word, kwds[word_len - MIN_KWD_LEN][i], word_len) == 0);

                    if (match) {
                        for (j = 0; j < word_len; j += 1) {
                            attr = array_item(event->line_attrs, old_col + j - 1);
                            yed_combine_attrs(attr, &key);
                        }
                        break;
                    }
                }
            }
        }

        /*
         * Try to match numbers.
         */
        if (!match) {
            match = 1;

            for (i = 0; i < word_len; i += 1) {
                if (word[i] < '0' || word[i] > '9') {
                    match = 0;
                    break;
                }
            }

            if (match) {
                if (last_was_minus) {
                    attr = array_item(event->line_attrs, old_col - 2);
                    yed_combine_attrs(attr, &num);
                }
                for (i = 0; i < word_len; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &num);
                }
            }
        }

        /* Function calls */
        if (!match) {
            if (old_col + word_len - 1 < array_len(line->chars)
            &&  yed_line_col_to_char(line, old_col + word_len) == '(') {
                for (i = 0; i < word_len; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &cal);
                }
            }
        }

        last_was_backslash = !spaces && word[word_len - 1] == '\\';
        last_was_minus     = !last_was_backslash && !spaces && word_len == 1 && (strncmp(word, "-", 1) == 0);
    }

    match = 0;

    /*
     * Highlight character literals.
     */
    i = 0;
    while (i < array_len(line->chars) - 2) {
        word = array_item(line->chars, i);
        if (*word == '\'') {
            j = 1;
            if (i + j < array_len(line->chars)) {
                if (*(word + j) == '\\') {
                    j += 1;
                }
                j += 1;
                if (i + j < array_len(line->chars)) {
                    if (*(word + j) == '\'') {
                        for (k = i; k <= i + j; k += 1) {
                            attr = array_item(event->line_attrs, k);
                            yed_combine_attrs(attr, &cha);
                        }

                    }
                }
            }
            i += j + 1;
        } else {
            i += 1;
        }
    }

    /*
     * Highlight string literals and comments.
     */
    i = 0;
    while (i < array_len(line->chars)) {
        word = array_item(line->chars, i);

        if (*word == '"') {
            j = 1;

            while (i + j < array_len(line->chars)) {
                c = word[j];

                if (c == '"') {
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

        /*
         * Highlight comments.
         */
        } else if (*word == '#') {
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
