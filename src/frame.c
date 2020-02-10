void yed_init_frames(void) {
    ys->frames       = array_make(yed_frame*);
    ys->active_frame = NULL;
}

yed_frame * yed_add_new_frame(float top_f, float left_f, float height_f, float width_f) {
    yed_frame *frame;

    frame = yed_new_frame(top_f, left_f, height_f, width_f);

    array_push(ys->frames, frame);

    return frame;
}

yed_frame * yed_add_new_frame_full(void) {
    yed_frame *frame;

    frame = yed_new_frame(0.0, 0.0, 1.0, 1.0);

    array_push(ys->frames, frame);

    return frame;
}

void frame_get_rect(yed_frame *frame, int *top,  int *left,  int *height,  int *width,
                                      int *btop, int *bleft, int *bheight, int *bwidth) {
    int fix_h, fix_w;

    fix_h = fix_w = 0;

    *top  = 1 + (int)(frame->top_f * (ys->term_rows));
    *btop = *top;
    if (*top > 1) {
        *top += 1;
        fix_h = 1;
    }

    *left  = 1 + (int)(frame->left_f * ys->term_cols);
    *bleft = *left;
    if (*left > 1) {
        *left += 1;
        fix_w  = 1;
    }

    *bheight  = (int)(frame->height_f * (ys->term_rows - 2)); /* -2 for command + status line */
    *height   = *bheight;
    if (*btop + *bheight - fix_h < (ys->term_rows - 2)) {
        *height -= 1;
    }
    *height -= fix_h;

    *bwidth  = (int)(frame->width_f * ys->term_cols);
    *width   = *bwidth;
    if (*bleft + *bwidth - 1 < (ys->term_cols)) {
        *width -= 1;
    }
    *width -= fix_w;
}

yed_frame * yed_new_frame(float top_f, float left_f, float height_f, float width_f) {
    yed_frame *frame;

    LIMIT(top_f, 0.0, 1.0);
    LIMIT(left_f, 0.0, 1.0);
    LIMIT(height_f, 0.0, 1.0 - top_f);
    LIMIT(width_f, 0.0, 1.0 - left_f);

    frame = malloc(sizeof(*frame));

    frame->v_link          = NULL;
    frame->h_link          = NULL;
    frame->buffer          = NULL;
    frame->top_f           = top_f;
    frame->left_f          = left_f;
    frame->height_f        = height_f;
    frame->width_f         = width_f;

    FRAME_RESET_RECT(frame);

    frame->cursor_line     = 1;
    frame->last_cursor_line= 1;
    frame->dirty_line      = frame->cursor_line;
    frame->cursor_col      = 1;
    frame->buffer_y_offset = 0;
    frame->buffer_x_offset = 0;
    frame->cur_x           = frame->left;
    frame->cur_y           = frame->top;
    frame->desired_x       = 1;
    frame->dirty           = 1;
    frame->scroll_off      = 5;
    frame->line_attrs      = array_make(yed_attrs);

    return frame;
}

void yed_delete_frame(yed_frame *frame) {
    int        i, save_cursor_row, save_cursor_col;
    yed_frame *new_active_frame, **frame_it;
    yed_event  event;

    event.kind  = EVENT_FRAME_PRE_DELETE;
    event.frame = frame;

    yed_trigger_event(&event);

    if (frame == ys->prev_active_frame) {
        ys->prev_active_frame = NULL;
    }

    new_active_frame = NULL;

    if (frame->v_link) {
        if (frame->v_link->left > frame->left) {
            frame->v_link->left_f = frame->left_f;
        }
        frame->v_link->width_f += frame->width_f;
        LIMIT(frame->v_link->width_f, 0.1, 1.0);
        frame->v_link->v_link = NULL;
        FRAME_RESET_RECT(frame->v_link);
        yed_frame_hard_reset_cursor_x(frame->v_link);
        new_active_frame = frame->v_link;
    } else if (frame->h_link) {
        save_cursor_row = frame->h_link->cursor_line;
        save_cursor_col = frame->h_link->cursor_col;
        if (frame->h_link->top > frame->top) {
            frame->h_link->top_f = frame->top_f;
        }
        frame->h_link->height_f += frame->height_f;
        LIMIT(frame->h_link->height_f, 0.1, 1.0);
        frame->h_link->height += frame->height;
        frame->h_link->h_link = NULL;
        FRAME_RESET_RECT(frame->h_link);
        yed_set_cursor_far_within_frame(frame->h_link, save_cursor_col, save_cursor_row);
        new_active_frame = frame->h_link;
    }

    i = 0;
    array_traverse(ys->frames, frame_it) {
        if ((*frame_it) == frame) {
            break;
        }
        i += 1;
    }

    if (i < array_len(ys->frames)) {
        array_delete(ys->frames, i);
    }

    if (!new_active_frame) {
        new_active_frame = ys->prev_active_frame;
    }

    if (!new_active_frame) {
        if (array_len(ys->frames)) {
            if (i < array_len(ys->frames)) {
                new_active_frame = *(yed_frame**)array_item(ys->frames, i);
            } else {
                new_active_frame = *(yed_frame**)array_item(ys->frames, 0);
            }
        }
    }

    if (ys->active_frame == frame) {
        ys->active_frame = NULL;
        if (new_active_frame) {
            yed_activate_frame(new_active_frame);
        }
    }

    yed_undraw_frame(frame);
    ys->redraw     = 1;
    ys->redraw_cls = 1;

    array_free(frame->line_attrs);

    free(frame);
}

yed_frame * yed_vsplit_frame(yed_frame *frame) {
    yed_frame *new_frame;
    int        orig_width, save_cursor_row, save_cursor_col, new_left_i, new_width_i;
    float      new_left_f, new_width_f;

    save_cursor_row = frame->cursor_line;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_far_within_frame(frame, 1, save_cursor_row);

    orig_width = frame->width;

    frame->width_f /= 2.0;
    FRAME_RESET_RECT(frame);

    new_left_i  = frame->left + frame->width - 1;
    new_width_i = orig_width - frame->width;

    new_left_f   = (float)new_left_i    / (float)(ys->term_cols);
    new_width_f  = (float)new_width_i / (float)(ys->term_cols - 1);

    new_frame = yed_add_new_frame(frame->top_f,
                                  new_left_f,
                                  frame->height_f,
                                  new_width_f);

    frame->v_link     = new_frame;
    new_frame->v_link = frame;

    yed_set_cursor_far_within_frame(frame, save_cursor_col, save_cursor_row);

    return new_frame;
}

yed_frame * yed_hsplit_frame(yed_frame *frame) {
    yed_frame *new_frame;
    int        orig_height, save_cursor_row, save_cursor_col, new_top_i, new_height_i;
    float      new_top_f, new_height_f;

    save_cursor_row = frame->cursor_line;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_far_within_frame(frame, 1, 1);

    orig_height  = frame->height;

    frame->height_f /= 2.0;
    FRAME_RESET_RECT(frame);

    new_top_i    = frame->top + frame->height - 1;
    new_height_i = orig_height - frame->height;

    new_top_f     = (float)new_top_i    / (float)(ys->term_rows);
    new_height_f  = (float)new_height_i / (float)(ys->term_rows - 2); /* -2 for command + status line */

    new_frame = yed_add_new_frame(new_top_f,
                                  frame->left_f,
                                  new_height_f,
                                  frame->width_f);

    frame->h_link     = new_frame;
    new_frame->h_link = frame;

    yed_set_cursor_far_within_frame(frame, save_cursor_col, save_cursor_row);

    return new_frame;
}

void yed_activate_frame(yed_frame *frame) {
    yed_event event;

    event.kind  = EVENT_FRAME_ACTIVATED;
    event.frame = frame;

    if (ys->active_frame && ys->active_frame != frame) {
        ys->prev_active_frame = ys->active_frame;
        ys->active_frame->dirty = 1;
    }

    ys->active_frame = frame;

    /*
     * Correct the cursor if the buffer has changed
     * while this frame was inactive.
     */
    if (frame->buffer && frame->buffer->kind == BUFF_KIND_YANK) {
        yed_set_cursor_far_within_frame(frame, 1, 1);
    }
    yed_frame_reset_cursor(frame);
    ys->active_frame->dirty = 1;
    ys->redraw              = 1;

    yed_trigger_event(&event);
}
#define FRAME_CELL(f, y_off, x_off)                                 \
    (ys->written_cells + (((f)->top + (y_off) - 1) * ys->term_cols) \
                            + (f)->left + (x_off) - 1)

#define FRAME_BCELL(f, y_off, x_off)                                 \
    (ys->written_cells + (((f)->btop + (y_off) - 1) * ys->term_cols) \
                            + (f)->bleft + (x_off) - 1)

void yed_clear_frame(yed_frame *frame) {
    int i, n, x, y, run_len, run_start_n;
    yed_attrs base_attr;
    char *cell;

    x = ys->cur_x;
    y = ys->cur_y;

    if (frame == ys->active_frame) {
        base_attr = yed_active_style_get_active();
    } else {
        base_attr = yed_active_style_get_inactive();
    }

    yed_set_cursor(frame->left, frame->top);
    yed_set_attr(base_attr);

    for (i = 0; i < frame->height; i += 1) {
        yed_set_cursor(frame->left, frame->top + i);

        run_len     = 0;
        run_start_n = 0;
        for (n = 0; n < frame->width; n += 1) {
            cell = FRAME_CELL(frame, i, n);

            if (*cell) {
                yed_set_cursor(frame->left + run_start_n, frame->top + i);
                append_n_to_output_buff(ys->_4096_spaces, run_len);
                run_len     = -1;
                run_start_n = n + 1;
            }

            run_len += 1;
        }

        yed_set_cursor(frame->left + run_start_n, frame->top + i);
        append_n_to_output_buff(ys->_4096_spaces, run_len);
    }

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);

    yed_set_cursor(x, y);
}

void yed_undraw_frame(yed_frame *frame) {
    int   i, n, x, y, run_len, run_start_n;
    char *cell;

    x = ys->cur_x;
    y = ys->cur_y;

    append_to_output_buff(TERM_RESET);

    for (i = 0; i < frame->bheight; i += 1) {
        yed_set_cursor(frame->bleft, frame->btop + i);

        run_len     = 0;
        run_start_n = 0;
        for (n = 0; n < frame->bwidth; n += 1) {
            cell = FRAME_CELL(frame, i, n);

            if (*cell) {
                yed_set_cursor(frame->bleft + run_start_n, frame->btop + i);
                append_n_to_output_buff(ys->_4096_spaces, run_len);
                run_len     = -1;
                run_start_n = n + 1;
            }

            run_len += 1;
        }

        yed_set_cursor(frame->bleft + run_start_n, frame->btop + i);
        append_n_to_output_buff(ys->_4096_spaces, run_len);
    }

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);

    yed_set_cursor(x, y);
}

void yed_frame_draw_line(yed_frame *frame, yed_line *line, int row, int y_offset, int x_offset) {
    yed_attrs  cur_attr, base_attr, sel_attr, tmp_attr, *attr_it;
    int        col, n_col, first_idx, first_col, width_skip, col_off, run_col_off, run_len, width, n_bytes, i, all_cols_visible;
    char      *cell, *bytes, *run_start;
    yed_event  event;


    /* Helper macros */
    #define DUMP_RUN()                                                    \
    do {                                                                  \
        yed_set_cursor(frame->left + run_col_off, frame->top + y_offset); \
        yed_set_attr(cur_attr);                                           \
        append_n_to_output_buff(run_start, run_len);                      \
    } while (0)

    #define NEXT_RUN()                                                    \
    do {                                                                  \
        run_start    = bytes;                                             \
        run_len      = 0;                                                 \
        run_col_off  = col_off;                                           \
    } while (0)


    /*
     * Determine what the baseline attributes of text should
     * look like.
     */
    if (frame == ys->active_frame
    &&  frame->cursor_line == row
    &&  !frame->buffer->has_selection
    &&  yed_get_var("cursor-line")) {

        base_attr = yed_active_style_get_cursor_line();
    } else if (frame == ys->active_frame) {
        base_attr = yed_active_style_get_active();
    } else {
        base_attr = yed_active_style_get_inactive();
    }


    /*
     * Store the base attributes for each column.
     * They might be modified later on.
     */
    array_clear(frame->line_attrs);
    for (col = 1; col <= line->visual_width; col += 1) {
        array_push(frame->line_attrs, base_attr);
    }

    /*
     * Find out if there are any columns that are in a selection.
     * If there are, apply the selection attributes to them in the
     * line_attrs array.
     */
    if (ys->active_frame == frame && frame->buffer->has_selection) {
        col = 1;
        if (ys->active_style) {
            sel_attr = yed_active_style_get_selection();
        } else {
            sel_attr        = base_attr;
            sel_attr.flags |= ATTR_INVERSE;
        }

        array_traverse(frame->line_attrs, attr_it) {
            if (yed_is_in_range(&frame->buffer->selection, row, col)) {
                yed_combine_attrs(attr_it, &sel_attr);
            }
            col += 1;
        }
    }

    /*
     * We're about to start drawing.
     * Do the pre-draw event.
     */
    event.kind       = EVENT_LINE_PRE_DRAW;
    event.frame      = frame;
    event.row        = row;
    event.line_attrs = frame->line_attrs;

    yed_trigger_event(&event);

    /*
     * Compute how many columns we're actually going to draw.
     * This is NOT necessarily just the line length or the
     * frame width.
     */
    n_col = MIN(MAX(line->visual_width - x_offset, 0), frame->width);

    first_col = (line->visual_width < x_offset) ? line->visual_width : x_offset + 1;
    first_idx = yed_line_col_to_idx(line, first_col);
    bytes     = array_item(line->chars, first_idx);
    width_skip = first_col - yed_line_idx_to_col(line, first_idx);

    /* Set up first run. */
    run_start   = bytes;
    run_len     = 0;
    run_col_off = 0;

    /* Set the initial attrs. */
    if (line->visual_width) {
        cur_attr = *(yed_attrs*)array_item(frame->line_attrs, 0);
        yed_set_attr(cur_attr);
    } else {
        cur_attr = base_attr;
        yed_set_attr(cur_attr);
    }

    /*
     * Okay, let's draw.
     */
    for (col_off = 0; col_off < n_col;) {
        width            = yed_get_glyph_width(bytes);
        n_bytes          = yed_get_glyph_len(bytes);
        all_cols_visible = 1;

        if (*bytes == '\t') {
            DUMP_RUN();

            for (i = width_skip; i < width && col_off < n_col; i += 1) {
                cell = FRAME_CELL(frame, y_offset, col_off);
                if (!*cell) {
                    tmp_attr = *(yed_attrs*)array_item(frame->line_attrs, first_col + col_off - 1);
                    if (!yed_attrs_eq(tmp_attr, cur_attr)) {
                        yed_set_attr(tmp_attr);
                        cur_attr = tmp_attr;
                    }
                    yed_set_cursor(frame->left + col_off, frame->top + y_offset);
                    append_n_to_output_buff(" ", 1);
                }
                col_off += 1;
            }

            /* Skip it so it's not in the next run. */
            bytes += n_bytes;

            NEXT_RUN();
        } else {
            for (i = width_skip; i < width && col_off < n_col; i += 1) {
                cell = FRAME_CELL(frame, y_offset, col_off);
                if (*cell) {
                    all_cols_visible = 0;
                    break;
                }
            }

            if (all_cols_visible) {
                for (i = width_skip; i < width && col_off < n_col; i += 1) {
                    tmp_attr = *(yed_attrs*)array_item(frame->line_attrs, first_col + col_off - 1);
                    if (yed_attrs_eq(tmp_attr, cur_attr)) {
                        run_len += n_bytes;
                    } else {
                        DUMP_RUN();
                        cur_attr = tmp_attr;
                        NEXT_RUN();
                        run_len += n_bytes;
                    }
                    col_off += 1;
                }
                bytes += n_bytes;
            } else {
                DUMP_RUN();
                col_off += width;
                bytes   += n_bytes;
                NEXT_RUN();
            }
        }

        width_skip = 0;
    }

    DUMP_RUN();

    bytes    = ys->_4096_spaces;
    cur_attr = base_attr;
    NEXT_RUN();

    for (; col_off < frame->width;) {
        cell = FRAME_CELL(frame, y_offset, col_off);

        if (*cell) {
            DUMP_RUN();
            col_off += 1;
            NEXT_RUN();
        } else {
            run_len += 1;
            col_off += 1;
        }
    }

    DUMP_RUN();

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
}

void yed_frame_draw_fill(yed_frame *frame, int y_offset) {
    int i, n, run_len, run_start_n;
    yed_attrs base_attr;
    char *cell;

    if (frame == ys->active_frame) {
        base_attr = yed_active_style_get_active();
    } else {
        base_attr = yed_active_style_get_inactive();
    }

    yed_set_cursor(frame->left, frame->top + y_offset);
    yed_set_attr(base_attr);

    for (i = 0; i < frame->height - y_offset; i += 1) {
        cell = FRAME_CELL(frame, y_offset + i, 0);

        if (*cell) {
            yed_set_cursor(frame->left + 1, frame->top + y_offset + i);
        } else {
            yed_set_cursor(frame->left, frame->top + y_offset + i);
            append_n_to_output_buff("~", 1);
        }

        yed_set_cursor(frame->left, frame->top + i);

        run_len     = 0;
        run_start_n = 1;
        for (n = 1; n < frame->width; n += 1) {
            cell = FRAME_CELL(frame, y_offset + i, n);

            if (*cell) {
                yed_set_cursor(frame->left + run_start_n, frame->top + y_offset + i);
                append_n_to_output_buff(ys->_4096_spaces, run_len);
                run_len     = -1;
                run_start_n = n + 1;
            }

            run_len += 1;
        }

        yed_set_cursor(frame->left + run_start_n, frame->top + y_offset + i);
        append_n_to_output_buff(ys->_4096_spaces, run_len);
    }

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
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

void yed_frame_set_pos(yed_frame *frame, float top_f, float left_f) {
    frame->top_f  = top_f;
    frame->left_f = left_f;

    FRAME_RESET_RECT(frame);
}

void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff) {
    yed_buffer *old_buff;

    old_buff      = frame->buffer;
    frame->buffer = buff;
    frame->dirty  = 1;

    if (old_buff) {
        if (!yed_buff_is_visible(old_buff)) {
            old_buff->last_cursor_row = frame->cursor_line;
            old_buff->last_cursor_col = frame->cursor_col;
        }
    }

    if (buff) {
        yed_set_cursor_far_within_frame(frame, 1, 1);
        yed_set_cursor_within_frame(frame, buff->last_cursor_col, buff->last_cursor_row);
    }
}

void yed_frame_draw_border(yed_frame *frame) {
    yed_attrs  attr;
    char      *cell, *t, *b, *l, *r, *tl, *tr, *bl, *br;
    int        i;

    if (frame == ys->active_frame) {
        attr = yed_active_style_get_active_border();
    } else {
        attr = yed_active_style_get_inactive_border();
    }
    yed_set_attr(attr);


    t  = "─";
    b  = "─";
    l  = "│";
    r  = "│";
    tl = "┌";
    tr = "┐";
    bl = "└";
    br = "┘";

/*     t  = "▀"; */
/*     b  = "▄"; */
/*     l  = "▌"; */
/*     r  = "▐"; */
/*     tl = "▛"; */
/*     tr = "▜ "; */
/*     bl = "▙"; */
/*     br = "▟"; */

    /* top */
    if (frame->top > 1) {
        for (i = 0; i < frame->bwidth; i += 1) {
            cell = FRAME_BCELL(frame, 0, i);

            if (!*cell) {
                yed_set_cursor(frame->bleft + i, frame->btop);
                if (i == 0 && frame->bleft > 1) {
                    append_to_output_buff(tl);
                } else if (i == frame->bwidth - 1 && frame->bleft + frame->bwidth - 1 < ys->term_cols) {
                    append_to_output_buff(tr);
                } else {
                    append_to_output_buff(t);
                }
                *cell = 1;
            }
        }
    }

    /* bottom */
    if (frame->btop + frame->bheight - 1 < (ys->term_rows - 2)) {
        for (i = 0; i < frame->bwidth; i += 1) {
            cell = FRAME_BCELL(frame, frame->bheight - 1, i);

            if (!*cell) {
                yed_set_cursor(frame->bleft + i, frame->btop + frame->bheight - 1);
                if (i == 0 && frame->bleft > 1) {
                    append_to_output_buff(bl);
                } else if (i == frame->bwidth - 1 && frame->bleft + frame->bwidth - 1 < ys->term_cols) {
                    append_to_output_buff(br);
                } else {
                    append_to_output_buff(b);
                }
                *cell = 1;
            }
        }
    }

    /* left */
    if (frame->left > 1) {
        for (i = 0; i < frame->bheight; i += 1) {
            cell = FRAME_BCELL(frame, i, 0);

            if (!*cell) {
                yed_set_cursor(frame->bleft, frame->btop + i);
                append_to_output_buff(l);
                *cell = 1;
            }
        }
    }

    /* right */
    if (frame->bleft + frame->bwidth - 1 < (ys->term_cols)) {
        for (i = 0; i < frame->bheight; i += 1) {
            cell = FRAME_BCELL(frame, i, frame->bwidth - 1);

            if (!*cell) {
                yed_set_cursor(frame->bleft + frame->bwidth - 1, frame->btop + i);
                append_to_output_buff(r);
                *cell = 1;
            }
        }
    }

    append_to_output_buff(TERM_RESET);
}

void yed_frame_update(yed_frame *frame) {
    int        i;
    char      *cell_row;
    yed_event  update_event, buff_draw_event;

    if (frame->last_cursor_line - frame->cursor_line > 1
    ||  frame->last_cursor_line - frame->cursor_line < -1) {

        frame->dirty = 1;
    }

    if (frame->dirty && frame->buffer) {
        yed_mark_dirty_frames(frame->buffer);
    }

    update_event.kind     = EVENT_FRAME_PRE_UPDATE;
    update_event.frame    = frame;
    buff_draw_event.kind  = EVENT_FRAME_PRE_BUFF_DRAW;
    buff_draw_event.frame = frame;


    yed_trigger_event(&update_event);

    if (ys->redraw) {
        FRAME_RESET_RECT(frame);
        yed_frame_draw_border(frame);
    }

    if (frame->buffer) {
        if (frame->dirty || ys->redraw) {
            yed_trigger_event(&buff_draw_event);

            yed_frame_draw_buff(frame, frame->buffer, frame->buffer_y_offset, frame->buffer_x_offset);
        } else {
            yed_frame_update_dirty_line(frame);
            if (frame == ys->active_frame
            &&  frame->cursor_line != frame->dirty_line) {
                yed_frame_update_cursor_line(frame);
            }
        }
    } else {
        if (frame->dirty || ys->redraw) {
            yed_trigger_event(&buff_draw_event);
            yed_clear_frame(frame);
        }
    }

    for (i = 0; i < frame->bheight; i += 1) {
        cell_row = FRAME_BCELL(frame, i, 0);
        memset(cell_row, 1, frame->bwidth);
    }

    frame->dirty = frame->dirty_line = 0;
    frame->last_cursor_line = frame->cursor_line;
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
            if (new_y > bot - 1 || new_y < f->top) {
                f->buffer_y_offset += dir;
                f->dirty            = 1;
            } else {
                f->cur_y = new_y;
            }
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
    int       new_x, width, old_x_off;
    yed_line *line;

    line = yed_buff_get_line(f->buffer, f->cursor_line);

    if (line->visual_width == 0) { return; }

    width = 1;

    if (dir > 0 && f->cursor_col <= line->visual_width) {
        dir   = 1;
        width = yed_get_glyph_width(array_item(line->chars, yed_line_col_to_idx(line, f->cursor_col)));
    } else if (dir < 0 && f->cursor_col > 1) {
        dir   = -1;
        width = yed_get_glyph_width(array_item(line->chars, yed_line_col_to_idx(line, f->cursor_col - 1)));
    }

    old_x_off  = f->buffer_x_offset;
    dir       *= width;

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

    LIMIT(f->buffer_x_offset, 0, line_width);
    (void)old_x_off;
    if (width > 1) {
        f->cur_x += f->buffer_x_offset - old_x_off;
    }
    LIMIT(f->cur_x, f->left, f->left + line_width - f->buffer_x_offset);

    f->cursor_col = f->buffer_x_offset + (f->cur_x - f->left + 1);
    f->desired_x  = f->cursor_col;
}

void yed_set_cursor_within_frame(yed_frame *f, int new_x, int new_y) {
    yed_event  event;
    int        col, row,
               line_width;
    yed_line  *line;

    row = new_y - f->cursor_line;
    yed_move_cursor_within_frame(f, 0, row);

    line       = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width = line->visual_width;

    col = new_x - f->cursor_col;

    if (f->cursor_col + col > line_width) {
        f->cur_x      = MIN(f->desired_x - 1, line_width) + f->left - f->buffer_x_offset;
        f->cursor_col = f->buffer_x_offset + (f->cur_x - f->left + 1);

        col = new_x - f->cursor_col;
    }

    yed_move_cursor_within_frame(f, col, 0);

    event.kind  = EVENT_CURSOR_MOVED;
    event.frame = f;

    yed_trigger_event(&event);
}

void yed_move_cursor_within_frame(yed_frame *f, int col, int row) {
    int       i,
              dir,
              line_width,
              potential_new_x,
              buff_n_lines,
              buff_big_enough_to_scroll,
              save_cursor_line;
    yed_line *line;
    yed_event event;

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
        potential_new_x  = f->desired_x;

        if (f->desired_x > line_width) {
            potential_new_x = line_width + 1;
        }

        if (potential_new_x <= f->buffer_x_offset) {
            f->buffer_x_offset = MAX(0, potential_new_x - f->width);
            f->desired_x       = potential_new_x;
            f->dirty           = 1;
        }

        if (f->desired_x <= line->visual_width) {
            f->desired_x = yed_line_idx_to_col(line, yed_line_col_to_idx(line, f->desired_x));
        }

        f->cur_x = MIN(f->desired_x - 1, line_width) + f->left - f->buffer_x_offset;

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

    event.kind  = EVENT_CURSOR_MOVED;
    event.frame = f;

    yed_trigger_event(&event);

}

void yed_set_cursor_far_within_frame(yed_frame *frame, int dst_x, int dst_y) {
    int buff_n_lines;

    if (frame->buffer) {
        buff_n_lines = bucket_array_len(frame->buffer->lines);

        if ((dst_y <  frame->buffer_y_offset + 1)
        ||  (dst_y >= frame->buffer_y_offset + frame->height)) {

            frame->buffer_x_offset = 0;
            frame->buffer_y_offset = MIN(dst_y, MAX(0, buff_n_lines - frame->height));
            frame->desired_x       = 1;
            frame->cur_x           = frame->left;
            frame->cur_y           = frame->top + frame->scroll_off;
            LIMIT(frame->cur_y,
                    frame->top,
                    MIN(frame->top + frame->height - 1,
                        frame->top + buff_n_lines - frame->buffer_y_offset - 1));
            frame->cursor_col      = 1;
            frame->cursor_line     = frame->buffer_y_offset + (frame->cur_y - frame->top + 1);
        }

        yed_set_cursor_within_frame(frame, dst_x, dst_y);

        frame->dirty = 1;
    }
}

void yed_frame_reset_cursor(yed_frame *frame) {
    if (frame->cur_y < frame->top
    ||  frame->cur_y >= frame->top + frame->height
    ||  frame->cur_x < frame->left
    ||  frame->cur_x >= frame->left + frame->width) {

        frame->cur_y = frame->top;
        frame->cur_x = frame->left;
    }

    if (frame->buffer) {
        yed_set_cursor_far_within_frame(frame, frame->cursor_col, frame->cursor_line);
    } else {
        frame->cur_y = frame->top;
        frame->cur_x = frame->left;
    }
}

void yed_frame_hard_reset_cursor_x(yed_frame *frame) {
    int dst_x;

    if (frame->buffer) {
        dst_x = frame->cursor_col;
        frame->buffer_x_offset = 0;
        frame->cur_x = frame->left;
        frame->cursor_col = 1;
        yed_set_cursor_within_frame(frame, dst_x, frame->cursor_line);
    } else {
        frame->cur_y = frame->top;
        frame->cur_x = frame->left;
    }
}

void yed_move_cursor_within_active_frame(int col, int row) {
    if (ys->active_frame) {
        yed_move_cursor_within_frame(ys->active_frame, col, row);
    }
}

void yed_update_frames(void) {
    yed_frame **frame;

    if (ys->active_frame) {
        yed_frame_update(ys->active_frame);
    }

    array_rtraverse(ys->frames, frame) {
        if (*frame == ys->active_frame) {
            continue;
        }
        yed_frame_update(*frame);
    }

    if (ys->active_frame) {
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

void yed_frames_remove_buffer(yed_buffer *buff) {
    yed_frame **frame;

    array_traverse(ys->frames, frame) {
        if ((*frame)->buffer == buff) {
            yed_set_cursor_far_within_frame((*frame), 1, 1);
            (*frame)->buffer = NULL;
            (*frame)->dirty  = 1;
        }
    }
}

void yed_mark_dirty_frames(yed_buffer *dirty_buff) {
    yed_frame **frame;

    array_traverse(ys->frames, frame) {
        if ((*frame)->buffer == dirty_buff) {
            (*frame)->dirty = 1;
        }
    }
}

void yed_mark_dirty_frames_line(yed_buffer *buff, int dirty_row) {
    yed_frame **frame;

    array_traverse(ys->frames, frame) {
        if ((*frame)->buffer == buff) {
            (*frame)->dirty_line = dirty_row;
        }
    }
}
