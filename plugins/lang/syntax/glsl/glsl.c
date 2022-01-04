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
#define WB "[[:>:]]"
#else
#define WB "\\b"
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
    ||  frame->buffer->ft != yed_get_ft("GLSL")) {
        return;
    }

    yed_syntax_line_event(&syn, event);
}


void unload(yed_plugin *self) {
    yed_syntax_free(&syn);
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler style;
    yed_event_handler buffdel;
    yed_event_handler buffmod;
    yed_event_handler line;

    char              *kwds[] = {
        "buffer", "const", "discard", "in", "layout", "out", "struct",
    };
    char              *control_flow[] = {
        "break", "case", "continue", "default", "do", "else", "for", "if", "return", "switch", "while",
    };
    char              *typenames[] = {
        "bool", "double", "float", "int", "uint", "void",
        "mat2", "mat2x2", "mat2x3", "mat2x4",
        "mat3", "mat3x2", "mat3x3", "mat3x4",
        "mat4", "mat4x2", "mat4x3", "mat4x4",
        "sampler", "sampler2D", "sampler3D",
        "uniform", "gsampler", "gradvec",
        "vec2",     "vec3",    "vec4",
        "bvec2",     "bvec3",    "bvec4",
        "ivec2",     "ivec3",    "ivec4",
        "uvec2",     "uvec3",    "uvec4",
        "dvec2",     "dvec3",    "dvec4",
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
            RANGE("/\\*");
            ENDRANGE(  "\\*/");
            RANGE("//");
            ENDRANGE("$");
            RANGE("^[[:space:]]*#[[:space:]]*if[[:space:]]+0"WB);
            ENDRANGE("^[[:space:]]*#[[:space:]]*(else|endif|elif|elifdef)"WB);
        APOP();

        APUSH("&code-fn-call");
            REGEXSUB("([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
        APOP();

        APUSH("&code-number");
            REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?)"WB, 2);
            REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
        APOP();

        APUSH("&code-keyword");
            ARRAY_LOOP(kwds) KWD(*it);
        APOP();

        APUSH("&code-control-flow");
            ARRAY_LOOP(control_flow) KWD(*it);
        APOP();

        APUSH("&code-typename");
            ARRAY_LOOP(typenames) KWD(*it);
        APOP();

        APUSH("&code-preprocessor");
            REGEX("^[[:space:]]*#[[:space:]]*(define|elif|else|endif|extension|if|ifdef|ifndef|line|undef|version)"WB);
        APOP();

        APUSH("&code-field");
            REGEXSUB("(\\.|->)[[:space:]]*([[:alpha:]_][[:alnum:]_]*)", 2);
        APOP();
    ENDSYN();

    return 0;
}
