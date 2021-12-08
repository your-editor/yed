#include <yed/plugin.h>

void indent_c_post_insert_handler(yed_event *event);
void indent_c_post_delete_back_handler(yed_event *event);

void indent(int n_args, char **args);
void unindent(int n_args, char **args);
void indent_line(yed_frame *frame, yed_line *line, int row, int tabw);
void unindent_line(yed_frame *frame, yed_line *line, int row, int tabw);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler insert, delete_back;

    YED_PLUG_VERSION_CHECK();

    insert.kind      = EVENT_BUFFER_POST_INSERT;
    insert.fn        = indent_c_post_insert_handler;
    delete_back.kind = EVENT_BUFFER_PRE_DELETE_BACK;
    delete_back.fn   = indent_c_post_delete_back_handler;

    yed_plugin_add_event_handler(self, insert);
    yed_plugin_add_event_handler(self, delete_back);

    yed_plugin_set_command(self, "indent", indent);
    yed_plugin_set_command(self, "unindent", unindent);

    return 0;
}

static int get_tabw(void) { return ys->tabw; }

static void do_indent(yed_frame *frame) {
    yed_line  *prev_line;
    int        i, indent_width,
               tabw;

    tabw = get_tabw();

    if (tabw <= 0) {
        return;
    }

    prev_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
    if (!prev_line)    { return; }

    indent_width = 0;
    while (indent_width < prev_line->visual_width
    &&     yed_line_col_to_glyph(prev_line, indent_width + 1)->c == ' ') {
        indent_width += 1;
    }

    if (prev_line->visual_width > 0
    &&  yed_line_col_to_glyph(prev_line, prev_line->visual_width)->c == '{') {
        indent_width += tabw;
    }

    for (i = 0; i < indent_width; i += 1) {
        yed_insert_into_line(frame->buffer, frame->cursor_line, 1, G(' '));
    }

    yed_move_cursor_within_frame(frame, 0, indent_width);
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
    &&     yed_line_col_to_glyph(line, i + 1)->c == ' ') {
        i += 1;
    }

    if (i != brace_col - 1)    { return; }

    if (tabw > i) { tabw = i; }

    yed_move_cursor_within_frame(frame, 0, -tabw);
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
    if (indent_c_disable_bs) {
        return;
    }

    frame = event->frame;

    if (frame->buffer->has_selection) {
        return;
    }

    tabw = get_tabw();

    if (tabw <= 0 || frame->cursor_col < tabw) {
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (!line)    { return; }

    col = all_spaces = 1;

    while (col < frame->cursor_col) {
        if (yed_line_col_to_glyph(line, col)->c != ' ') {
            all_spaces = 0;
            break;
        }
        col += 1;
    }

    if (!all_spaces)    { return; }

    if (col % tabw == 1) {
        yed_move_cursor_within_frame(frame, 0, -(tabw - 1));
        for (i = 0; i < (tabw - 1); i += 1) {
            yed_delete_from_line(frame->buffer, frame->cursor_line, 1);
        }
    }
}

void indent(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         tabw,
                save_col,
                r1, c1, r2, c2,
                row;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    tabw = get_tabw();
    buff = frame->buffer;

    yed_start_undo_record(frame, buff);

    save_col = frame->cursor_col;
    yed_set_cursor_within_frame(frame, frame->cursor_line, 1);

    if (buff->has_selection) {
        yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);

        for (row = r1; row <= r2; row += 1) {
            line = yed_buff_get_line(buff, row);
            indent_line(frame, line, row, tabw);
        }

        frame->dirty = 1;
    } else {
        line = yed_buff_get_line(buff, frame->cursor_line);
        indent_line(frame, line, frame->cursor_line, tabw);
    }

    yed_set_cursor_within_frame(frame, frame->cursor_line, save_col);

    yed_end_undo_record(frame, buff);
}

void unindent(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    int         tabw,
                save_col,
                r1, c1, r2, c2,
                row;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    tabw = get_tabw();
    buff = frame->buffer;

    yed_start_undo_record(frame, buff);

    save_col = frame->cursor_col;
    yed_set_cursor_within_frame(frame, frame->cursor_line, 1);

    if (buff->has_selection) {
        yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);

        for (row = r1; row <= r2; row += 1) {
            line = yed_buff_get_line(buff, row);
            unindent_line(frame, line, row, tabw);
        }

        frame->dirty = 1;
    } else {
        line = yed_buff_get_line(buff, frame->cursor_line);
        unindent_line(frame, line, frame->cursor_line, tabw);
    }

    yed_set_cursor_within_frame(frame, frame->cursor_line, save_col);

    yed_end_undo_record(frame, buff);
}

void indent_line(yed_frame *frame, yed_line *line, int row, int tabw) {
    int col,
        rem, add,
        i;

    col = 1;

     while (col <= line->visual_width) {
        if (yed_line_col_to_glyph(line, col)->c != ' ') {
            break;
        }
        col += 1;
    }

    rem = (col - 1) % tabw;
    add = rem ? (tabw - rem) : tabw;

    for (i = 0; i < add; i += 1) {
        yed_insert_into_line(frame->buffer, row, 1, G(' '));
    }
}

void unindent_line(yed_frame *frame, yed_line *line, int row, int tabw) {
    int col,
        rem, sub,
        i;

    col = 1;

     while (col <= line->visual_width) {
        if (yed_line_col_to_glyph(line, col)->c != ' ') {
            break;
        }
        col += 1;
    }

    if (col == 1)    { return; }

    rem = (col - 1) % tabw;
    sub = rem ? rem : tabw;

    for (i = 0; i < sub; i += 1) {
        yed_delete_from_line(frame->buffer, row, 1);
    }
}
