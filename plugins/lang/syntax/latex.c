#include <yed/plugin.h>
#include <yed/highlight.h>

highlight_info hinfo1;
highlight_info hinfo2;

void unload(yed_plugin *self);
void syntax_latex_line_handler(yed_event *event);
void syntax_latex_highlight_comments(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler  line;

    YED_PLUG_VERSION_CHECK();

    line.kind = EVENT_LINE_PRE_DRAW;
    line.fn   = syntax_latex_line_handler;

    yed_plugin_set_unload_fn(self, unload);
    yed_plugin_add_event_handler(self, line);

    highlight_info_make(&hinfo1);
    highlight_info_make(&hinfo2);

    highlight_within(&hinfo1, "$", "$", '\\', -1, HL_STR);
    highlight_within(&hinfo1, "``", "''", 0, -1, HL_CON);
    highlight_within(&hinfo1, "`", "'", 0, -1, HL_CON);

    /*
    ** This can't properly handle escaped % characters..
    ** See syntax_latex_highlight_comments()
    */
/*     highlight_to_eol_from(&hinfo1, "%", HL_COMMENT); */


    /*
    ** Use a separate highlighter so that these get highlighted within
    ** equations and such.
    **/
    highlight_prefixed_words_inclusive(&hinfo2, '\\', HL_CALL);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo2);
    highlight_info_free(&hinfo1);
    ys->redraw = 1;
}

void syntax_latex_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->ft != yed_get_ft("LaTeX")) {
        return;
    }

    highlight_line(&hinfo1, event);
    highlight_line(&hinfo2, event);
    syntax_latex_highlight_comments(event);
}

void syntax_latex_highlight_comments(yed_event *event) {
    yed_attrs  comment_attrs;
    yed_line  *line;
    yed_glyph *g;
    yed_glyph *prev;
    int        idx;
    int        col;
    yed_attrs *attr;

    comment_attrs = yed_get_active_style_scomp(STYLE_code_comment);
    line          = yed_buff_get_line(event->frame->buffer, event->row);

    prev = NULL;
    yed_line_glyph_traverse(*line, g) {
        if (g->c == '%') {
            if (prev == NULL || prev->c != '\\') {
                idx = ((void*)g) - ((void*)array_data(line->chars));
                col = yed_line_idx_to_col(line, idx);
                array_traverse_from(event->line_attrs, attr, col - 1) {
                    yed_combine_attrs(attr, &comment_attrs);
                }
                break;
            }
        }
        prev = g;
    }
}
