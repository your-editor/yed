#include <yed/plugin.h>

void syntax_yedrc_line_handler(yed_event *event);
void syntax_yedrc_highlight(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_yedrc_line_handler;

    yed_plugin_add_event_handler(self, h);

    ys->redraw = 1;

    return 0;
}

void syntax_yedrc_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_YEDRC) {
        return;
    }

    syntax_yedrc_highlight(event);
}

void syntax_yedrc_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, com, key, num, str;
    int        col, old_col, word_len, spaces, i, j, k, match, last_was_minus;
    char       c, *word, *word_cpy;

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    com = yed_active_style_get_code_comment();
    key = yed_active_style_get_code_keyword();
    num = yed_active_style_get_code_number();
    str = yed_active_style_get_code_string();

    col = 1;

    last_was_minus      = 0;
    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        spaces   = 0;
        match    = 0;
        c        = yed_line_col_to_char(line, col);

        if (isalnum(c) || c == '_' || c == '-') {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_char(line, col);

                if (!isalnum(c) && c != '_' && c != '-') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_char(line, col);

                if (isalnum(c) || c == '_' || c == '-' || isspace(c)) {
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
        if (!match) {
            word_cpy = strndup(word, word_len);
            if (!!yed_get_command(word_cpy)) {
                for (i = 0; i < word_len; i += 1) {
                    attr = array_item(event->line_attrs, old_col + i - 1);
                    yed_combine_attrs(attr, &key);
                }
                match = 1;
            }
            free(word_cpy);
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

        last_was_minus     = !spaces && word_len == 1 && (strncmp(word, "-", 1) == 0);
    }

    match = 0;

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
        } else if (*word == '\'') {
            j = 1;

            while (i + j < array_len(line->chars)) {
                c = word[j];

                if (c == '\'') {
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
