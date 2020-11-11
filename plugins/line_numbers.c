#include <yed/plugin.h>

void line_numbers_line_handler(yed_event *event);
void unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = line_numbers_line_handler;

    yed_plugin_add_event_handler(self, line);
    yed_plugin_set_unload_fn(self, unload);

    ys->redraw = 1;

    return 0;
}

static int n_digits(int i) {
    int n;
    int x;

    if (i <= 0) { return 1; }

    n = 1;
    x = 10;

    while (i / x) {
        n += 1;
        x *= 10;
    }

    return n;
}

void line_numbers_line_handler(yed_event *event) {
    int  n_lines;
    int  n_cols;
    char num_buff[16];

    if (event->frame->buffer == NULL)         { return; }
    if (event->frame->buffer->name
    &&  event->frame->buffer->name[0] == '*') { return; }

    n_lines = yed_buff_n_lines(event->frame->buffer);
    n_cols  = n_digits(n_lines) + 2;

    if (event->frame->gutter_width != n_cols) {
        yed_frame_set_gutter_width(event->frame, n_cols);
    }

    snprintf(num_buff, sizeof(num_buff),
             " %*d ", n_cols - 2, event->row);

    array_clear(event->gutter_glyphs);
    array_push_n(event->gutter_glyphs, num_buff, strlen(num_buff));
}

void unload(yed_plugin *self) {
    yed_frame **fit;

    array_traverse(ys->frames, fit) {
        yed_frame_set_gutter_width(*fit, 0);
    }
}
