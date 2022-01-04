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
    ||  frame->buffer->ft != yed_get_ft("C++")) {
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
        "__asm__",  "asm", "alignas", "alignof", "and", "and_eq", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto",
        "bitand", "bitor",
        "const", "class", "compl", "concept ", "consteval ", "constexpr ", "constinit ", "const_cast",
        "decltype ", "delete", "dynamic_cast",
        "enum",     "extern", "explicit", "export",
        "friend",
        "inline",
        "mutable",
        "namespace", "new", "noexcept ", "not", "not_eq",
        "operator", "or", "or_eq",
        "private", "protected", "public",
        "restrict", "reflexpr ", "register", "reinterpret_cast", "requires ",
        "sizeof", "static", "struct",
        "static_assert ", "static_cast", "synchronized ", "typedef",
        "template", "thread_local ", "typedef", "typeid", "typename",
        "union", "using",
        "virtual", "volatile",
        "xor", "xor_eq",
    };
    char              *control_flow[] = {
        "break",
        "case", "catch", "co_await", "co_return", "co_yield", "continue",
        "default", "do",
        "else",
        "for",
        "goto",
        "if",
        "return",
        "switch",
        "throw", "try",
        "while",
    };
    char              *typenames[] = {
        "bool",
        "char", "char8_t", "char16_t", "char32_t",
        "double",
        "float",
        "long",
        "int",
        "short", "size_t", "ssize_t",
        "unsigned",
        "void",
        "wchar_t",
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
                ONELINE();
            ENDRANGE("$");
            RANGE("^[[:space:]]*#[[:space:]]*if[[:space:]]+0"WB);
            ENDRANGE("^[[:space:]]*#[[:space:]]*(else|endif|elif|elifdef)"WB);
        APOP();

        APUSH("&code-string");
            REGEX("'(\\\\.|[^'\\\\])'");

            RANGE("\""); ONELINE(); SKIP("\\\\\"");
                APUSH("&code-escape");
                    REGEX("\\\\.");
                APOP();
            ENDRANGE("\"");
        APOP();

        APUSH("&code-fn-call");
            REGEXSUB("([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
        APOP();

        APUSH("&code-number");
            REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB, 2);
            REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
            REGEXSUB("(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
        APOP();

        APUSH("&code-keyword");
            ARRAY_LOOP(kwds) KWD(*it);
        APOP();

        APUSH("&code-control-flow");
            ARRAY_LOOP(control_flow) KWD(*it);
            REGEXSUB("^[[:space:]]*([[:alpha:]_][[:alnum:]_]*):", 1);
        APOP();

        APUSH("&code-typename");
            ARRAY_LOOP(typenames) KWD(*it);
        APOP();

        APUSH("&code-preprocessor");
            KWD("__VA_ARGS__");
            KWD("__FILE__");
            KWD("__func__");
            KWD("__FUNCTION__");
            KWD("__LINE__");
            KWD("__DATE__");
            KWD("__TIME__");
            KWD("__STDC__");
            KWD("__STDC_VERSION__");
            KWD("__STDC_HOSTED__");
            KWD("__cplusplus");
            KWD("__OBJC__");
            KWD("__ASSEMBLER__");

            RANGE("^[[:space:]]*#[[:space:]]*include"); ONELINE();
                APUSH("&code-string");
                    REGEX("[<\"].*");
                APOP();
            ENDRANGE("$");

            REGEX("^[[:space:]]*#[[:space:]]*(define|elif|else|endif|error|if|ifdef|ifndef|line|message|pragma|undef|warning)"WB);
        APOP();

        APUSH("&code-constant");
            KWD("NULL");
            KWD("stdin");
            KWD("stdout");
            KWD("stderr");
            KWD("true");
            KWD("false");
            KWD("nullptr");
            KWD("this");
        APOP();

        APUSH("&code-field");
            REGEXSUB("(\\.|->)[[:space:]]*([[:alpha:]_][[:alnum:]_]*)", 2);
        APOP();
    ENDSYN();

    return 0;
}
