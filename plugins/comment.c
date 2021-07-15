#include <yed/plugin.h>

void comment_toggle(int n_args, char **args);
int  comment_toggle_line(yed_frame *frame, yed_line *line, int row);

void comment_toggle_line_c(yed_frame *frame, yed_line *line, int row);
void comment_line_c(yed_frame *frame, int row);
void uncomment_line_c(yed_frame *frame, int row);

void comment_toggle_line_hash(yed_frame *frame, yed_line *line, int row);
void comment_line_hash(yed_frame *frame, int row);
void uncomment_line_hash(yed_frame *frame, int row);

void comment_toggle_line_latex(yed_frame *frame, yed_line *line, int row);
void comment_line_latex(yed_frame *frame, int row);
void uncomment_line_latex(yed_frame *frame, int row);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "comment-toggle", comment_toggle);
    return 0;
}

void comment_toggle(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         save_col,
                row,
                r1, c1, r2, c2,
                status;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    yed_start_undo_record(frame, buff);

    save_col = frame->cursor_col;
    yed_set_cursor_within_frame(frame, frame->cursor_line, 1);

    if (buff->has_selection) {
        yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);

        for (row = r1; row <= r2; row += 1) {
            line   = yed_buff_get_line(buff, row);
            status = comment_toggle_line(frame, line, row);
        }

        frame->dirty = 1;

        YEXE("select-off");
    } else {
        line   = yed_buff_get_line(buff, frame->cursor_line);
        status = comment_toggle_line(frame, line, frame->cursor_line);
    }

    yed_set_cursor_within_frame(frame, frame->cursor_line, save_col);

    if (status == 0) {
        yed_cerr("unhandled ft");
        yed_cancel_undo_record(frame, buff);
    } else {
        yed_end_undo_record(frame, buff);
    }
}

int comment_toggle_line(yed_frame *frame, yed_line *line, int row) {
    if (       frame->buffer->ft == yed_get_ft("C")      ||
               frame->buffer->ft == yed_get_ft("C++"))   {
        comment_toggle_line_c(frame, line, row);

    } else if (frame->buffer->ft == yed_get_ft("Shell")  ||
               frame->buffer->ft == yed_get_ft("bJou")   ||
               frame->buffer->ft == yed_get_ft("Python") ||
               frame->buffer->ft == yed_get_ft("yedrc")) {
        comment_toggle_line_hash(frame, line, row);

    } else if (frame->buffer->ft == yed_get_ft("LaTeX")) {
        comment_toggle_line_latex(frame, line, row);

    } else {
        return 0;
    }

    return 1;
}


/* C */
void comment_toggle_line_c(yed_frame *frame, yed_line *line, int row) {
    int        line_len;
    yed_glyph *g;

    line_len = line->visual_width;

    /* Are we uncommenting? */
    if (line_len >= 6) {
        g = yed_line_col_to_glyph(line, 1);
        if (g->c == '/') { g = yed_line_col_to_glyph(line, 2);
        if (g->c == '*') { g = yed_line_col_to_glyph(line, 3);
        if (g->c == ' ') { g = yed_line_col_to_glyph(line, line_len);
        if (g->c == '/') { g = yed_line_col_to_glyph(line, line_len - 1);
        if (g->c == '*') { g = yed_line_col_to_glyph(line, line_len - 2);
        if (g->c == ' ') {
            uncomment_line_c(frame, row);
            return;
        }}}}}}
    }

    comment_line_c(frame, row);
}

void comment_line_c(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, G(' '));
    yed_insert_into_line(frame->buffer, row, 1, G('*'));
    yed_insert_into_line(frame->buffer, row, 1, G('/'));

    yed_append_to_line(frame->buffer, row, G(' '));
    yed_append_to_line(frame->buffer, row, G('*'));
    yed_append_to_line(frame->buffer, row, G('/'));
}

void uncomment_line_c(yed_frame *frame, int row) {
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);

    yed_pop_from_line(frame->buffer, row);
    yed_pop_from_line(frame->buffer, row);
    yed_pop_from_line(frame->buffer, row);
}

/* Hash-comment languages */
void comment_toggle_line_hash(yed_frame *frame, yed_line *line, int row) {
    int        line_len;
    yed_glyph *g;

    line_len = line->visual_width;

    /* Are we uncommenting? */
    if (line_len >= 2) {
        g = yed_line_col_to_glyph(line, 1);
        if (g->c == '#') {
            g = yed_line_col_to_glyph(line, 2);
            if (g->c == ' ') {
                uncomment_line_hash(frame, row);
                return;
            }
        }
    }

    comment_line_hash(frame, row);
}

void comment_line_hash(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, G(' '));
    yed_insert_into_line(frame->buffer, row, 1, G('#'));
}

void uncomment_line_hash(yed_frame *frame, int row) {
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);
}

/* LaTeX */
void comment_toggle_line_latex(yed_frame *frame, yed_line *line, int row) {
    int        line_len;
    yed_glyph *g;

    line_len = line->visual_width;

    /* Are we uncommenting? */
    if (line_len >= 2) {
        g = yed_line_col_to_glyph(line, 1);
        if (g->c == '%') {
            g = yed_line_col_to_glyph(line, 2);
            if (g->c == ' ') {
                uncomment_line_hash(frame, row);
                return;
            }
        }
    }

    comment_line_latex(frame, row);
}

void comment_line_latex(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, G(' '));
    yed_insert_into_line(frame->buffer, row, 1, G('%'));
}

void uncomment_line_latex(yed_frame *frame, int row) {
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);
}
