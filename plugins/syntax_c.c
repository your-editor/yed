#include "plugin.h"

void syntax_c_line_handler(yed_event *event);
void syntax_c_frame_handler(yed_event *event);
void syntax_c_buff_mod_pre_handler(yed_event *event);
void syntax_c_buff_mod_post_handler(yed_event *event);

void syntax_c_highlight(yed_event *event);
void syntax_c_find_comment_lines(yed_frame *frame);
int  syntax_c_line_has_comment_delim(yed_line *line);

static int lines_in_comment[4096];
static yed_frame *last_frame;
static int line_has_delim;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;

    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_c_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_c_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_c_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_c_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);

    return 0;
}

void syntax_c_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_C) {
        return;
    }

    syntax_c_highlight(event);
}

void syntax_c_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_C) {
        return;
    }

    syntax_c_find_comment_lines(frame);
}

void syntax_c_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_C) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line) {
        return;
    }

    line_has_delim = syntax_c_line_has_comment_delim(line);
}

void syntax_c_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    int        delim;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_C) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line) {
        return;
    }

    delim = syntax_c_line_has_comment_delim(line);

    if (line_has_delim != delim) {
        frame->dirty = 1;
    }
}

void syntax_c_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, pp, cal, num, con, cha, str;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_hash, last_was_minus, delim;
    char       c, *word,
              *kwds[][8] = {
                  { "do",       "if",       NULL,       NULL,       NULL,     NULL,     NULL,     NULL     }, /* 2 */
                  { "int",      "for",      "asm",      NULL,       NULL,     NULL,     NULL,     NULL     }, /* 3 */
                  { "long",     "char",     "bool",     "void",     "enum",   "else",   "case",   "goto"   }, /* 4 */
                  { "const",    "short",    "float",    "while",    "break",  "union",     NULL,     NULL  }, /* 5 */
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
    #define MIN_KWD_LEN   (2)
    #define MAX_KWD_LEN   (8)
    #define MIN_PPKWD_LEN (2)
    #define MAX_PPKWD_LEN (7)

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    pp  = yed_active_style_get_code_preprocessor();
    cal = yed_active_style_get_code_fn_call();
    num = yed_active_style_get_code_number();
    con = yed_active_style_get_code_constant();
    cha = yed_active_style_get_code_character();
    str = yed_active_style_get_code_string();

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);
    col   = 1;

    if (frame != last_frame) {
        syntax_c_find_comment_lines(frame);
    }

    if (lines_in_comment[event->row - frame->buffer_y_offset]) {
        for (k = 0 ; k < array_len(line->chars); k += 1) {
            attr         = array_item(event->line_attrs, k);
            attr->flags &= ~(ATTR_BOLD);
            attr->flags |= com.flags;
            attr->fg     = com.fg;
        }
        return;
    }

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
                        attr->flags &= ~(ATTR_BOLD);
                        attr->flags |= key.flags;
                        attr->fg     = key.fg;

                        for (j = 0; j < word_len; j += 1) {
                            attr         = array_item(event->line_attrs, old_col + j - 1);
                            attr->flags |= key.flags;
                            attr->fg     = key.fg;
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
                            attr->flags &= ~(ATTR_BOLD);
                            attr->flags |= key.flags;
                            attr->fg     = key.fg;
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
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= num.flags;
                    attr->fg     = num.fg;
                }
                for (i = 0; i < word_len; i += 1) {
                    attr         = array_item(event->line_attrs, old_col + i - 1);
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= num.flags;
                    attr->fg     = num.fg;
                }
            }
        }

        /* NULL */
        if (!match) {
            if (word_len == 4 && strncmp(word, "NULL", 4) == 0) {
                match = 1;

                for (i = 0; i < 4; i += 1) {
                    attr         = array_item(event->line_attrs, old_col + i - 1);
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= con.flags;
                    attr->fg     = con.fg;
                }
            }
        }

        /* Function calls */
        if (!match) {
            if (old_col + word_len - 1 < array_len(line->chars)
            &&  yed_line_col_to_char(line, old_col + word_len) == '(') {
                for (i = 0; i < word_len; i += 1) {
                    attr         = array_item(event->line_attrs, old_col + i - 1);
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= cal.flags;
                    attr->fg     = cal.fg;
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
                            attr->flags &= ~(ATTR_BOLD);
                            attr->flags |= cha.flags;
                            attr->fg     = cha.fg;
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
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= str.flags;
                    attr->fg     = str.fg;
                }
            }

            i += j + 1;

        /*
         * Highlight C++-style comments.
         */
        } else if (*word == '/' && *(word + 1) == '/') {
            for (k = i; k < array_len(line->chars); k += 1) {
                attr         = array_item(event->line_attrs, k);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= com.flags;
                attr->fg     = com.fg;
            }
            break;

        /*
         * Highlight single-line C-style comments.
         */
        } else if (*word == '/' && *(word + 1) == '*') {
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
                    attr->flags &= ~(ATTR_BOLD);
                    attr->flags |= com.flags;
                    attr->fg     = com.fg;
                }
            }

            i += j + 1;
        } else {
            i += 1;
        }
    }

    /*
     * See if there's a delimiter of a multi-line C-style comment.
     */
    delim = syntax_c_line_has_comment_delim(line);

    if (delim) {
        if (delim < 0) {
            for (k = 0; k < (0 - delim + 1); k += 1) {
                attr         = array_item(event->line_attrs, k);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= com.flags;
                attr->fg     = com.fg;
            }
        } else if (delim > 0) {
            for (k = delim - 1; k < array_len(line->chars); k += 1) {
                attr         = array_item(event->line_attrs, k);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= com.flags;
                attr->fg     = com.fg;
            }
        }
    }
}

void syntax_c_find_comment_lines(yed_frame *frame) {
    yed_line  *line;
    int        row, delim, i, visited, currently_in_comment;

    memset(lines_in_comment, 0, sizeof(lines_in_comment));

    row     = frame->buffer_y_offset + 1;
    visited = 0;

    lines_in_comment[visited] = 0;

    if (row == 1) {
        line = yed_buff_get_line(frame->buffer, 1);
        if (line) {
            delim = syntax_c_line_has_comment_delim(line);

            if (delim > 0) {
                lines_in_comment[visited] = 1;
            }
        }
    } else {
        for (i = row - 1; i >= 1; i -= 1) {
            /* Give up after 100 lines. */
            if (row - i > 100)    { break; }

            line = yed_buff_get_line(frame->buffer, i);

            if (!line)    { break; }

            delim = syntax_c_line_has_comment_delim(line);

            if (delim != 0) {
                if (delim > 0) {
                    lines_in_comment[visited] = 1;
                }
                break;
            }
        }
    }

    currently_in_comment = (lines_in_comment[visited]);

    row     += 1;
    visited += 1;

    bucket_array_traverse_from(frame->buffer->lines, line, frame->buffer_y_offset) {
        delim = syntax_c_line_has_comment_delim(line);

        if (delim < 0) {
            currently_in_comment = 0;
        }

        lines_in_comment[visited] = currently_in_comment;

        if (currently_in_comment) {
            lines_in_comment[visited] = 1;
        }

        if (delim > 0) {
            currently_in_comment = 1;
        }

        if (visited == frame->height)    { break; }

        visited += 1;
        row     += 1;
    }

    last_frame = frame;
}

int syntax_c_line_has_comment_delim(yed_line *line) {
    char c, *start, *data, *end;
    int  len;
    int  open, delim;

    start = data = array_data(line->chars);
    len   = array_len(line->chars);

    if (!len)    { return 0; }

    end = start + len;

    delim = 0;
    open  = 0;
    for (; data < end - 1; data += 1) {
        c = *data;
        if (c == '/') {
            if (*(data + 1) == '*') {
                open  += 1;
                delim  = data - start + 1;
                data  += 1; /* skip the star so it isn't counted in a close */
            }
        } else if (c == '*') {
            if (*(data + 1) == '/') {
                open  -= 1;
                delim  = -(data - start + 1);
                data  += 1; /* skip the slash so it isn't counted in an open */
            }
        }
    }

    return (!!open) * delim;
}
