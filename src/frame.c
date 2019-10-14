#include "internal.h"

void yed_init_frames(void) {
    ys->frames       = tree_make_c(yed_frame_id_t, yed_frame_ptr_t, strcmp);
    ys->active_frame = NULL;
}

yed_frame * yed_add_new_frame(yed_frame_id_t id, int top, int left, int height, int width) {
    yed_frame *frame;

    frame = yed_new_frame(id, top, left, height, width);

    tree_insert(ys->frames, frame->id, frame);

    return frame;
}

yed_frame * yed_get_frame(yed_frame_id_t id) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t) it;

    it = tree_lookup(ys->frames, id);

    if (tree_it_good(it)) {
        return tree_it_val(it);
    }

    return NULL;
}

yed_frame * yed_get_or_add_frame(yed_frame_id_t id) {
    yed_frame *frame;

    frame = yed_get_frame(id);

    if (!frame) {
        frame = yed_add_new_frame(id, 1, 1, ys->term_rows - 1, ys->term_cols);
    }

    return frame;
}

yed_frame * yed_new_frame(yed_frame_id_t id, int top, int left, int height, int width) {
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
    frame->cursor_col      = 1;
    frame->buffer_y_offset = 0;
    frame->buffer_x_offset = 0;
    frame->cur_x           = left;
    frame->cur_y           = top;
    frame->desired_x       = 1;
    frame->dirty           = 1;
    frame->scroll_off      = 5;

    return frame;
}

void yed_activate_frame(yed_frame *frame) {
    if (ys->active_frame && ys->active_frame != frame) {
        ys->active_frame->dirty      = 1;
        ys->active_frame->dirty_line = 0;
    }

    ys->active_frame             = frame;

    /*
     * Correct the cursor if the buffer has changed
     * while this frame was inactive.
     */
    if (frame->buffer->kind == BUFF_KIND_YANK) {
        yed_set_cursor_far_within_frame(frame, 1, 1);
    }
    yed_frame_reset_cursor(frame);
    ys->active_frame->dirty      = 1;
    ys->active_frame->dirty_line = ys->active_frame->cursor_line;
}

void yed_clear_frame(yed_frame *frame) {
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

void yed_frame_draw_line(yed_frame *frame, yed_line *line, int row, int y_offset, int x_offset) {
    int  n, col, n_col, starting_idx, set_sel_style;
    uint8_t attr_idx;
    char c;
    yed_cell *cell_it;
    yed_event event;

    event.kind  = EVENT_LINE_PRE_DRAW;
    event.frame = frame;
    event.row   = row;

    yed_trigger_event(&event);

    /*
     * @bad @incorrect
     *
     * This isn't how this is going to work.
     * A cell's width indicates how many columns the displayed
     * data takes up.
     *
     * That is TOTALLY DIFFERENT than the number of bytes!!!
     *
     * For now this is what I'm doing though.
     *
     *                  - Brandon Kammerdiener, Sept. 25, 2019
     */

    n_col = MIN(MAX(line->visual_width - x_offset, 0), frame->width);
    n     = 0;
    if (array_len(line->cells) < x_offset) {
        starting_idx = array_len(line->cells);
    } else {
        starting_idx = yed_line_col_to_cell_idx(line, x_offset + 1);
    }

    yed_set_cursor(frame->left, frame->top + y_offset);

    col = starting_idx + 1;

    array_traverse_from(line->cells, cell_it, starting_idx) {
        if (n == n_col)    { break; }
        c = cell_it->c;

        if (cell_it->attr_idx != attr_idx) {
            attr_idx = cell_it->attr_idx;
            yed_set_attr(attr_idx);
        }

        set_sel_style = 0;
        if (frame->buffer->has_selection
        && yed_is_in_range(&frame->buffer->selection, row, col)) {
            append_to_output_buff(TERM_INVERSE);
            set_sel_style = 1;
        }

        if (c == '\t') {
            append_to_output_buff(TERM_BG_RED);
            append_n_to_output_buff(" ", 1);
            append_to_output_buff(TERM_RESET);
            append_to_output_buff(TERM_CURSOR_HIDE);
        } else {
            append_n_to_output_buff(&c, 1);
        }

        if (set_sel_style) {
            append_to_output_buff(TERM_RESET);
            append_to_output_buff(TERM_CURSOR_HIDE);
        }

        n   += 1;
        col += 1;
    }

    for (; n < frame->width; n += 1) {
        append_n_to_output_buff(" ", 1);
    }
}

void yed_frame_draw_fill(yed_frame *frame, int y_offset) {
    int i, n;

    for (i = 0; i < frame->height - y_offset; i += 1) {
        yed_set_cursor(frame->left, frame->top + y_offset + i);
        append_n_to_output_buff("~", 1);
        for (n = 0; n < frame->width - 1; n += 1) {
            append_n_to_output_buff(" ", 1);
        }
    }
}

void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int y_offset, int x_offset) {
    yed_line *line;
    int lines_drawn;
    int row;

    lines_drawn = 0;

    row = y_offset + 1;
    bucket_array_traverse_from(buff->lines, line, y_offset) {
        yed_frame_draw_line(frame, line, row, lines_drawn, x_offset);

        lines_drawn += 1;

        if (lines_drawn == frame->height)    { break; }
        row += 1;
    }

    yed_frame_draw_fill(frame, lines_drawn);
}

void yed_frame_set_pos(yed_frame *frame, int top, int left) {
    frame->top  = top;
    frame->left = left;
}

void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff) {
    frame->buffer = buff;
    frame->dirty  = 1;
}

void yed_frame_update(yed_frame *frame) {
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

void yed_move_cursor_once_y_within_frame(yed_frame *f, int dir, int buff_n_lines, int buff_big_enough_to_scroll) {
    int new_y,
        bot;

    if (dir > 0) {
        dir = 1;
    } else if (dir < 0) {
        dir = -1;
    }

    bot = f->top + f->height;

    if (dir) {
        new_y = f->cur_y + dir;

        if (buff_big_enough_to_scroll) {
            if (f->buffer_y_offset < buff_n_lines - f->height
            && new_y >= bot - f->scroll_off) {

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
    }

    LIMIT(f->buffer_y_offset, 0, buff_n_lines - 1);
    LIMIT(f->cur_y,
            f->top,
            MIN(bot - 1,
                f->top + buff_n_lines - f->buffer_y_offset - 1));

    /*
     * Update the cursor line.
     */
    f->cursor_line = f->buffer_y_offset + (f->cur_y - f->top + 1);
}

void yed_move_cursor_once_x_within_frame(yed_frame *f, int dir, int line_width) {
    int       new_x;

    if (dir > 0) {
        dir = 1;
    } else if (dir < 0) {
        dir = -1;
    }

    if (dir) {
        new_x = f->cur_x + dir;

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
    }

    LIMIT(f->cur_x, f->left, f->left + line_width - f->buffer_x_offset);

    f->desired_x  = f->cur_x;
    f->cursor_col = f->buffer_x_offset + (f->cur_x - f->left + 1);
}

void yed_set_cursor_within_frame(yed_frame *f, int new_x, int new_y) {
    int       col, row;

    row = new_y - f->cursor_line;
    yed_move_cursor_within_frame(f, 0, row);
    col = new_x - f->cursor_col;
    yed_move_cursor_within_frame(f, col, 0);
}

void yed_move_cursor_within_frame(yed_frame *f, int col, int row) {
    int       i,
              dir,
              line_width,
              update_desired_x,
              buff_n_lines,
              buff_big_enough_to_scroll,
              save_cursor_line;
    yed_line *line;

    buff_n_lines              = bucket_array_len(f->buffer->lines);
    buff_big_enough_to_scroll = buff_n_lines > 2 * f->scroll_off;

    save_cursor_line = f->cursor_line;

    dir = row > 0 ? 1 : -1;
    for (i = 0; i < dir * row; i += 1) {
        yed_move_cursor_once_y_within_frame(f, dir, buff_n_lines, buff_big_enough_to_scroll);
    }

    if (save_cursor_line != f->cursor_line) {
        f->dirty_line = save_cursor_line;
    }

    line             = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width       = line->visual_width;

    if (row) {
        /*
         * Update x values tied y.
         */
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

        f->cursor_col = f->buffer_x_offset + (f->cur_x - f->left + 1);
    }


    dir = col > 0 ? 1 : -1;
    for (i = 0; i < dir * col; i += 1) {
        yed_move_cursor_once_x_within_frame(f, dir, line_width);
    }

    if (f->buffer->has_selection && !f->buffer->selection.locked) {
        f->buffer->selection.cursor_row = f->cursor_line;
        f->buffer->selection.cursor_col = f->cursor_col;
    }

    if (row > 1 || row < -1) {
        f->dirty = 1;
    }

}

void yed_set_cursor_far_within_frame(yed_frame *frame, int dst_x, int dst_y) {
    int buff_n_lines;

    if (frame->buffer) {
        buff_n_lines = bucket_array_len(frame->buffer->lines);

        if ((dst_y <  frame->buffer_y_offset + 1)
        ||  (dst_y >= frame->buffer_y_offset + frame->height)) {

            frame->buffer_x_offset = 0;
            frame->buffer_y_offset = MIN(dst_y, MAX(0, buff_n_lines - frame->height));
            frame->cur_x           = frame->desired_x       = frame->left;
            frame->cur_y           = frame->top + frame->scroll_off;
            LIMIT(frame->cur_y,
                    frame->top,
                    MIN(frame->top + frame->height - 1,
                        frame->top + buff_n_lines - frame->buffer_y_offset - 1));
            frame->cursor_col      = 1;
            frame->cursor_line     = frame->buffer_y_offset + frame->cur_y;
        }

        yed_set_cursor_within_frame(frame, dst_x, dst_y);

        frame->dirty = 1;
    }
}

void yed_frame_reset_cursor(yed_frame *frame) {
    yed_set_cursor_far_within_frame(frame, frame->cursor_col, frame->cursor_line);
}

void yed_move_cursor_within_active_frame(int col, int row) {
    if (ys->active_frame) {
        yed_move_cursor_within_frame(ys->active_frame, col, row);
    }
}

void yed_update_frames(void) {
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

int yed_frame_line_is_visible(yed_frame *frame, int row) {
    if (!frame->buffer) {
        return 0;
    }
    return    (row >= frame->buffer_y_offset + 1)
           && (row <= frame->buffer_y_offset + frame->height)
           && (row <= bucket_array_len(frame->buffer->lines));
}

int yed_frame_line_to_y(yed_frame *frame, int row) {
    if (!frame->buffer || !yed_frame_line_is_visible(frame, row)) {
        return 0;
    }

    return frame->top + row - (frame->buffer_y_offset + 1);
}

void yed_frame_update_dirty_line(yed_frame *frame) {
    yed_line *line;
    int       y;

    if (!frame->dirty_line) {
        return;
    }

    y = yed_frame_line_to_y(frame, frame->dirty_line);
    if (y) {
        line = yed_buff_get_line(frame->buffer, frame->dirty_line);
        if (line) {
            yed_frame_draw_line(frame, line, frame->dirty_line, y - frame->top, frame->buffer_x_offset);
        }
    }
}

void yed_frame_update_cursor_line(yed_frame *frame) {
    yed_line *line;
    int       y;

    if (!frame->cursor_line) {
        return;
    }

    y = yed_frame_line_to_y(frame, frame->cursor_line);
    if (y) {

/*         if (frame == ys->active_frame && !frame->buffer->has_selection) { */
/*             append_to_output_buff("\e[0;30;46m"); */
/*             append_to_output_buff(TERM_BG_GREEN); */
/*             append_to_output_buff(TERM_BLACK); */
/*         } */
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);

        ASSERT(line != NULL, "didn't get a cursor line");

        yed_frame_draw_line(frame, line, frame->cursor_line, y - frame->top, frame->buffer_x_offset);
/*         if (frame == ys->active_frame && !frame->buffer->has_selection) { */
/*             append_to_output_buff(TERM_RESET); */
/*             append_to_output_buff(TERM_CURSOR_HIDE); */
/*         } */
    }
}

void yed_mark_dirty_frames(yed_buffer *dirty_buff) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    tree_traverse(ys->frames, it) {
        frame = tree_it_val(it);
        if (frame->buffer == dirty_buff) {
            frame->dirty = 1;
        }
    }
}

void yed_mark_dirty_frames_line(yed_buffer *buff, int dirty_row) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    tree_traverse(ys->frames, it) {
        frame = tree_it_val(it);
        if (frame->buffer == buff) {
            frame->dirty_line = dirty_row;
        }
    }
}
