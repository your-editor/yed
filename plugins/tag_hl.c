#include "plugin.h"

void tag_hl_line_handler(yed_event *event);
void tag_hl_hl_tags(yed_event *event);


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = tag_hl_line_handler;

    yed_plugin_add_event_handler(self, line);

    return 0;
}

void tag_hl_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  frame != ys->active_frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    tag_hl_hl_tags(event);
}

void tag_hl_hl_tags(yed_event *event) {
    yed_line  *line;
    yed_attrs *attr;
    yed_attrs  atn;
    int        last_was_at, i,
               col, old_col, spaces, word_len;
    char       c, *word;

    line         = yed_buff_get_line(event->frame->buffer, event->row);
    last_was_at  = 0;
    atn          = yed_active_style_get_attention();
    col          = 1;

    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        spaces   = 0;
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

        if (last_was_at) {
            attr         = array_item(event->line_attrs, old_col - 2);
            attr->flags &= ~(ATTR_BOLD);
            attr->flags |= atn.flags;
            attr->fg     = atn.fg;

            for (i = 0; i < word_len; i += 1) {
                attr         = array_item(event->line_attrs, old_col + i - 1);
                attr->flags &= ~(ATTR_BOLD);
                attr->flags |= atn.flags;
                attr->fg     = atn.fg;
            }
        }

        last_was_at = !spaces && word[word_len - 1] == '@';
    }
}
