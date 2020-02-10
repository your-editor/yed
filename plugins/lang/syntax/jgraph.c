#include <yed/plugin.h>

void syntax_jgraph_line_handler(yed_event *event);
void syntax_jgraph_frame_handler(yed_event *event);
void syntax_jgraph_buff_mod_pre_handler(yed_event *event);
void syntax_jgraph_buff_mod_post_handler(yed_event *event);

void syntax_jgraph_highlight(yed_event *event);
void syntax_jgraph_find_comment_lines(yed_frame *frame);
int  syntax_jgraph_line_has_comment_delim(yed_line *line);

static int lines_in_comment[4096];
static yed_frame *last_frame;
static int line_has_delim;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;

    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_jgraph_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_jgraph_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_jgraph_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_jgraph_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);

    ys->redraw = 1;

    return 0;
}

void syntax_jgraph_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_JGRAPH) {
        return;
    }

    syntax_jgraph_highlight(event);
}

void syntax_jgraph_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_JGRAPH) {
        return;
    }

    syntax_jgraph_find_comment_lines(frame);
}

void syntax_jgraph_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_JGRAPH) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line) {
        return;
    }

    line_has_delim = syntax_jgraph_line_has_comment_delim(line);
}

void syntax_jgraph_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    int        delim;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_JGRAPH) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line) {
        return;
    }

    delim = syntax_jgraph_line_has_comment_delim(line);

    if (line_has_delim != delim) {
        frame->dirty = 1;
    }
}

void syntax_jgraph_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, cal, num;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_minus, delim;
    char       c, *word,
              *kwds[][5] = {
                  { "x",        "y",        NULL,       NULL,       NULL       }, /* 1  */
                  { NULL,       NULL,       NULL,       NULL,       NULL       }, /* 2  */
                  { "box",      "eps",      "pts",      NULL,       NULL       }, /* 3  */
                  { "none",     "text",     "xbar",     "ybar",     NULL       }, /* 4  */
                  { "cross",    "solid",    "xaxis",    "yaxis",    NULL       }, /* 5  */
                  { "circle",   "dashed",   "dotted",   NULL,       NULL       }, /* 6  */
                  { "diamond",  "dotdash",  "ellipse",  "general",  NULL       }, /* 7  */
                  { "longdash", "marktype", "newcurve", "newgraph", "triangle" }, /* 8  */
                  { "dotdotdash", "postscript",   NULL, NULL,       NULL       }, /* 9  */
                  { NULL,       NULL,       NULL,       NULL,       NULL       }, /* 10 */
                  { NULL,       NULL,       NULL,       NULL,       NULL       }, /* 11 */
                  { NULL,       NULL,       NULL,       NULL,       NULL       }, /* 12 */
                  { NULL,       NULL,       NULL,       NULL,       NULL       }, /* 13 */
                  { "dotdotdashdash", NULL, NULL,       NULL,       NULL       }  /* 14 */
              };

    #define MIN_KWD_LEN   (1)
    #define MAX_KWD_LEN   (14)

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    cal = yed_active_style_get_code_fn_call();
    num = yed_active_style_get_code_number();

    col   = 1;

    if (frame != last_frame) {
        syntax_jgraph_find_comment_lines(frame);
    }

    if (lines_in_comment[event->row - frame->buffer_y_offset]) {
        for (k = 0 ; k < array_len(line->chars); k += 1) {
            attr = array_item(event->line_attrs, k);
            yed_combine_attrs(attr, &com);
        }
        return;
    }

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

        /* Commands */
        if (!match) {
            if (word_len == 8 && strncmp(word, "newgraph", 8) == 0) {
                match = 1;

                for (i = 0; i < 8; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &cal);
                }
            }
        }
        if (!match) {
            if (word_len == 8 && strncmp(word, "newcurve", 8) == 0) {
                match = 1;

                for (i = 0; i < 8; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &cal);
                }
            }
        }
        if (!match) {
            if (word_len == 8 && strncmp(word, "marktype", 8) == 0) {
                match = 1;

                for (i = 0; i < 8; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &cal);
                }
            }
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

        last_was_minus = !spaces && word_len == 1 && (strncmp(word, "-", 1) == 0);
    }

    match = 0;

    /*
     * Highlight single-line C-style comments.
     */
    i = 0;
    while (i < array_len(line->chars) - 1) {
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
                    attr = array_item(event->line_attrs, k);
                    yed_combine_attrs(attr, &com);
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
    delim = syntax_jgraph_line_has_comment_delim(line);

    if (delim) {
        if (delim < 0) {
            for (k = 0; k < (0 - delim + 1); k += 1) {
                attr = array_item(event->line_attrs, k);
                yed_combine_attrs(attr, &com);
            }
        } else if (delim > 0) {
            for (k = delim - 1; k < array_len(line->chars); k += 1) {
                attr = array_item(event->line_attrs, k);
                yed_combine_attrs(attr, &com);
            }
        }
    }
}

void syntax_jgraph_find_comment_lines(yed_frame *frame) {
    yed_line  *line;
    int        row, delim, i, visited, currently_in_comment;

    memset(lines_in_comment, 0, sizeof(lines_in_comment));

    row     = frame->buffer_y_offset + 1;
    visited = 0;

    lines_in_comment[visited] = 0;

    if (row == 1) {
        line = yed_buff_get_line(frame->buffer, 1);
        if (line) {
            delim = syntax_jgraph_line_has_comment_delim(line);

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

            delim = syntax_jgraph_line_has_comment_delim(line);

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
        delim = syntax_jgraph_line_has_comment_delim(line);

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

int syntax_jgraph_line_has_comment_delim(yed_line *line) {
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
        if (c == '(') {
            if (*(data + 1) == '*') {
                open  += 1;
                delim  = data - start + 1;
                data  += 1; /* skip the star so it isn't counted in a close */
            }
        } else if (c == '*') {
            if (*(data + 1) == ')') {
                open  -= 1;
                delim  = -(data - start + 1);
                data  += 1; /* skip the paren so it isn't counted in an open */
            }
        }
    }

    return (!!open) * delim;
}
