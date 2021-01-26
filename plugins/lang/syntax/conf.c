#include <yed/plugin.h>
#include <yed/highlight.h>

void unload(yed_plugin *self);
void syntax_conf_line_handler(yed_event *event);

highlight_info hinfo;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = syntax_conf_line_handler;

    yed_plugin_add_event_handler(self, h);

    highlight_info_make(&hinfo);

    highlight_to_eol_from(&hinfo, "#", HL_COMMENT);
    highlight_numbers(&hinfo);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', -1, HL_STR);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void syntax_conf_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Config")) {
        return;
    }

    highlight_line(&hinfo, event);
}
