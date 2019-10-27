#include "plugin.h"

void indent_c_post_insert_handler(yed_event *event);
void indent_c_post_delete_back_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler insert, delete_back;

    insert.kind      = EVENT_BUFFER_POST_INSERT;
    insert.fn        = indent_c_post_insert_handler;
    delete_back.kind = EVENT_BUFFER_PRE_DELETE_BACK;
    delete_back.fn   = indent_c_post_delete_back_handler;

    yed_plugin_add_event_handler(self, insert);
    yed_plugin_add_event_handler(self, delete_back);

    return 0;
}

static void do_indent(yed_frame *frame) {
    yed_line  *prev_line;
    int        i, indent_width;
    char      *c;

    prev_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
    if (!prev_line)    { return; }

    indent_width = 0;
    while (indent_width < array_len(prev_line->chars)
    &&     *(c = array_item(prev_line->chars, indent_width)) == ' ') {
        indent_width += 1;
    }

    if (array_len(prev_line->chars) > 0
    &&  yed_line_col_to_char(prev_line, array_len(prev_line->chars)) == '{') {
        indent_width += 4;
    }

    for (i = 0; i < indent_width; i += 1) {
        yed_insert_into_line(frame->buffer, frame->cursor_line, 1, ' ');
    }

    yed_move_cursor_within_frame(frame, indent_width, 0);
}

static void do_brace_backup(yed_frame *frame) {
    yed_line *line;
    int       i, brace_col;

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line)    { return; }

    brace_col = frame->cursor_col - 1;

    if (brace_col == 1)    { return; }

    i = 0;

    while (i < brace_col
    &&     yed_line_col_to_char(line, i + 1) == ' ') {
        i += 1;
    }

    if (i != brace_col - 1)    { return; }

    yed_move_cursor_within_frame(frame, -4, 0);
    for (i = 0; i < 4; i += 1) {
        yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
    }
}

void indent_c_post_insert_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (event->key == ENTER) {
        do_indent(frame);
    } else if (event->key == '}') {
        do_brace_backup(frame);
    }
}

void indent_c_post_delete_back_handler(yed_event *event) {
    yed_frame *frame;
    yed_line  *line;
    int        i, col, all_spaces;

    frame = event->frame;

    if (frame->cursor_col < 4) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line)    { return; }
    
    col = all_spaces = 1;

    while (col < frame->cursor_col) {
        if (yed_line_col_to_char(line, col) != ' ') {
            all_spaces = 0;
            break;
        }
        col += 1;
    }

    if (!all_spaces)    { return; }

    if (col % 4 == 1) {
        yed_move_cursor_within_frame(frame, -3, 0);
        for (i = 0; i < 3; i += 1) {
            yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
        }
    }
}
