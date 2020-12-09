#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo1;
highlight_info hinfo2;
highlight_info hinfo3;

void unload(yed_plugin *self);
void syntax_jgraph_line_handler(yed_event *event);
void syntax_jgraph_frame_handler(yed_event *event);
void syntax_jgraph_buff_mod_pre_handler(yed_event *event);
void syntax_jgraph_buff_mod_post_handler(yed_event *event);


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;
    char              *edit_kwds[] = {
        "x", "y", "fontsize", "linesep",
        "hjl", "hjc", "hjr", "vjc", "vjt", "vjb",
        "font", "rotate", "lgray", "lcolor", "y_epts",
        "pts", "x_epts", "label", "marksize", "gmarks",
        "pfill", "pcfill", "fill", "cfill", "afill", "acfill",
        "marktype", "glines", "pattern", "apattern", "ppattern",
        "linetype", "linethickness", "gray", "color", "mrotate",
        "poly", "nopoly", "nopoly", "bezier",
        "nobezier", "asize", "clip", "noclip", "at", "size", "max",
        "min", "hash", "shash", "mhash", "precision", "label",
        "hash_format", "hash_labels", "log_base", "draw_at", "nodraw", "draw", "hash_at", "mhash_at", "hash_label",
        "hash_scale", "auto_hash_marks", "no_auto_hash_marks",
        "auto_hash_labels", "no_auto_hash_labels", "draw_hash_labels_at",
        "draw_hash_marks_at", "no_draw_hash_labels", "draw_hash_labels",
        "no_draw_axis_line", "draw_axis_line", "no_draw_axis", "draw_axis",
        "no_draw_hash_marks", "draw_hash_marks", "no_draw_axis_label",
        "draw_axis_label", "no_grid_lines", "grid_lines", "no_mgrid_lines",
        "mgrid_lines", "grid_gray", "grid_color", "mgrid_gray",
        "mgrid_color", "linelength", "columns", "columnsep", "linebreak",
        "midspace", "defaults", "xaxis", "yaxis", "curve", "newcurve",
        "copycurve", "newline", "title", "legend", "x_translate",
        "y_translate", "string", "newstring", "copystring",
        "copyline", "inherit_axes", "Y", "X", "border", "noborder", "clip",
        "noclip", "graph", "newgraph", "copygraph", "bbox", "preample",
        "epilogue",
    };
    char              *value_kwds[] = {
        "none", "solid", "dotted", "dashed", "longdash", "dotdash",
        "dotdotdash", "dotdotdashdash", "larrow",
        "nolarrow", "rarrow", "norarrow", "larrows", "nolarrows",
        "rarrows", "norarrows", "log", "linear",
        "right", "left", "top", "bottom", "on", "off", "custom",
        "circle", "box", "diamond", "triangle", "x", "cross",
        "ellipse", "general", "general_nf", "general_bez",
        "general_bez_nf", "postscript", "eps",
        "xbar", "ybar", "text", "stripe", "estripe"
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);


    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_jgraph_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_jgraph_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_jgraph_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_jgraph_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);

    highlight_info_make(&hinfo1);

    highlight_add_kwd(&hinfo1, "shell", HL_KEY);


    highlight_info_make(&hinfo2);

    ARRAY_LOOP(edit_kwds)
        highlight_add_kwd(&hinfo2, *it, HL_KEY);
    ARRAY_LOOP(value_kwds)
        highlight_add_kwd(&hinfo2, *it, HL_CON);
    highlight_add_kwd(&hinfo2, "include", HL_PP);
    highlight_to_eol_from(&hinfo2, ":", HL_CALL);
    highlight_to_eol_from(&hinfo2, "shell", HL_IGNORE);
    highlight_numbers(&hinfo2);

    highlight_info_make(&hinfo3);

    highlight_within_multiline(&hinfo3, "\"", "\"", '\\', HL_STR);
    highlight_within_multiline(&hinfo3, "'", "'", '\\', HL_CHAR);
    highlight_within_multiline(&hinfo3, "(*", "*)", 0, HL_COMMENT);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo2);
    highlight_info_free(&hinfo1);
    ys->redraw = 1;
}

void syntax_jgraph_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Jgraph")) {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo2, event);
}

void syntax_jgraph_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Jgraph")) {
        return;
    }

    highlight_line(&hinfo1, event);
    highlight_line(&hinfo2, event);
    highlight_line(&hinfo3, event);
}

void syntax_jgraph_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Jgraph")) {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo3, event);
}

void syntax_jgraph_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Jgraph")) {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo3, event);
}
