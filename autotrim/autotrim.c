#include <yed/plugin.h>

void autotrim_pre_write_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_BUFFER_PRE_WRITE;
    h.fn   = autotrim_pre_write_handler;

    yed_plugin_add_event_handler(self, h);

    return 0;
}

void autotrim_pre_write_handler(yed_event *event) {
    yed_frame *f;
    yed_line  *line;
    int        row, cursor_col;

    f = NULL;

    if (ys->active_frame && ys->active_frame->buffer == event->buffer) {
        f = ys->active_frame;
    }

    yed_start_undo_record(f, event->buffer);

    if (f) {
        cursor_col = ys->active_frame->cursor_col;
        yed_set_cursor_within_frame(ys->active_frame, ys->active_frame->cursor_line, 1);
    }

    for (row = 1; row <= yed_buff_n_lines(event->buffer); row += 1) {
        line = yed_buff_get_line(event->buffer, row);
        while (line->visual_width
        &&     yed_line_col_to_glyph(line, line->visual_width)->c == ' ') {
            yed_pop_from_line(event->buffer, row);
        }
    }

    if (f) {
        yed_set_cursor_within_frame(f, f->cursor_line, cursor_col);
    }

    yed_end_undo_record(f, event->buffer);

    /*
     * *ATTENTION*
     *
     * We will merge these changes with the previous record in the
     * undo history.
     *
     * The effect of this is that the changes in the buffer will be
     * appropriately reflected in the undo history without adding
     * unnecessary undo records that the user will have to go through
     * in order to get to older buffer states.
     */

    yed_merge_undo_records(event->buffer);
}
