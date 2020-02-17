#include <yed/plugin.h>
#include <yed/highlight.h>

void unload(yed_plugin *self);
void syntax_yedrc_line_handler(yed_event *event);
void syntax_yedrc_highlight(yed_event *event);

highlight_info hinfo;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    yed_plugin_set_unload_fn(self, unload);

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_yedrc_line_handler;

    yed_plugin_add_event_handler(self, h);

    highlight_info_make(&hinfo);

    highlight_numbers(&hinfo);
    highlight_to_eol_from(&hinfo, "#", HL_COMMENT);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', -1, HL_STR);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
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
    highlight_line(&hinfo, event);
}

void syntax_yedrc_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, key;
    int        col, old_col, word_len, spaces, i;
    char       c, *word, *word_cpy;

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    key = yed_active_style_get_code_keyword();

    col = 1;

    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        spaces   = 0;
        c        = yed_line_col_to_glyph(line, col)->c;

        if (isalnum(c) || c == '_' || c == '-') {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_glyph(line, col)->c;

                if (!isalnum(c) && c != '_' && c != '-') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;
                c         = yed_line_col_to_glyph(line, col)->c;

                if (isalnum(c) || c == '_' || c == '-' || isspace(c)) {
                    break;
                }
            }
        }

        if (isspace(c)) {
            while (col <= line->visual_width) {
                spaces += 1;
                col    += 1;
                c       = yed_line_col_to_glyph(line, col)->c;

                if (!isspace(c)) {
                    break;
                }
            }
        }


        /*
         * Try to match keywords.
         */
        word_cpy = strndup(word, word_len);
        if (!!yed_get_command(word_cpy)) {
            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &key);
            }
        }
        free(word_cpy);
    }
}
