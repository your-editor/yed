#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo1;
highlight_info hinfo2;

void unload(yed_plugin *self);
void log_hl_line_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler line;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = log_hl_line_handler;

    yed_plugin_add_event_handler(self, line);

    highlight_info_make(&hinfo1);

    highlight_within(&hinfo1, "[", "]", 0, -1, HL_NUM);
    highlight_to_eol_from(&hinfo1, "#", HL_COMMENT);
    highlight_within(&hinfo1, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo1, "'", "'", '\\', -1, HL_CHAR);

    highlight_info_make(&hinfo2);

    highlight_to_eol_from(&hinfo2, "[!]", HL_ATTN);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo2);
    highlight_info_free(&hinfo1);
    ys->redraw = 1;
}

void log_hl_hl_cmds(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    yed_attrs *attr, cal;
    int        col, old_col, word_len, i;
    char       c, *word, *word_cpy;

    frame = event->frame;
    line  = yed_buff_get_line(frame->buffer, event->row);

    if (!line->visual_width) { return; }

    cal = yed_active_style_get_code_fn_call();

    col = 1;

    while (col <= line->visual_width) {
        old_col  = col;
        word     = array_data(line->chars) + col - 1;
        word_len = 0;
        c        = yed_line_col_to_glyph(line, col)->c;

        if (c == ')') { return; }

        if (isalnum(c) || c == '_' || c == '-') {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (!isalnum(c) && c != '_' && c != '-') {
                    break;
                }
            }
        } else {
            while (col <= line->visual_width) {
                word_len += 1;
                col      += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (isalnum(c) || c == '_' || c == '-' || isspace(c)) {
                    break;
                }
            }
        }

        if (isspace(c)) {
            while (col <= line->visual_width) {
                col += 1;

                if (col > line->visual_width) { break; }

                c = yed_line_col_to_glyph(line, col)->c;

                if (!isspace(c)) { break; }
            }
        }


        /*
         * Try to match keywords.
         */
        word_cpy = strndup(word, word_len);
        if (!!yed_get_command(word_cpy)) {
            for (i = 0; i < word_len; i += 1) {
                attr = array_item(event->line_attrs, old_col + i - 1);
                yed_combine_attrs(attr, &cal);
            }
        }
        free(word_cpy);
    }
}

void log_hl_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  frame->buffer != ys->log_buff) {
        return;
    }

    log_hl_hl_cmds(event);
    highlight_line(&hinfo1, event);
    highlight_line(&hinfo2, event);
}
