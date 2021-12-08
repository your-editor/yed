#include <yed/plugin.h>
#include <yed/syntax.h>

static yed_syntax syn;


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


void syntax_yedrc_highlight(yed_event *event);

void estyle(yed_event *event)   { yed_syntax_style_event(&syn, event);         }
void ebuffdel(yed_event *event) { yed_syntax_buffer_delete_event(&syn, event); }
void ebuffmod(yed_event *event) { yed_syntax_buffer_mod_event(&syn, event);    }
void eline(yed_event *event)  {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("yedrc")) {
        return;
    }

    syntax_yedrc_highlight(event);
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
            RANGE("#"); ONELINE(); ENDRANGE("$");
        APOP();

        APUSH("&code-string");
            RANGE("\""); SKIP("\\\\\""); ENDRANGE("\"");
            RANGE("'");  SKIP("\\\\'");  ENDRANGE("'");
        APOP();

        APUSH("&code-number");
            REGEXSUB("(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+)))"WB, 2);
            REGEXSUB("(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
        APOP();
    ENDSYN();

    ys->redraw = 1;

    return 0;
}

void syntax_yedrc_highlight(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, key, ty;
    int        col, old_col, word_len, i;
    char       c, *word, *word_cpy;

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    key = yed_active_style_get_code_keyword();
    ty  = yed_active_style_get_code_typename();

    col = 1;

    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        c        = yed_line_col_to_glyph(line, col)->c;

        if (c == ')') { return; }

        if (is_alnum(c) || c == '_' || c == '-') {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (!is_alnum(c) && c != '_' && c != '-') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (is_alnum(c) || c == '_' || c == '-' || is_space(c)) {
                    break;
                }
            }
        }

        if (is_space(c)) {
            while (col <= line->visual_width) {
                col += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (!is_space(c)) { break; }
            }
        }


        /*
         * Try to match keywords.
         */
        word_cpy = strndup(word, word_len);
        if (!!yed_get_command(word_cpy)) {
            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &key);
            }
        } else if (yed_get_ft(word_cpy) != FT_ERR_NOT_FOUND) {
            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &ty);
            }
        }
        free(word_cpy);
    }
}
