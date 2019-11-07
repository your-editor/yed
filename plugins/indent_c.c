#include "plugin.h"

void indent_c_post_insert_handler(yed_event *event);
void indent_c_post_delete_back_handler(yed_event *event);

void indent_line(int n_args, char **args);
void unindent_line(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler insert, delete_back;

    insert.kind      = EVENT_BUFFER_POST_INSERT;
    insert.fn        = indent_c_post_insert_handler;
    delete_back.kind = EVENT_BUFFER_PRE_DELETE_BACK;
    delete_back.fn   = indent_c_post_delete_back_handler;

    yed_plugin_add_event_handler(self, insert);
    yed_plugin_add_event_handler(self, delete_back);

    yed_plugin_set_command(self, "indent-line", indent_line);
    yed_plugin_set_command(self, "unindent-line", unindent_line);

    return 0;
}

static int get_tabw(void) {
    char *tabw_str;
    int   tabw;

    tabw_str = yed_get_var("tab-width");

    if (tabw_str) {
        sscanf(tabw_str, "%d", &tabw);
    } else {
        tabw = 4;
    }

    return tabw;
}

static void do_indent(yed_frame *frame) {
    yed_line  *prev_line;
    int        i, indent_width,
               tabw;
    char      *c;

    tabw = get_tabw();

    if (tabw <= 0) {
        return;
    }

    prev_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
    if (!prev_line)    { return; }

    indent_width = 0;
    while (indent_width < array_len(prev_line->chars)
    &&     *(c = array_item(prev_line->chars, indent_width)) == ' ') {
        indent_width += 1;
    }

    if (array_len(prev_line->chars) > 0
    &&  yed_line_col_to_char(prev_line, array_len(prev_line->chars)) == '{') {
        indent_width += tabw;
    }

    for (i = 0; i < indent_width; i += 1) {
        yed_insert_into_line(frame->buffer, frame->cursor_line, 1, ' ');
    }

    yed_move_cursor_within_frame(frame, indent_width, 0);
}

static void do_brace_backup(yed_frame *frame) {
    yed_line *line;
    int       i, brace_col, tabw;

    tabw = get_tabw();

    if (tabw <= 0) {
        return;
    }

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

    yed_move_cursor_within_frame(frame, -tabw, 0);
    for (i = 0; i < tabw; i += 1) {
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
    int        i, col, tabw, all_spaces;
    char *indent_c_disable_bs;

    indent_c_disable_bs = yed_get_var("indent-c-disable-bs");
    if(indent_c_disable_bs) {
      return;
    }

    frame = event->frame;

    tabw = get_tabw();

    if (tabw <= 0 || frame->cursor_col < tabw) {
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

    if (col % tabw == 1) {
        yed_move_cursor_within_frame(frame, -(tabw - 1), 0);
        for (i = 0; i < (tabw - 1); i += 1) {
            yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
        }
    }
}

void indent_line(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         tabw,
                col, save_cursor_col,
                rem, add,
                i;

    tabw = get_tabw();

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
    line = yed_buff_get_line(buff, frame->cursor_line);

    if (!line)    { return; }

    col = 1;

     while (col <= line->visual_width) {
        if (yed_line_col_to_char(line, col) != ' ') {
            break;
        }
        col += 1;
    }

    rem             = (col - 1) % tabw;
    add             = rem ? (tabw - rem) : tabw;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);

    for (i = 0; i < add; i += 1) {
        yed_insert_into_line(buff, frame->cursor_line, 1, ' ');
    }

    yed_set_cursor_within_frame(frame, save_cursor_col + add, frame->cursor_line);
}

void unindent_line(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         tabw,
                col, save_cursor_col,
                rem, sub,
                i;

    tabw = get_tabw();

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
    line = yed_buff_get_line(buff, frame->cursor_line);

    if (!line)    { return; }

    col = 1;

    while (col <= line->visual_width) {
        if (yed_line_col_to_char(line, col) != ' ') {
            break;
        }
        col += 1;
    }

    if (col == 1)    { return; }

    rem             = (col - 1) % tabw;
    sub             = rem ? rem : tabw;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);

    for (i = 0; i < sub; i += 1) {
        yed_delete_from_line(buff, frame->cursor_line, 1);
    }

    yed_set_cursor_within_frame(frame, save_cursor_col - sub, frame->cursor_line);
}
