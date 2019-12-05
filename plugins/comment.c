#include "plugin.h"

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
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    yed_start_undo_record(frame, buff);

    save_col = frame->cursor_col;
    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);

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

    yed_set_cursor_within_frame(frame, save_col, frame->cursor_line);

    if (status == 0) {
        yed_append_text_to_cmd_buff("[!] unhandled ft");
        yed_cancel_undo_record(frame, buff);
    } else {
        yed_end_undo_record(frame, buff);
    }
}

int comment_toggle_line(yed_frame *frame, yed_line *line, int row) {
    switch (frame->buffer->file.ft) {
        case FT_C:
        case FT_CXX:   comment_toggle_line_c(frame, line, row);     break;

        case FT_SH:
        case FT_BJOU:  comment_toggle_line_hash(frame, line, row);  break;

        case FT_LATEX: comment_toggle_line_latex(frame, line, row); break;

        default:
            return 0;
    }

    return 1;
}


/* C */
void comment_toggle_line_c(yed_frame *frame, yed_line *line, int row) {
    int   line_len;
    char *c;

    line_len = array_len(line->chars);

    /* Are we uncommenting? */
    if (array_len(line->chars) >= 6) {
        c = array_item(line->chars, 0);
        if (*c == '/') { c = array_item(line->chars, 1);
        if (*c == '*') { c = array_item(line->chars, 2);
        if (*c == ' ') { c = array_item(line->chars, line_len - 1);
        if (*c == '/') { c = array_item(line->chars, line_len - 2);
        if (*c == '*') { c = array_item(line->chars, line_len - 3);
        if (*c == ' ') {
            uncomment_line_c(frame, row);
            return;
        }}}}}}
    }

    comment_line_c(frame, row);
}

void comment_line_c(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, ' ');
    yed_insert_into_line(frame->buffer, row, 1, '*');
    yed_insert_into_line(frame->buffer, row, 1, '/');

    yed_append_to_line(frame->buffer, row, ' ');
    yed_append_to_line(frame->buffer, row, '*');
    yed_append_to_line(frame->buffer, row, '/');
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
    int   line_len;
    char *c;

    line_len = array_len(line->chars);

    /* Are we uncommenting? */
    if (array_len(line->chars) >= 2) {
        c = array_item(line->chars, 0);
        if (*c == '#') {
            c = array_item(line->chars, 1);
            if (*c == ' ') {
                uncomment_line_hash(frame, row);
                return;
            }
        }
    }

    comment_line_hash(frame, row);
}

void comment_line_hash(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, ' ');
    yed_insert_into_line(frame->buffer, row, 1, '#');
}

void uncomment_line_hash(yed_frame *frame, int row) {
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);
}

/* LaTeX */
void comment_toggle_line_latex(yed_frame *frame, yed_line *line, int row) {
    int   line_len;
    char *c;

    line_len = array_len(line->chars);

    /* Are we uncommenting? */
    if (array_len(line->chars) >= 2) {
        c = array_item(line->chars, 0);
        if (*c == '%') {
            c = array_item(line->chars, 1);
            if (*c == ' ') {
                uncomment_line_latex(frame, row);
                return;
            }
        }
    }

    comment_line_latex(frame, row);
}

void comment_line_latex(yed_frame *frame, int row) {
    yed_insert_into_line(frame->buffer, row, 1, ' ');
    yed_insert_into_line(frame->buffer, row, 1, '%');
}

void uncomment_line_latex(yed_frame *frame, int row) {
    yed_delete_from_line(frame->buffer, row, 1);
    yed_delete_from_line(frame->buffer, row, 1);
}
