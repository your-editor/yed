#include "plugin.h"

void comment_toggle_line(int n_args, char **args);
void comment_line(yed_frame *frame, yed_line *line);
void uncomment_line(yed_frame *frame, yed_line *line);

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_command(self, "comment-toggle-line", comment_toggle_line);
    return 0;
}

void comment_toggle_line(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        line_len;
    char      *c;

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

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    if (!line)    { return; }

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
            uncomment_line(frame, line);
            return;
        }}}}}}
    }

    comment_line(frame, line);
}

void comment_line(yed_frame *frame, yed_line *line) {
    int save_cursor_col;

    save_cursor_col = frame->cursor_col;

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);

    yed_insert_into_line(frame->buffer, frame->cursor_line, 1, ' ');
    yed_insert_into_line(frame->buffer, frame->cursor_line, 1, '*');
    yed_insert_into_line(frame->buffer, frame->cursor_line, 1, '/');

    yed_append_to_line(line, ' ');
    yed_append_to_line(line, '*');
    yed_append_to_line(line, '/');

    yed_set_cursor_within_frame(frame, save_cursor_col, frame->cursor_line);
}

void uncomment_line(yed_frame *frame, yed_line *line) {
    int save_cursor_col;

    save_cursor_col = frame->cursor_col;

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);

    yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
    yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
    yed_delete_from_line(frame->buffer, frame->cursor_line, 1);

    array_pop(line->chars);
    line->visual_width -= 1;
    array_pop(line->chars);
    line->visual_width -= 1;
    array_pop(line->chars);
    line->visual_width -= 1;

    yed_set_cursor_within_frame(frame, save_cursor_col, frame->cursor_line);
}
