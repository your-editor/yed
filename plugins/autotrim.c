#include "plugin.h"

void autotrim_pre_write_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_BUFFER_PRE_WRITE;
    h.fn   = autotrim_pre_write_handler;

    yed_plugin_add_event_handler(self, h);

    return 0;
}

void autotrim_pre_write_handler(yed_event *event) {
    yed_frame *f;
    yed_line  *line;
    int        cursor_col;

    f = NULL;
    if (ys->active_frame && ys->active_frame->buffer == event->buffer) {
        f = ys->active_frame;
        cursor_col = ys->active_frame->cursor_col;
        yed_set_cursor_within_frame(ys->active_frame, 1, ys->active_frame->cursor_line);
    }

    bucket_array_traverse(event->buffer->lines, line) {
        while (array_len(line->chars)
        &&     *(char*)array_last(line->chars) == ' ') {
            array_pop(line->chars);
            line->visual_width -= 1;
        }
    }

    if (f) {
        yed_set_cursor_within_frame(ys->active_frame, cursor_col, ys->active_frame->cursor_line);
    }
}
