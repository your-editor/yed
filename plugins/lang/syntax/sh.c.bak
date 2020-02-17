#include <yed/plugin.h>

void syntax_sh_line_handler(yed_event *event);
void syntax_sh_highlight(yed_event *event);

static int col_filter[1024];

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_sh_line_handler;

    yed_plugin_add_event_handler(self, line);

    ys->redraw = 1;

    return 0;
}

void syntax_sh_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_SH) {
        return;
    }

    syntax_sh_highlight(event);
}

void syntax_sh_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, cal, num, con, cha, str;
    int        col, old_col, word_len, spaces, last_spaces, i, j, k, match, balance, expan_col, sq_string_on, dq_string_on, save_sq_string_on, save_dq_string_on, str_delim,
               last_was_minus, last_was_dollar, last_was_dollar_bracket, last_was_dollar_paren;
    char       c, *word,
              *kwds[][8] = {
                  { "if",       "fi",       "do",       "in",       NULL,     NULL,     NULL,     NULL     }, /* 2 */
                  { "for",      NULL,       NULL,       NULL,       NULL,     NULL,     NULL,     NULL     }, /* 3 */
                  { "done",     "else",     "elif",     "then",     "wait",   "case",   "esac",   "time"   }, /* 4 */
                  { "while",    "until",    "shift",    "break",    NULL,     NULL,     NULL,     NULL     }, /* 5 */
                  { "source",   "select",   "export",   NULL,       NULL,     NULL,     NULL,     NULL     }, /* 6 */
                  { NULL,       NULL,       NULL,       NULL,       NULL,     NULL,     NULL,     NULL     }, /* 7 */
                  { "continue", "function", NULL,       NULL,       NULL,     NULL,     NULL,     NULL     }, /* 8 */
              },
             *builtins[][18] = {
                  { "bg",        "cd",       "fc",       "fg",      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 2 */
                  { "let",       "pwd",      "set",      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 3 */
                  { "bind",      "dirs",     "echo",     "eval",    "exec",    "exit",    "false",   "hash",    "help",    "jobs",    "kill",    "popd", "read", "test", "trap", "true", "type", "wait" }, /* 4 */
                  { "alias",     "break",    "local",    "pushd",   "shopt",   "times",   "umask",   "unset",   NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 5 */
                  { "caller",    "disown",   "enable",   "logout",  "printf",  "return",  "ulimit",  NULL,      NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 6 */
                  { "builtin",   "command",  "compgen",  "compopt", "declare", "getopts", "history", "mapfile", "suspend", "typeset", "unalias", NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 7 */
                  { "complete",  "readonly", NULL,       NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 8 */
                  { "readarray", NULL,       NULL,       NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL   }, /* 9 */
              };
    #define MIN_KWD_LEN     (2)
    #define MAX_KWD_LEN     (8)
    #define MIN_BUILTIN_LEN (2)
    #define MAX_BUILTIN_LEN (9)

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    cal = yed_active_style_get_code_fn_call();
    num = yed_active_style_get_code_number();
    con = yed_active_style_get_code_constant();
    cha = yed_active_style_get_code_character();
    str = yed_active_style_get_code_string();

    memset(col_filter, 0, sizeof(col_filter));



    col = 1;
    last_was_minus = 0;
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

        /*
         * Try to match keywords.
         */
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

        /*
         * Try to match builtins.
         */

        if (!match) {
            if (word_len >= MIN_BUILTIN_LEN && word_len <= MAX_BUILTIN_LEN) {
                for (i = 0; i < sizeof(builtins[word_len - MIN_BUILTIN_LEN]) / sizeof(const char*); i += 1) {
                    if (!builtins[word_len - MIN_BUILTIN_LEN][i])    { break; }

                    match = (strncmp(word, builtins[word_len - MIN_BUILTIN_LEN][i], word_len) == 0);

                    if (match) {
                        for (j = 0; j < word_len; j += 1) {
                            attr = array_item(event->line_attrs, old_col + j - 1);
                            yed_combine_attrs(attr, &cal);
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

        last_was_minus = !spaces && word_len == 1 && (strncmp(word, "-", 1) == 0);
    }





    /*
     * Highlight expansions.
     */

    last_was_dollar         = 0;
    last_was_dollar_bracket = 0;
    last_was_dollar_paren   = 0;
    col                     = 1;

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

        sq_string_on            = 0;
        dq_string_on            = 0;

        if (!match) {
            if (last_was_dollar) {
                attr = array_item(event->line_attrs, old_col - 2);
                yed_combine_attrs(attr, &con);
                col_filter[old_col - 1] = 1;

                for (i = 0; i < word_len; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &con);

                    col_filter[old_col + i] = 1;
                }
            } else if (last_was_dollar_paren) {
                attr = array_item(event->line_attrs, old_col - (3 + last_spaces));
                yed_combine_attrs(attr, &con);
                col_filter[old_col - 2] = 1;
                attr         = array_item(event->line_attrs, old_col - (2 + last_spaces));
                yed_combine_attrs(attr, &con);
                col_filter[old_col - 1] = 1;

                expan_col = old_col;
                balance   = 1;
                while (expan_col <= array_len(line->chars)) {
                    str_delim = 0;
                    c         = yed_line_col_to_char(line, expan_col);

                    if (c == '(') {
                        balance  += 1;
                    } else if (c == ')') {
                        dq_string_on = save_dq_string_on;
                        sq_string_on = save_sq_string_on;
                        balance  -= 1;
                    } else if (c == '"') {
                        if (expan_col == 1 || yed_line_col_to_char(line, expan_col - 1) != '\\') {
                            str_delim = 1;
                        }
                    } else if (c == '\'') {
                        str_delim = 1;
                    } else if (c == '$') {
                        if (expan_col < array_len(line->chars)) {
                            if (yed_line_col_to_char(line, expan_col + 1) == '(') {
                                save_dq_string_on = dq_string_on;
                                save_sq_string_on = sq_string_on;
                                dq_string_on = 0;
                                sq_string_on = 0;
                            }
                        }
                    }

                    if (str_delim) {
                        if (!dq_string_on && c == '\"') {
                            str_delim = 0;
                            dq_string_on = !dq_string_on;
                        } else if (!sq_string_on && c == '\'') {
                            str_delim = 0;
                            sq_string_on = !sq_string_on;
                        }
                    }

                    col_filter[expan_col] = 1;

                    if (dq_string_on || sq_string_on) {
                        attr = array_item(event->line_attrs, expan_col - 1);
                        yed_combine_attrs(attr, &str);
                    }

                    if (balance == 0) {
                        attr = array_item(event->line_attrs, expan_col - 1);
                        yed_combine_attrs(attr, &con);
                        break;
                    }

                    if (str_delim) {
                        if      (dq_string_on) { dq_string_on = !dq_string_on; }
                        else if (sq_string_on) { sq_string_on = !dq_string_on; }
                    }

                    expan_col += 1;
                }
            } else if (last_was_dollar_bracket) {
                attr = array_item(event->line_attrs, old_col - (3 + last_spaces));
                yed_combine_attrs(attr, &con);
                col_filter[old_col - 2] = 1;
                attr = array_item(event->line_attrs, old_col - (2 + last_spaces));
                yed_combine_attrs(attr, &con);
                col_filter[old_col - 1] = 1;

                expan_col = old_col;
                balance   = 1;
                while (expan_col <= array_len(line->chars)) {
                    str_delim = 0;
                    c         = yed_line_col_to_char(line, expan_col);

                    if      (c == '{')  { balance  += 1; }
                    else if (c == '}')  { balance  -= 1; }
                    else if (c == '"')  {
                        if (expan_col == 1 || yed_line_col_to_char(line, expan_col - 1) != '\\') {
                            str_delim = 1;
                        }
                    } else if (c == '\'') {
                        str_delim = 1;
                    }

                    if (str_delim) {
                        if (!dq_string_on && c == '"') {
                            str_delim = 0;
                            dq_string_on = 1;
                        } else if (!sq_string_on && c == '\'') {
                            str_delim = 0;
                            sq_string_on = 1;
                        }
                    }

                    col_filter[expan_col] = 1;

                    if (balance == 0) {
                        attr = array_item(event->line_attrs, expan_col - 1);
                        yed_combine_attrs(attr, &con);
                        break;
                    }

                    if (dq_string_on || sq_string_on) {
                        attr = array_item(event->line_attrs, expan_col - 1);
                        yed_combine_attrs(attr, &str);
                    } else {
                        attr = array_item(event->line_attrs, expan_col - 1);
                        yed_combine_attrs(attr, &con);
                    }

                    if (str_delim) {
                        if      (dq_string_on) { dq_string_on = 0; }
                        else if (sq_string_on) { sq_string_on = 0; }
                    }

                    expan_col += 1;
                }
            } else {
                for (i = 0; i < word_len; i += 1) {
                    c = yed_line_col_to_char(line, old_col + i);
                    if (c == '$') {
                        for (j = i; j < word_len; j += 1) {
                            c = yed_line_col_to_char(line, old_col + j);
                            if (c == '"')    { break; }

                            attr = array_item(event->line_attrs, old_col + j - 1);
                            yed_combine_attrs(attr, &con);
                            col_filter[old_col + j] = 1;
                        }
                        break;
                    }
                }
            }
        }

        last_was_dollar         = !spaces && ((word_len == 1 && (strncmp(word, "$", 1) == 0))
                                          ||  (strncmp(word + word_len - 1, "$", 1) == 0));
        last_was_dollar_bracket = last_was_dollar_paren = 0;
        for (i = 0; i < word_len - 1; i += 1) {
            c = yed_line_col_to_char(line, old_col + i);
            if (c == '$') {
                c = yed_line_col_to_char(line, old_col + i + 1);
                if (c == '{') {
                    last_was_dollar_bracket = 1;
                    spaces = word_len - i - 2;
                    break;
                } else if (c == '(') {
                    last_was_dollar_paren   = 1;
                    spaces = word_len - i - 2;
                    break;
                }
            }
        }

        last_spaces = spaces;
    }

    match = 0;

    /*
     * Highlight backtick expansion.
     */
    i = 0;
    while (i < array_len(line->chars) - 1) {
        c = *(char*)array_item(line->chars, i);

        if (c == '`') {
            col_filter[i + 1] = 1;
            attr = array_item(event->line_attrs, i);
            yed_combine_attrs(attr, &con);

            j = 1;

            while (i + j < array_len(line->chars)) {
                c = *(char*)array_item(line->chars, i + j);

                col_filter[i + j + 1] = 1;

                if (c == '`') {
                    attr = array_item(event->line_attrs, i + j);
                    yed_combine_attrs(attr, &con);
                    break;
                }

                j += 1;
            }

            i += j + 1;
        }

        i += 1;
    }

    /*
     * Highlight string literals.
     */
    i = 0;
    while (i < array_len(line->chars) - 1) {
        word = array_item(line->chars, i);

        if (!col_filter[i + 1] && *word == '"') {
            j = 1;

            while (i + j < array_len(line->chars)) {
                c = word[j];

                if (!col_filter[i + j + 1] && c == '"') {
                    match = 1;
                    break;
                } else if (c == '\\') {
                    j += 1;
                }
                j += 1;
            }

            if (match) {
                for (k = i; k <= i + j; k += 1) {
                    if (col_filter[k + 1])    { continue; }

                    if (*(char*)array_item(line->chars, k) == '#') {
                        col_filter[k + 1] = 1;
                    }

                    attr = array_item(event->line_attrs, k);
                    yed_combine_attrs(attr, &str);
                }
            }

            i += j + 1;
        } else if (!col_filter[i + 1] && *word == '\'') {
            j = 1;

            while (i + j < array_len(line->chars)) {
                c = word[j];

                if (!col_filter[i + j + 1] && c == '\'') {
                    match = 1;
                    break;
                }
                j += 1;
            }

            if (match) {
                for (k = i; k <= i + j; k += 1) {
                    if (col_filter[k + 1])    { continue; }

                    if (*(char*)array_item(line->chars, k) == '#') {
                        col_filter[k + 1] = 1;
                    }

                    attr = array_item(event->line_attrs, k);
                    yed_combine_attrs(attr, &cha);
                }
            }

            i += j + 1;
        } else {
            i += 1;
        }
    }

    /*
     * Highlight comments.
     */
    i = 0;
    while (i < array_len(line->chars)) {
        word = array_item(line->chars, i);
        if (!col_filter[i + 1] && *word == '#') {
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
