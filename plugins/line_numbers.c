#include <yed/plugin.h>

void line_numbers_line_handler(yed_event *event);
void line_numbers_frame_pre_update(yed_event *event);
void unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;
    yed_event_handler frame_pre_update;

    YED_PLUG_VERSION_CHECK();

    line.kind             = EVENT_LINE_PRE_DRAW;
    line.fn               = line_numbers_line_handler;
    frame_pre_update.kind = EVENT_FRAME_PRE_UPDATE;
    frame_pre_update.fn   = line_numbers_frame_pre_update;

    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, frame_pre_update);
    yed_plugin_set_unload_fn(self, unload);

    if (yed_get_var("line-number-scomp") == NULL) {
        yed_set_var("line-number-scomp", "code-comment");
    }

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

static int scomp_save = -1;

void line_numbers_line_handler(yed_event *event) {
    int        n_lines;
    int        n_cols;
    char       num_buff[16];
    yed_attrs  attr;
    yed_attrs *dst;

    if (event->frame->buffer == NULL
    ||  (event->frame->buffer->name
    &&  event->frame->buffer->name[0] == '*')) {

        yed_frame_set_gutter_width(event->frame, 0);
        return;
    }

    n_lines = yed_buff_n_lines(event->frame->buffer);
    n_cols  = n_digits(n_lines) + 2;

    if (event->frame->gutter_width != n_cols) {
        yed_frame_set_gutter_width(event->frame, n_cols);
    }

    snprintf(num_buff, sizeof(num_buff),
             " %*d ", n_cols - 2, event->row);

    array_clear(event->gutter_glyphs);
    array_push_n(event->gutter_glyphs, num_buff, strlen(num_buff));

    attr = yed_get_active_style_scomp(scomp_save);

    array_traverse(event->gutter_attrs, dst) {
        yed_combine_attrs(dst, &attr);
    }
}

void line_numbers_frame_pre_update(yed_event *event) {
    int scomp;

    scomp = yed_get_active_style_scomp_nr_by_name(yed_get_var("line-number-scomp"));
    if (scomp != scomp_save) {
        ys->redraw = 1;
    }
    scomp_save = scomp;

    if (event->frame->buffer == NULL) {
        yed_frame_set_gutter_width(event->frame, 0);
    }
}

void unload(yed_plugin *self) {
    yed_frame **fit;

    array_traverse(ys->frames, fit) {
        yed_frame_set_gutter_width(*fit, 0);
    }
}
