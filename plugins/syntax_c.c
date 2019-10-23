#include "plugin.h"

void syntax_c_handler(yed_event *event);
void syntax_c_highlight(yed_event *event);
void syntax_c_highlight_keywords(yed_event *event);
void syntax_c_highlight_numbers(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_c_handler;

    yed_plugin_add_event_handler(self, h);

    return 0;
}

void syntax_c_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    syntax_c_highlight(event);
}

void syntax_c_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_hash, last_was_minus;
    char       c, *word,
              *kwds[][8] = {
                  { "do",       "if",       NULL,      NULL,       NULL,     NULL,     NULL,     NULL   }, /* 2 */
                  { "int",      "for",      NULL,      NULL,       NULL,     NULL,     NULL,     NULL   }, /* 3 */
                  { "long",     "char",     "bool",    "void",     "enum",   "else",   "case",   "goto" }, /* 4 */
                  { "const",    "short",    "float",   "while",    "break",  NULL,     NULL,     NULL   }, /* 5 */
                  { "static",   "size_t",   "double",  "struct",   "switch", "return", "sizeof", NULL   }, /* 6 */
                  { "ssize_t",  "typedef",  "default", NULL,       NULL,     NULL,     NULL,     NULL   }, /* 7 */
                  { "unsigned", "continue", NULL,      NULL,       NULL,     NULL,     NULL,     NULL   }  /* 8 */
              },
             *pp_kwds[][3] = {
                  { "if",      NULL,     NULL     }, /* 2 */
                  { NULL,      NULL,     NULL     }, /* 3 */
                  { "else",    NULL,     NULL     }, /* 4 */
                  { "undef",   "endif",  "ifdef"  }, /* 5 */
                  { "define",  "ifndef", "pragma" }, /* 6 */
                  { "include", NULL,     NULL     }  /* 7 */
             };
    #define MIN_KWD_LEN   (2)
    #define MAX_KWD_LEN   (8)
    #define MIN_PPKWD_LEN (2)
    #define MAX_PPKWD_LEN (7)

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);
    col   = 1;

    last_was_hash  = 0;
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
        if (last_was_hash) {
            if (word_len >= MIN_PPKWD_LEN && word_len <= MAX_PPKWD_LEN) {
                for (i = 0; i < sizeof(pp_kwds[word_len - MIN_PPKWD_LEN]) / sizeof(const char*); i += 1) {
                    if (!pp_kwds[word_len - MIN_PPKWD_LEN][i])    { break; }

                    match = (strncmp(word, pp_kwds[word_len - MIN_PPKWD_LEN][i], word_len) == 0);

                    if (match) {
                        attr         = array_item(event->line_attrs, old_col - 2);
                        attr->flags  = ATTR_RGB;
                        attr->flags |= ATTR_BOLD;
                        attr->fg     = RGB_32(64, 121, 140);

                        for (j = 0; j < word_len; j += 1) {
                            attr         = array_item(event->line_attrs, old_col + j - 1);
                            attr->flags  = ATTR_RGB;
                            attr->flags |= ATTR_BOLD;
                            attr->fg     = RGB_32(64, 121, 140);
                        }
                        break;
                    }
                }
            }
        } else {
            if (word_len >= MIN_KWD_LEN && word_len <= MAX_KWD_LEN) {
                for (i = 0; i < sizeof(kwds[word_len - MIN_KWD_LEN]) / sizeof(const char*); i += 1) {
                    if (!kwds[word_len - MIN_KWD_LEN][i])    { break; }

                    match = (strncmp(word, kwds[word_len - MIN_KWD_LEN][i], word_len) == 0);

                    if (match) {
                        for (j = 0; j < word_len; j += 1) {
                            attr         = array_item(event->line_attrs, old_col + j - 1);
                            attr->flags  = ATTR_RGB;
                            attr->flags |= ATTR_BOLD;
                            attr->fg     = RGB_32(64, 121, 140);
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
                    attr         = array_item(event->line_attrs, old_col - 2);
                    attr->flags  = ATTR_RGB;
                    attr->fg     = RGB_32(216, 30, 91);
                }
                for (i = 0; i < word_len; i += 1) {
                    attr         = array_item(event->line_attrs, old_col + i - 1);
                    attr->flags  = ATTR_RGB;
                    attr->fg     = RGB_32(216, 30, 91);
                }
            }
        }

        /* NULL */
        if (!match) {
            if (strncmp(word, "NULL", word_len) == 0) {
                match = 1;

                for (i = 0; i < word_len; i += 1) {
                    attr         = array_item(event->line_attrs, old_col + i - 1);
                    attr->flags  = ATTR_RGB;
                    attr->fg     = RGB_32(252, 163, 17);
                }
            }
        }

        last_was_hash  = !spaces && word_len == 1 && (strncmp(word, "#", 1) == 0);
        last_was_minus = !last_was_hash && !spaces && word_len == 1 && (strncmp(word, "-", 1) == 0);
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
                            attr         = array_item(event->line_attrs, k);
                            attr->flags  = ATTR_RGB;
                            attr->fg     = RGB_32(83, 170, 111);
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
     * Highlight string literals.
     */
    i = 0;
    while (i < array_len(line->chars) - 1) {
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
                    attr         = array_item(event->line_attrs, k);
                    attr->flags  = ATTR_RGB;
                    attr->fg     = RGB_32(83, 170, 111);
                }
            }

            i += j + 1;
        } else {
            i += 1;
        }
    }

    /*
     * Highlight C-style comments.
     */
    i = 0;
    while (i < array_len(line->chars) - 3) {
        word = array_item(line->chars, i);
        if (*word == '/' && *(word + 1) == '*') {
            j = 2;
            while (1) {
                if (i + j + 1 >= array_len(line->chars)) {
                    break;
                }
                if (*(word + j) == '*' && *(word + j + 1) == '/') {
                    j     += 1;
                    match  = 1;
                    break;
                }
                j += 1;
            }

            if (match) {
                for (k = i; k <= i + j; k += 1) {
                    attr         = array_item(event->line_attrs, k);
                    attr->flags  = ATTR_RGB;
                    attr->flags |= ATTR_BOLD;
                    attr->fg     = RGB_32(72, 180, 235);
                }
            }

            i += j + 1;
        } else {
            i += 1;
        }
    }
}
