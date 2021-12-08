#include <yed/plugin.h>
#include <yed/syntax.h>

static yed_syntax syn;

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

#define _CHECK(x, r)                                                      \
do {                                                                      \
    if (x) {                                                              \
        LOG_FN_ENTER();                                                   \
        yed_log("[!] " __FILE__ ":%d regex error for '%s': %s", __LINE__, \
                r,                                                        \
                yed_syntax_get_regex_err(&syn));                          \
        LOG_EXIT();                                                       \
    }                                                                     \
} while (0)

#define SYN()          yed_syntax_start(&syn)
#define ENDSYN()       yed_syntax_end(&syn)
#define APUSH(s)       yed_syntax_attr_push(&syn, s)
#define APOP(s)        yed_syntax_attr_pop(&syn)
#define RANGE(r)       _CHECK(yed_syntax_range_start(&syn, r), r)
#define ONELINE()      yed_syntax_range_one_line(&syn)
#define SKIP(r)        _CHECK(yed_syntax_range_skip(&syn, r), r)
#define ENDRANGE(r)    _CHECK(yed_syntax_range_end(&syn, r), r)
#define REGEX(r)       _CHECK(yed_syntax_regex(&syn, r), r)
#define REGEXSUB(r, g) _CHECK(yed_syntax_regex_sub(&syn, r, g), r)
#define KWD(k)         yed_syntax_kwd(&syn, k)

#ifdef __APPLE__
#define WBE "[[:<:]]"
#define WBS "[[:>:]]"
#else
#define WBE "\\b"
#define WBS "\\b"
#endif

void estyle(yed_event *event)   { yed_syntax_style_event(&syn, event);         }
void ebuffdel(yed_event *event) { yed_syntax_buffer_delete_event(&syn, event); }
void ebuffmod(yed_event *event) { yed_syntax_buffer_mod_event(&syn, event);    }
void eline(yed_event *event)  {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("Jgraph")) {
        return;
    }

    yed_syntax_line_event(&syn, event);
}


void unload(yed_plugin *self) {
    yed_syntax_free(&syn);
    ys->redraw = 1;
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler style;
    yed_event_handler buffdel;
    yed_event_handler buffmod;
    yed_event_handler line;

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
        "epilogue", "include", "shell",
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

    style.kind = EVENT_STYLE_CHANGE;
    style.fn   = estyle;
    yed_plugin_add_event_handler(self, style);

    buffdel.kind = EVENT_BUFFER_PRE_DELETE;
    buffdel.fn   = ebuffdel;
    yed_plugin_add_event_handler(self, buffdel);

    buffmod.kind = EVENT_BUFFER_POST_MOD;
    buffmod.fn   = ebuffmod;
    yed_plugin_add_event_handler(self, buffmod);

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = eline;
    yed_plugin_add_event_handler(self, line);


    SYN();
        APUSH("&code-comment");
            RANGE("\\(\\*"); ENDRANGE("\\*\\)");
        APOP();

        APUSH("&code-string");
            RANGE("\""); SKIP("\\\\\"");            ENDRANGE("\"");
            RANGE("'");  SKIP("\\\\'");             ENDRANGE("'");
            RANGE(":");  SKIP("\\\\[[:space:]]*$"); ENDRANGE("$");
        APOP();

        APUSH("&code-number");
            REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+)))"WBS, 2);
            REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+)"WBS, 2);
        APOP();

        APUSH("&code-keyword");
            ARRAY_LOOP(edit_kwds) KWD(*it);
        APOP();

        APUSH("&code-constant");
            ARRAY_LOOP(value_kwds) KWD(*it);
        APOP();
    ENDSYN();


    ys->redraw = 1;

    return 0;
}
