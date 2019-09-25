#include "internal.h"

static void yed_init_frames(void) {
    ys->frames       = tree_make_c(yed_frame_id_t, yed_frame_ptr_t, strcmp);
    ys->active_frame = NULL;
}

static yed_frame * yed_add_new_frame(yed_frame_id_t id, int top, int left, int height, int width) {
    yed_frame *frame;

    frame = yed_new_frame(id, top, left, height, width);

    tree_insert(ys->frames, frame->id, frame);

    return frame;
}

static yed_frame * yed_get_frame(yed_frame_id_t id) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t) it;

    it = tree_lookup(ys->frames, id);

    if (tree_it_good(it)) {
        return tree_it_val(it);
    }

    return NULL;
}

static yed_frame * yed_get_or_add_frame(yed_frame_id_t id) {
    yed_frame *frame;

    frame = yed_get_frame(id);

    if (!frame) {
        frame = yed_add_new_frame(id, 1, 1, ys->term_rows - 1, ys->term_cols);
    }

    return frame;
}

static yed_frame * yed_new_frame(yed_frame_id_t id, int top, int left, int height, int width) {
    yed_frame *frame;

    frame = malloc(sizeof(*frame));

    frame->id              = strdup(id);
    frame->buffer          = NULL;
    frame->top             = top;
    frame->left            = left;
    frame->height          = height;
    frame->width           = width;
    frame->cursor_line     = 1;
    frame->dirty_line      = frame->cursor_line;
    frame->buffer_y_offset = 0;
    frame->buffer_x_offset = 0;
    frame->cur_x           = left;
    frame->cur_y           = top;
    frame->desired_x       = 1;
    frame->dirty           = 1;
    frame->scroll_off      = 5;

    return frame;
}

static void yed_activate_frame(yed_frame *frame) {
    if (ys->active_frame && ys->active_frame != frame) {
        ys->active_frame->dirty      = 1;
        ys->active_frame->dirty_line = 0;
    }

    ys->active_frame             = frame;
    ys->active_frame->dirty      = 1;
    ys->active_frame->dirty_line = ys->active_frame->cursor_line;

    /*
     * Correct the cursor if the buffer has changed
     * while this frame was inactive.
     */
    yed_move_cursor_within_frame(frame, 0, 0);
}

static void yed_clear_frame(yed_frame *frame) {
    int r, c;
    int x, y;

    x = ys->cur_x;
    y = ys->cur_y;

    for (r = 0; r < frame->height; r += 1) {
        yed_set_cursor(frame->left, frame->top + r);

        for (c = 0; c < frame->width; c += 1) {
            append_n_to_output_buff(" ", 1);
        }
    }

    yed_set_cursor(x, y);
}

static void yed_frame_draw_line(yed_frame *frame, yed_line *line, int y_offset, int x_offset) {
    int n;

    yed_set_cursor(frame->left, frame->top + y_offset);
    n = MIN(MAX(array_len(line->chars) - x_offset, 0), frame->width);
    append_n_to_output_buff(array_data(line->chars) + x_offset, n);
    for (; n < frame->width; n += 1) {
        append_n_to_output_buff(" ", 1);
    }
}

static void yed_frame_draw_fill(yed_frame *frame, int y_offset) {
    int i, n;

    for (i = 0; i < frame->height - y_offset; i += 1) {
        yed_set_cursor(frame->left, frame->top + y_offset + i);
        append_n_to_output_buff("~", 1);
        for (n = 0; n < frame->width - 1; n += 1) {
            append_n_to_output_buff(" ", 1);
        }
    }
}

static void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int y_offset, int x_offset) {
    yed_line *line;
    int lines_drawn;
    int lines_seen;

    lines_drawn = 0;
    lines_seen  = 0;

    array_traverse(buff->lines, line) {
        if (lines_seen++ < y_offset)    { continue; }

        yed_frame_draw_line(frame, line, lines_drawn, x_offset);

        lines_drawn += 1;

        if (lines_drawn == frame->height)    { break; }
    }

    yed_frame_draw_fill(frame, lines_drawn);
}

static void yed_frame_set_pos(yed_frame *frame, int top, int left) {
    frame->top  = top;
    frame->left = left;
}

static void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff) {
    frame->buffer = buff;
    frame->dirty  = 1;
}

static void yed_frame_update(yed_frame *frame) {
    if (frame->buffer) {
        if (frame->dirty) {
            yed_frame_draw_buff(frame, frame->buffer, frame->buffer_y_offset, frame->buffer_x_offset);
        }
        if (frame->dirty_line != frame->cursor_line) {
            yed_frame_update_dirty_line(frame);
        }
        yed_frame_update_cursor_line(frame);
    } else {
        if (frame->dirty) {
            yed_clear_frame(frame);
        }
    }

    frame->dirty = frame->dirty_line = 0;
}

static void yed_set_cursor_within_bufferless_frame(yed_frame *f, int new_x, int new_y) {
    LIMIT(f->cur_y, f->top,  f->top + f->height - 1);
    LIMIT(f->cur_x, f->left, f->left + f->width - 1);

    f->desired_x = f->cur_x;
}

static void yed_move_cursor_within_bufferless_frame(yed_frame *f, int col, int row) {
    yed_set_cursor_within_bufferless_frame(f, f->cur_x + col, f->cur_y + row);
}

static void yed_move_cursor_once_y_within_frame(yed_frame *f, int dir) {
    int       new_y;
    int       buff_n_lines;
    yed_line *line;
    int       line_width;
    int       update_desired_x;

    if (dir > 0) {
        dir = 1;
    } else if (dir < 0) {
        dir = -1;
    } else {
        return;
    }

    new_y = f->cur_y + dir;

    buff_n_lines = array_len(f->buffer->lines);

    if (buff_n_lines > 2 * f->scroll_off) {
        if (f->buffer_y_offset < buff_n_lines - f->height - 1
        && new_y >= f->top + f->height - f->scroll_off) {

            f->buffer_y_offset += dir;
            f->dirty            = 1;
        } else if (f->buffer_y_offset >= 1
               &&  new_y < f->top + f->scroll_off) {

            f->buffer_y_offset += dir;
            f->dirty            = 1;
        } else {
            f->cur_y = new_y;
        }
    } else {
        f->cur_y = new_y;
    }

    LIMIT(f->cur_y,
            f->top,
            MIN(f->top + f->height - 1, f->top + buff_n_lines - 1));

    /*
     * Update the cursor line.
     */
    f->dirty_line  = f->cursor_line;
    f->cursor_line = f->buffer_y_offset + (f->cur_y - f->top + 1);

    /*
     * Update x values tied y.
     */
    line             = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width       = array_len(line->chars);
    update_desired_x = 0;

    if (line_width < f->buffer_x_offset) {
        f->buffer_x_offset = 0;
        f->dirty           = 1;
        update_desired_x   = 1;
    }
    f->cur_x = MIN(f->desired_x, f->left + line_width - f->buffer_x_offset);

    if (update_desired_x) {
        f->desired_x = f->cur_x;
    }
}

static void yed_move_cursor_once_x_within_frame(yed_frame *f, int dir) {
    int       new_x;
    yed_line *line;
    int       line_width;

    if (dir > 0) {
        dir = 1;
    } else if (dir < 0) {
        dir = -1;
    } else {
        return;
    }

    new_x      = f->cur_x + dir;
    line       = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width = array_len(line->chars);

    if (new_x >= f->left + f->width) {
        if (f->buffer_x_offset <= line_width - f->width) {
            f->buffer_x_offset += dir;
            f->dirty            = 1;
        }
    } else if (new_x < f->left) {
        if (f->buffer_x_offset >= 1) {
            f->buffer_x_offset += dir;
            f->dirty            = 1;
        }
    } else {
        f->cur_x = new_x;
    }

    LIMIT(f->cur_x, f->left, f->left + line_width - f->buffer_x_offset);

    f->desired_x = f->cur_x;
}

static void yed_set_cursor_within_frame(yed_frame *f, int new_x, int new_y) {
    int       col, row;

    if (!f->buffer) {
        yed_set_cursor_within_bufferless_frame(f, new_x, new_y);
        return;
    }

    row = new_y - f->cur_y;
    col = new_x - f->cur_x;

    yed_move_cursor_within_frame(f, col, row);

}

static void yed_move_cursor_within_frame(yed_frame *f, int col, int row) {
    int i, dir;

    if (row) {
        dir = row > 0 ? 1 : -1;
        for (i = 0; i < dir * row; i += 1) {
            yed_move_cursor_once_y_within_frame(f, dir);
        }
    }

    if (col) {
        dir = col > 0 ? 1 : -1;
        for (i = 0; i < dir * col; i += 1) {
            yed_move_cursor_once_x_within_frame(f, dir);
        }
    }
}

#if 0

static int yed_update_frame_buffer_y_offset(yed_frame *f, int col, int row) {
    int buff_n_lines;

    buff_n_lines = array_len(f->buffer->lines);

    if (buff_n_lines > (2 * f->scroll_off)) {

        if ((row > 0 && f->cur_y == f->top + f->height - 1 - f->scroll_off)
        ||  (row < 0 && f->cur_y == f->top + f->scroll_off)) {

            f->buffer_y_offset += row;
            f->dirty_line     = f->cursor_line;
            f->cursor_line   += row;
            LIMIT(f->cursor_line, 1, buff_n_lines);
            f->buffer_y_offset  = MAX(MIN(f->buffer_y_offset, buff_n_lines - f->height - 1), 0);
            f->dirty          = 1;

            if (row > 0 && f->buffer_y_offset < buff_n_lines - f->height - 1) {
                return 1;
            }
            if (row < 0 && f->buffer_y_offset >= 1) {
                return 1;
            }
        }
    }

    return 0;
}

static void yed_move_cursor_within_frame(yed_frame *f, int col, int row) {
    int       changed_buff_off;
    yed_line *current_line;

    if (!f->buffer) {
        yed_move_cursor_within_bufferless_frame(f, col, row);
        return;
    }

    changed_buff_off = yed_update_frame_buffer_y_offset(f, col, row);

    if (!changed_buff_off) {
        f->cur_y += row;
        LIMIT(f->cur_y, f->top,
			  MIN(array_len(f->buffer->lines), f->top + f->height - 1));
    }

    f->dirty_line  = f->cursor_line;
    f->cursor_line = f->buffer_y_offset + (f->cur_y - f->top + 1);
    LIMIT(f->cursor_line, 1, array_len(f->buffer->lines));

    current_line   = array_item(f->buffer->lines, f->cursor_line - 1);

    if (col == 0) {
        f->cur_x = MIN(f->desired_x, f->left + array_len(current_line->chars));
    } else {
        f->cur_x += col;
        f->cur_x  = MIN(f->cur_x, f->left + array_len(current_line->chars));
        LIMIT(f->cur_x, f->left, f->left + f->width - 1);

        f->desired_x = f->cur_x;
    }
}
#endif

static void yed_move_cursor_within_active_frame(int col, int row) {
    if (ys->active_frame) {
        yed_move_cursor_within_frame(ys->active_frame, col, row);
    }
}

static void yed_update_frames(void) {
    yed_frame                                *frame;
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;

    tree_traverse(ys->frames, it) {
        frame = tree_it_val(it);
        if (frame == ys->active_frame) {
            continue;
        }
        yed_frame_update(frame);
    }

    if (ys->active_frame) {
        yed_frame_update(ys->active_frame);
        yed_set_cursor(ys->active_frame->cur_x, ys->active_frame->cur_y);
    }
}

static int yed_frame_line_is_visible(yed_frame *frame, int row) {
    if (!frame->buffer) {
        return 0;
    }
    return    (row >= frame->buffer_y_offset + 1)
           && (row <= frame->buffer_y_offset + frame->height)
           && (row <= array_len(frame->buffer->lines));
}

static int yed_frame_line_to_y(yed_frame *frame, int row) {
    if (!frame->buffer || !yed_frame_line_is_visible(frame, row)) {
        return 0;
    }

    return frame->top + row - (frame->buffer_y_offset + 1);
}

static void yed_frame_update_dirty_line(yed_frame *frame) {
    yed_line *line;
    int       y;

    if (!frame->dirty_line) {
        return;
    }

    y = yed_frame_line_to_y(frame, frame->dirty_line);
    if (y) {
        line = yed_buff_get_line(frame->buffer, frame->dirty_line);
        yed_frame_draw_line(frame, line, y - frame->top, frame->buffer_x_offset);
    }
}

static void yed_frame_update_cursor_line(yed_frame *frame) {
    yed_line *line;
    int       y;

    if (!frame->cursor_line) {
        return;
    }

    y = yed_frame_line_to_y(frame, frame->cursor_line);
    if (y) {

        if (frame == ys->active_frame) {
            append_to_output_buff(TERM_BG_BLUE);
            append_to_output_buff(TERM_DARK_GRAY);
        }
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);
        yed_frame_draw_line(frame, line, y - frame->top, frame->buffer_x_offset);
        if (frame == ys->active_frame) {
            append_to_output_buff(TERM_RESET);
            append_to_output_buff(TERM_CURSOR_HIDE);
        }
    }
}

static void yed_frame_take_key(yed_frame *frame, int key) {
    char *key_str;
    char  key_str_buff[2];

    switch (key) {
        case KEY_UP:        yed_execute_command("cursor-up",    0, NULL); break;
        case KEY_DOWN:      yed_execute_command("cursor-down",  0, NULL); break;
        case KEY_RIGHT:     yed_execute_command("cursor-right", 0, NULL); break;
        case KEY_LEFT:      yed_execute_command("cursor-left",  0, NULL); break;
        case KEY_BACKSPACE: yed_execute_command("delete-back",  0, NULL); break;
        case CTRL('l'):     yed_execute_command("frame-next",   0, NULL); break;
        default: {
            if (key == '\n' || !iscntrl(key)) {
                key_str_buff[0] = (char)key;
                key_str_buff[1] = 0;
                key_str         = key_str_buff;
                yed_execute_command("insert", 1, &key_str);
            }
        }
    }
}

static void yed_mark_dirty_frames(yed_buffer *dirty_buff) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    tree_traverse(ys->frames, it) {
        frame = tree_it_val(it);
        if (frame->buffer == dirty_buff) {
            frame->dirty = 1;
        }
    }
}

static void yed_mark_dirty_frames_line(yed_buffer *buff, int dirty_row) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    tree_traverse(ys->frames, it) {
        frame = tree_it_val(it);
        if (frame->buffer == buff) {
            frame->dirty_line = dirty_row;
        }
    }
}
