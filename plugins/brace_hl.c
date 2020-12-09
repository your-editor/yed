#include <yed/plugin.h>

void brace_hl_cursor_moved_handler(yed_event *event);
void brace_hl_line_handler(yed_event *event);
void brace_hl_buff_mod_handler(yed_event *event);

void brace_hl_find_braces(yed_frame *frame);
void brace_hl_hl_braces(yed_event *event);

static int beg_row;
static int beg_col;
static int end_row;
static int end_col;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler cursor_moved, line;

    YED_PLUG_VERSION_CHECK();

    cursor_moved.kind  = EVENT_CURSOR_MOVED;
    cursor_moved.fn    = brace_hl_cursor_moved_handler;
    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = brace_hl_line_handler;

    yed_plugin_add_event_handler(self, cursor_moved);
    yed_plugin_add_event_handler(self, line);

    return 0;
}

void brace_hl_cursor_moved_handler(yed_event *event) {
    yed_frame *frame;
    int        save_beg_row, save_end_row;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    save_beg_row = beg_row;
    save_end_row = end_row;

    brace_hl_find_braces(event->frame);

    if (beg_row != save_beg_row
    ||  end_row != save_end_row) {

        frame->dirty = 1;
    }
}

void brace_hl_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  frame != ys->active_frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    brace_hl_hl_braces(event);
}

void brace_hl_buff_mod_handler(yed_event *event) {
    yed_frame *frame;
    int        save_beg_row, save_end_row;

    frame = event->frame;

    if (!frame
    ||  frame != ys->active_frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    save_beg_row = beg_row;
    save_end_row = end_row;

    brace_hl_find_braces(event->frame);

    if (beg_row != save_beg_row
    ||  end_row != save_end_row) {

        frame->dirty = 1;
    }
}

void brace_hl_find_braces(yed_frame *frame) {
    int        row, col;
    int        first_vis_row, last_vis_row;
    int        balance;
    yed_line  *line;
    yed_glyph *g;

    beg_row = beg_col = end_row = end_col = 0;

    row           = frame->cursor_line;
    col           = frame->cursor_col;
    first_vis_row = frame->buffer_y_offset + 1;
    last_vis_row  = MIN(frame->buffer_y_offset + frame->height,
                        bucket_array_len(frame->buffer->lines));
    balance       = 0;

    /* Scan backwards. */
    for (; row >= first_vis_row; row -= 1) {
        line = yed_buff_get_line(frame->buffer, row);

        if (line->visual_width == 0) { continue; }

        if (row == frame->cursor_line) {
            if (col == 1) { continue; }
            col -= 1;
        } else {
            col = line->visual_width;
        }

        while (col > 0) {
            g = yed_line_col_to_glyph(line, col);

            if (g->c == '{') {
                if (balance == 0) {
                    beg_row = row;
                    beg_col = col;
                    goto done_back;
                } else {
                    balance += 1;
                }
            } else if (g->c == '}') {
                balance -= 1;
            }

            col -= yed_get_glyph_len(*g);
        }
    }
done_back:

    row     = frame->cursor_line;
    col     = frame->cursor_col;
    balance = 0;

    /* Scan forwards. */
    for (; row <= last_vis_row; row += 1) {
        line = yed_buff_get_line(frame->buffer, row);

        if (line->visual_width == 0) { continue; }

        if (row != frame->cursor_line) {
            col = 1;
        }

        while (col <= line->visual_width) {
            g = yed_line_col_to_glyph(line, col);

            if (g->c == '{') {
                balance += 1;
            } else if (g->c == '}') {
                if (balance == 0) {
                    end_row = row;
                    end_col = col;
                    goto done_forward;
                } else {
                    balance -= 1;
                }
            }

            col += yed_get_glyph_len(*g);
        }
    }

done_forward:
    return;
}

void brace_hl_hl_braces(yed_event *event) {
    yed_attrs *attr;
    yed_attrs  atn;

    if (beg_row && beg_col && beg_row == event->row) {
        atn  = yed_active_style_get_attention();
        attr = array_item(event->frame->line_attrs, beg_col - 1);
        yed_combine_attrs(attr, &atn);
    }

    if (end_row && end_col && end_row == event->row) {
        atn  = yed_active_style_get_attention();
        attr = array_item(event->frame->line_attrs, end_col - 1);
        yed_combine_attrs(attr, &atn);
    }
}
