#include <yed/plugin.h>
#include <yed/highlight.h>

highlight_info hinfo;

void unload(yed_plugin *self);
void syntax_latex_line_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler  line;

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = syntax_latex_line_handler;

    yed_plugin_set_unload_fn(self, unload);
    yed_plugin_add_event_handler(self, line);

    highlight_info_make(&hinfo);

    highlight_prefixed_words_inclusive(&hinfo, '\\', HL_CALL);
    highlight_within(&hinfo, "$", "$", '\\', -1, HL_STR);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void syntax_latex_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_LATEX) {
        return;
    }

    highlight_line(&hinfo, event);
}
