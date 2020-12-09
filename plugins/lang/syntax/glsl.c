#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo;

void unload(yed_plugin *self);
void syntax_glsl_line_handler(yed_event *event);
void syntax_glsl_frame_handler(yed_event *event);
void syntax_glsl_buff_mod_pre_handler(yed_event *event);
void syntax_glsl_buff_mod_post_handler(yed_event *event);


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;
    char              *kwds[] = {
        "bool",     "break", "buffer",
        "case",     "const",   "continue",
        "default",  "discard", "do",       "double",
        "else",
        "float",    "for",
        "gsampler", "gradvec",
        "if",       "in", "int",
        "layout",
        "mat2", "mat2x2", "mat2x3", "mat2x4",
        "mat3", "mat3x2", "mat3x3", "mat3x4",
        "mat4", "mat4x2", "mat4x3", "mat4x4",
        "out",
        "return",
        "sampler", "sampler2D", "sampler3D",
        "struct",   "switch",
        "uint", "uniform",
        "vec2",     "vec3",    "vec4",
        "bvec2",     "bvec3",    "bvec4",
        "ivec2",     "ivec3",    "ivec4",
        "uvec2",     "uvec3",    "uvec4",
        "dvec2",     "dvec3",    "dvec4",
        "void",
        "while",
    };
    char              *pp_kwds[] = {
        "define",
        "else", "endif", "error", "extension",
        "if", "ifdef", "ifndef",
        "line",
        "undef",
        "version",
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);


    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_glsl_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_glsl_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_glsl_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_glsl_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);


    highlight_info_make(&hinfo);

    ARRAY_LOOP(kwds)
        highlight_add_kwd(&hinfo, *it, HL_KEY);
    ARRAY_LOOP(pp_kwds)
        highlight_add_prefixed_kwd(&hinfo, '#', *it, HL_PP);
    highlight_add_kwd(&hinfo, "__LINE__", HL_PP);
    highlight_add_kwd(&hinfo, "__VERSION__", HL_PP);
    highlight_add_kwd(&hinfo, "gl_Position", HL_CON);
    highlight_suffixed_words(&hinfo, '(', HL_CALL);
    highlight_numbers(&hinfo);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', 1, HL_CHAR);
    highlight_to_eol_from(&hinfo, "//", HL_COMMENT);
    highlight_within_multiline(&hinfo, "/*", "*/", 0, HL_COMMENT);
    highlight_within_multiline(&hinfo, "#if 0", "#endif", 0, HL_COMMENT);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void syntax_glsl_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    || (frame->buffer->ft != yed_get_ft("GLSL"))) {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo, event);
}

void syntax_glsl_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    || (frame->buffer->ft != yed_get_ft("GLSL"))) {
        return;
    }

    highlight_line(&hinfo, event);
}

void syntax_glsl_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    || (frame->buffer->ft != yed_get_ft("GLSL"))) {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo, event);
}

void syntax_glsl_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    || (frame->buffer->ft != yed_get_ft("GLSL"))) {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo, event);
}
