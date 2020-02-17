#include <yed/plugin.h>
#include <yed/highlight.h>

highlight_info hinfo;

void unload(yed_plugin *self);
void tag_hl_line_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = tag_hl_line_handler;

    yed_plugin_set_unload_fn(self, unload);
    yed_plugin_add_event_handler(self, line);

    highlight_info_make(&hinfo);

    highlight_prefixed_words_inclusive(&hinfo, '@', HL_ATTN);

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void tag_hl_line_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_glyph *g;
    int        found;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, event->row);

    /* Quick pass to make this faster since there usually won't be tags. */
    found = 0;
    yed_line_glyph_traverse(*line, g) {
        if (g->c == '@') {
            found = 1;
            break;
        }
    }

    if (found) {
        highlight_line(&hinfo, event);
    }
}
