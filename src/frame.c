void yed_init_frames(void) {
    ys->frames       = array_make(yed_frame*);
    ys->active_frame = NULL;
}

yed_frame * yed_add_new_frame(float top_f, float left_f, float height_f, float width_f) {
    yed_frame *frame;

    frame = yed_new_frame(top_f, left_f, height_f, width_f);

    array_push(ys->frames, frame);

    ys->redraw = 1;

    return frame;
}

yed_frame * yed_add_new_frame_full(void) {
    yed_frame *frame;

    frame = yed_new_frame(0.0, 0.0, 1.0, 1.0);

    array_push(ys->frames, frame);

    ys->redraw = 1;

    return frame;
}

void frame_get_rect(yed_frame *frame, int *top,  int *left,  int *height,  int *width,
                                      int *btop, int *bleft, int *bheight, int *bwidth) {

    /* Use of -2 below is to account for command + status lines. */

    /* Measurements including possible borders. */
    *btop    = 1 + (int)(roundf(frame->top_f    * (ys->term_rows - 2)));
    *bleft   = 1 + (int)(roundf(frame->left_f   * ys->term_cols));
    *bheight =     (int)(roundf(frame->height_f * (ys->term_rows - 2)));
    *bwidth  =     (int)(roundf(frame->width_f  * ys->term_cols));

    if (!(*bleft & 1)) { *bwidth += 1; }

    /* Sanity checks and limitations. */
    LIMIT(*btop,    1, ys->term_rows - 2);
    LIMIT(*bheight, 1, ys->term_rows - 2);
    LIMIT(*bleft,   1, ys->term_cols);
    LIMIT(*bwidth,  1, ys->term_cols);

    while (*btop + *bheight - 1 > (ys->term_rows - 2)) { *bheight -= 1; }
    while (*bleft + *bwidth - 1 > ys->term_cols)       { *bwidth  -= 1; }

    /* Now the area inside any possible borders. */
    *top    = *btop  + (*btop > 1);  /* When btop is 1, so is top.   */
    *left   = *bleft + (*bleft > 1); /* When bleft is 1, so is left. */
    *height = *bheight
                - (*top != *btop)                                /* 1 shorter if there's a border row on the top... */
                - ((*btop + *bheight - 1) != ys->term_rows - 2); /* ... or the bottom.                              */
    *width  = *bwidth
                - (*left != *bleft)                              /* 1 shorter if there's a border row on the left... */
                - ((*bleft + *bwidth - 1) != ys->term_cols);     /* ... or the right.                                */
}

yed_frame * yed_new_frame(float top_f, float left_f, float height_f, float width_f) {
    yed_frame *frame;

    LIMIT(top_f, 0.0, 1.0);
    LIMIT(left_f, 0.0, 1.0);
    LIMIT(height_f, 0.0, 1.0 - top_f);
    LIMIT(width_f, 0.0, 1.0 - left_f);

    height_f = roundf(height_f * (float)(ys->term_cols - 2)) / (float)(ys->term_cols - 2);
    width_f  = roundf(width_f * (float)ys->term_cols) / (float)ys->term_cols;

    frame = malloc(sizeof(*frame));

    frame->buffer       = NULL;
    frame->top_f        = top_f;
    frame->left_f       = left_f;
    frame->height_f     = height_f;
    frame->width_f      = width_f;
    frame->gutter_width = 0;

    FRAME_RESET_RECT(frame);


    frame->cursor_line          = 1;
    frame->last_cursor_line     = 1;
    frame->dirty_line           = frame->cursor_line;
    frame->cursor_line_is_dirty = 1;
    frame->cursor_col           = 1;
    frame->buffer_y_offset      = 0;
    frame->buffer_x_offset      = 0;
    frame->cur_x                = frame->left + frame->gutter_width;
    frame->cur_y                = frame->top;
    frame->desired_col          = 1;
    frame->dirty                = 1;
    frame->scroll_off           = yed_get_default_scroll_offset();
    frame->line_attrs           = array_make(yed_attrs);
    frame->gutter_glyphs        = array_make(char);
    frame->gutter_attrs         = array_make(yed_attrs);

    frame->tree = yed_frame_tree_add_root(frame);

    return frame;
}

void yed_delete_frame(yed_frame *frame) {
    int             i;
    yed_frame      *new_active_frame, **frame_it;
    yed_event       event;
    yed_frame_tree *next_leaf;

    memset(&event, 0, sizeof(event));
    event.kind  = EVENT_FRAME_PRE_DELETE;
    event.frame = frame;

    yed_trigger_event(&event);

    if (event.cancel) { return; }

    if (frame == ys->prev_active_frame) {
        ys->prev_active_frame = NULL;
    }

    new_active_frame = NULL;

    if (frame->tree) {
        if (!yed_frame_tree_is_root(frame->tree)) {
            next_leaf = yed_frame_tree_get_split_leaf_prefer_left_or_topmost(frame->tree);
            if (next_leaf) {
                new_active_frame = next_leaf->frame;
            }
        }
        yed_frame_tree_delete_leaf(frame->tree);
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

    if (array_len(ys->frames) == 0) {
        ys->active_frame = ys->prev_active_frame = NULL;
    }
}

yed_frame * yed_vsplit_frame(yed_frame *frame) {
    int        save_cursor_row, save_cursor_col;
    yed_frame *new_frame;

    save_cursor_row = frame->cursor_line;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_far_within_frame(frame, save_cursor_row, 1);

    yed_frame_tree_leaf_vsplit(frame->tree);

    new_frame = frame->tree->parent->child_trees[1]->frame;

    yed_set_cursor_far_within_frame(frame, save_cursor_row, save_cursor_col);

    return new_frame;
}

yed_frame * yed_hsplit_frame(yed_frame *frame) {
    int        save_cursor_row, save_cursor_col;
    yed_frame *new_frame;

    save_cursor_row = frame->cursor_line;
    save_cursor_col = frame->cursor_col;

    yed_set_cursor_far_within_frame(frame, save_cursor_row, 1);

    yed_frame_tree_leaf_hsplit(frame->tree);

    new_frame = frame->tree->parent->child_trees[1]->frame;

    yed_set_cursor_far_within_frame(frame, save_cursor_row, save_cursor_col);

    return new_frame;
}

void yed_activate_frame(yed_frame *frame) {
    int       buff_n_lines;
    int       save_cursor_line;
    yed_event event;

    if (ys->active_frame && ys->active_frame != frame) {
        ys->prev_active_frame = ys->active_frame;
        ys->active_frame->dirty = 1;
    }

    ys->active_frame = frame;

    /*
     * Correct the cursor if the buffer has changed.
     */
    if (frame->buffer) {
        buff_n_lines = bucket_array_len(frame->buffer->lines);
        if (frame->cursor_line > buff_n_lines) {
            save_cursor_line = frame->cursor_line;
            yed_set_cursor_far_within_frame(frame, 1, 1);
            yed_set_cursor_far_within_frame(frame, save_cursor_line, 1);
        } else if (frame->buffer->kind == BUFF_KIND_YANK) {
            yed_set_cursor_far_within_frame(frame, 1, 1);
        }
    }

    yed_frame_reset_cursor(frame);
    ys->active_frame->dirty = 1;
    ys->redraw              = 1;

    memset(&event, 0, sizeof(event));
    event.kind  = EVENT_FRAME_ACTIVATED;
    event.frame = frame;
    yed_trigger_event(&event);

    if (frame->buffer != NULL) {
        memset(&event, 0, sizeof(event));
        event.kind   = EVENT_BUFFER_FOCUSED;
        event.frame  = frame;
        event.buffer = frame->buffer;
        yed_trigger_event(&event);
    }
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
    append_to_output_buff(TERM_CURSOR_HIDE);

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

char _get_nprint_char(unsigned char nprint) {
    char c;

    if (nprint <= 0x1F) {
        c = nprint | 0x40;
        goto out;
    }

    switch (nprint) {
        case 0x7F: c = '?'; break;
    }

out:
    return c;
}

void yed_frame_draw_line(yed_frame *frame, yed_line *line, int row, int y_offset, int x_offset) {
    yed_attrs  cur_attr, base_attr, sel_attr, tmp_attr, *attr_it;
    int        col, n_col, first_idx, first_col, width_skip, col_off, run_col_off, run_len, width, n_bytes, i, all_cols_visible, nprint_glyph_pos;
    char       nprint_chars[2] = { '^', '?' };
    char      *cell, *bytes, *gutter_bytes, *run_start;
    yed_event  event;
    int        save_gutter_width;


    /* Helper macros */
    #define DUMP_RUN()                                                    \
    do {                                                                  \
        yed_set_cursor(frame->left + run_col_off + frame->gutter_width,   \
                       frame->top + y_offset);                            \
        yed_set_attr(cur_attr);                                           \
        append_n_to_output_buff(run_start, run_len);                      \
    } while (0)

    #define NEXT_RUN()                                                    \
    do {                                                                  \
        run_start    = bytes;                                             \
        run_len      = 0;                                                 \
        run_col_off  = col_off;                                           \
    } while (0)

    #define DUMP_RUN_GUTTER()                                             \
    do {                                                                  \
        yed_set_cursor(frame->left + run_col_off,                         \
                       frame->top + y_offset);                            \
        yed_set_attr(cur_attr);                                           \
        append_n_to_output_buff(run_start, run_len);                      \
    } while (0)

    #define NEXT_RUN_GUTTER()                                             \
    do {                                                                  \
        run_start    = gutter_bytes;                                      \
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
    &&  yed_var_is_truthy("cursor-line")) {

        base_attr = yed_active_style_get_cursor_line();
    } else if (frame == ys->active_frame) {
        base_attr = yed_active_style_get_active();
    } else {
        base_attr = yed_active_style_get_inactive();
    }


    memset(&event, 0, sizeof(event));
    event.kind          = EVENT_ROW_PRE_CLEAR;
    event.frame         = frame;
    event.row           = row;
    event.row_base_attr = base_attr;

    yed_trigger_event(&event);

    base_attr = event.row_base_attr;

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

again_gutter:

    array_clear(frame->gutter_glyphs);
    array_clear(frame->gutter_attrs);
    if (frame == ys->active_frame) {
        cur_attr = yed_active_style_get_active_gutter();
    } else {
        cur_attr = yed_active_style_get_inactive_gutter();
    }
    for (col = 1; col <= frame->gutter_width; col += 1) {
        array_push(frame->gutter_attrs, cur_attr);
    }

    save_gutter_width = frame->gutter_width;

    /*
     * We're about to start drawing.
     * Do the pre-draw event.
     */
    memset(&event, 0, sizeof(event));
    event.kind           = EVENT_LINE_PRE_DRAW;
    event.frame          = frame;
    event.row            = row;
    event.row_base_attr  = base_attr;
    event.line_attrs     = frame->line_attrs;
    event.gutter_glyphs  = frame->gutter_glyphs;
    event.gutter_attrs   = frame->gutter_attrs;

    yed_trigger_event(&event);

    if (frame->gutter_width != save_gutter_width) { goto again_gutter; }

    frame->line_attrs    = event.line_attrs;
    frame->gutter_glyphs = event.gutter_glyphs;
    frame->gutter_attrs  = event.gutter_attrs;

    /*
     * Okay, let's draw..
     */

    /*
     * First, draw the gutter.
     */
    if (frame->gutter_width > 0) {
        array_zero_term(frame->gutter_glyphs);
        gutter_bytes = array_data(frame->gutter_glyphs);

        /* Set up first run. */
        run_start   = gutter_bytes;
        run_len     = 0;
        run_col_off = 0;

        cur_attr = *(yed_attrs*)array_item(frame->gutter_attrs, 0);
        yed_set_attr(cur_attr);

        col_off = 0;
        for (gutter_bytes = event.gutter_glyphs.data;
            (void*)gutter_bytes < (event.gutter_glyphs.data + (event.gutter_glyphs.used * event.gutter_glyphs.elem_size)); ) {

            width = yed_get_glyph_width(*(yed_glyph*)gutter_bytes);
            if (col_off + width > frame->gutter_width) { break; }

            n_bytes = yed_get_glyph_len(*(yed_glyph*)gutter_bytes);

            all_cols_visible = 1;

            for (i = 0; i < width; i += 1) {
                all_cols_visible &= !*FRAME_CELL(frame, y_offset, col_off + i);
            }

            if (all_cols_visible) {
                tmp_attr = *(yed_attrs*)array_item(frame->gutter_attrs, col_off);
                if (yed_attrs_eq(tmp_attr, cur_attr)) {
                    run_len += n_bytes;
                } else {
                    DUMP_RUN_GUTTER();
                    cur_attr = tmp_attr;
                    NEXT_RUN_GUTTER();
                    run_len += n_bytes;
                }
                col_off      += width;
                gutter_bytes += n_bytes;
            } else {
                DUMP_RUN_GUTTER();
                col_off      += width;
                gutter_bytes += n_bytes;
                NEXT_RUN_GUTTER();
            }
        }

        if (run_len) {
            DUMP_RUN_GUTTER();
        }

        gutter_bytes = ys->_4096_spaces;

        run_start   = gutter_bytes;
        run_len     = 0;
        run_col_off = col_off;
        for (; col_off < frame->gutter_width;) {
            cell = FRAME_CELL(frame, y_offset, col_off);
            if (!*cell) {
                tmp_attr = *(yed_attrs*)array_item(frame->gutter_attrs, col_off);

                if (yed_attrs_eq(tmp_attr, cur_attr)) {
                    run_len += 1;
                } else {
                    DUMP_RUN_GUTTER();
                    cur_attr = tmp_attr;
                    NEXT_RUN_GUTTER();
                    run_len += 1;
                }

                col_off += 1;
            } else {
                DUMP_RUN_GUTTER();
                col_off += 1;
                NEXT_RUN_GUTTER();
            }
        }

        if (run_len > 0) {
            DUMP_RUN_GUTTER();
        }
    }

    /* Set the initial attrs. */
    if (line->visual_width) {
        cur_attr = *(yed_attrs*)array_item(frame->line_attrs, 0);
        yed_set_attr(cur_attr);
    } else {
        cur_attr = base_attr;
        yed_set_attr(cur_attr);
    }

    /*
     * Compute how many columns we're actually going to draw.
     * This is NOT necessarily just the line length or the
     * frame width.
     */
    n_col  = MIN(MAX(line->visual_width - x_offset, 0), frame->width - frame->gutter_width);

    first_col = (line->visual_width < x_offset) ? line->visual_width : x_offset + 1;
    first_idx = yed_line_col_to_idx(line, first_col);
    bytes     = array_item(line->chars, first_idx);
    width_skip = first_col - yed_line_idx_to_col(line, first_idx);

    /* Set up first run. */
    run_start   = bytes;
    run_len     = 0;
    run_col_off = 0;

    /*
     * Now draw the rest of the line.
     */
    for (col_off = 0; col_off < n_col;) {
        width            = yed_get_glyph_width(*(yed_glyph*)bytes);
        n_bytes          = yed_get_glyph_len(*(yed_glyph*)bytes);
        all_cols_visible = 1;

        if (*bytes == '\t') {
            DUMP_RUN();

            for (i = width_skip; i < width && col_off < n_col; i += 1) {
                cell = FRAME_CELL(frame, y_offset, col_off + frame->gutter_width);
                if (!*cell) {
                    tmp_attr = *(yed_attrs*)array_item(frame->line_attrs, first_col + col_off - 1);
                    if (!yed_attrs_eq(tmp_attr, cur_attr)) {
                        yed_set_attr(tmp_attr);
                        cur_attr = tmp_attr;
                    }
                    yed_set_cursor(frame->left + frame->gutter_width + col_off, frame->top + y_offset);
                    append_n_to_output_buff(" ", 1);
                }
                col_off += 1;
            }

            /* Skip it so it's not in the next run. */
            bytes += n_bytes;

            NEXT_RUN();
        } else if (n_bytes == 1 && !isprint(*bytes)) {
            DUMP_RUN();

            nprint_chars[1] = _get_nprint_char(*(unsigned char*)bytes);

            nprint_glyph_pos = 0;

            for (i = width_skip; i < width && col_off < n_col; i += 1) {
                cell = FRAME_CELL(frame, y_offset, col_off + frame->gutter_width);
                if (!*cell) {
                    tmp_attr = *(yed_attrs*)array_item(frame->line_attrs, first_col + col_off - 1);
                    if (!yed_attrs_eq(tmp_attr, cur_attr)) {
                        yed_set_attr(tmp_attr);
                        cur_attr = tmp_attr;
                    }
                    yed_set_cursor(frame->left + frame->gutter_width + col_off, frame->top + y_offset);
                    append_n_to_output_buff(nprint_chars + nprint_glyph_pos, 1);
                }
                col_off          += 1;
                nprint_glyph_pos += 1;
            }

            /* Skip it so it's not in the next run. */
            bytes += n_bytes;

            NEXT_RUN();
        } else {
            for (i = width_skip; i < width && col_off < n_col; i += 1) {
                cell = FRAME_CELL(frame, y_offset, col_off + frame->gutter_width);
                if (*cell) {
                    all_cols_visible = 0;
                    break;
                }
            }

            if (all_cols_visible) {
                tmp_attr = *(yed_attrs*)array_item(frame->line_attrs, first_col + col_off - 1);
                if (yed_attrs_eq(tmp_attr, cur_attr)) {
                    run_len += n_bytes;
                } else {
                    DUMP_RUN();
                    cur_attr = tmp_attr;
                    NEXT_RUN();
                    run_len += n_bytes;
                }
                col_off += width;
                bytes   += n_bytes;
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

    for (; col_off < frame->width - frame->gutter_width;) {
        cell = FRAME_CELL(frame, y_offset, col_off + frame->gutter_width);

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
    char *fill_str;
    int   fill_str_len;
    int i, j, n, run_len, run_start_n;
    yed_attrs base_attr;
    yed_attrs gut_attr;
    char *fill_scomp;
    int   fill_scomp_nr;
    yed_attrs fill_attr_pre;
    yed_attrs fill_attr;
    char *cell;

    if ((fill_str = yed_get_var("fill-string")) == NULL) {
        fill_str = DEFAULT_FILL_STRING;
    }
    fill_str_len = strlen(fill_str);

    if (frame == ys->active_frame) {
        base_attr = yed_active_style_get_active();
        gut_attr  = yed_active_style_get_active_gutter();
    } else {
        base_attr = yed_active_style_get_inactive();
        gut_attr  = yed_active_style_get_inactive_gutter();
    }

    fill_scomp = yed_get_var("fill-scomp");
    if (fill_scomp == NULL) {
        fill_attr = base_attr;
    } else {
        fill_scomp_nr = yed_scomp_nr_by_name(fill_scomp);
        if (fill_scomp_nr >= 0) {
            fill_attr_pre = base_attr;
            fill_attr = yed_get_active_style_scomp(fill_scomp_nr);
            yed_combine_attrs(&fill_attr_pre, &fill_attr);
            fill_attr = fill_attr_pre;
        } else {
            fill_attr = base_attr;
        }
    }

    /* This isn't right.. it should be it's own style component. */
    fill_attr = base_attr;

    yed_set_cursor(frame->left, frame->top + y_offset);

    if (frame->gutter_width > 0) {
        yed_set_attr(gut_attr);
    }

    for (i = 0; i < frame->height - y_offset; i += 1) {
        for (j = 0; j < frame->gutter_width; j += 1) {
            cell = FRAME_CELL(frame, y_offset + i, j);
            if (!*cell) {
                yed_set_cursor(frame->left + j, frame->top + y_offset + i);
                append_n_to_output_buff(" ", 1);
            }
        }
    }

    yed_set_attr(fill_attr);

    for (i = 0; i < frame->height - y_offset; i += 1) {
        cell = FRAME_CELL(frame, y_offset + i, 0);

        yed_set_cursor(frame->left, frame->top + i);

        run_len     = 0;
        run_start_n = frame->gutter_width;
        for (n = frame->gutter_width; n < frame->width; n += 1) {
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

    for (i = 0; i < frame->height - y_offset; i += 1) {
        cell = FRAME_CELL(frame, y_offset + i, frame->gutter_width);
        if (!*cell) {
            yed_set_cursor(frame->left + frame->gutter_width, frame->top + y_offset + i);
            append_n_to_output_buff(fill_str, fill_str_len);
        }
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

    yed_mark_dirty_direct_draws(frame->top,  frame->top +  frame->height - 1,
                                frame->left, frame->left + frame->width  - 1);
}

void yed_frame_set_pos(yed_frame *frame, float top_f, float left_f) {
    frame->top_f  = top_f;
    frame->left_f = left_f;

    FRAME_RESET_RECT(frame);

    yed_frame_reset_cursor(frame);
}

void yed_frame_set_gutter_width(yed_frame *frame, int width) {
    if (width < 0) {
        width = 0;
    }

    frame->gutter_width = width;

    yed_frame_reset_cursor(frame);

    frame->dirty = 1;
}

void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff) {
    yed_event   event;
    yed_buffer *old_buff;

    old_buff = frame->buffer;

    if (old_buff == buff) {
        return;
    }

    memset(&event, 0, sizeof(event));
    event.kind  = EVENT_FRAME_PRE_SET_BUFFER;
    event.frame = frame;
    yed_trigger_event(&event);

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
        yed_set_cursor_within_frame(frame, buff->last_cursor_row, buff->last_cursor_col);
    }

    event.kind = EVENT_FRAME_POST_SET_BUFFER;
    yed_trigger_event(&event);

    if (frame == ys->active_frame) {
        memset(&event, 0, sizeof(event));
        event.kind   = EVENT_BUFFER_FOCUSED;
        event.frame  = frame;
        event.buffer = buff;
        yed_trigger_event(&event);
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
    int        buff_n_lines;
    int        save_cursor_line;
    int        i;
    char      *cell_row;
    yed_event  update_event, buff_draw_event;

    /*
     * Correct the cursor if the buffer has changed.
     */
    if (frame->buffer) {
        buff_n_lines = bucket_array_len(frame->buffer->lines);
        if (frame->cursor_line > buff_n_lines) {
            save_cursor_line = frame->cursor_line;
            yed_set_cursor_far_within_frame(frame, 1, 1);
            yed_set_cursor_far_within_frame(frame, save_cursor_line, 1);
        } else if (frame->buffer->kind == BUFF_KIND_YANK) {
            yed_set_cursor_far_within_frame(frame, 1, 1);
        }
    }


    if (frame->last_cursor_line - frame->cursor_line > 1
    ||  frame->last_cursor_line - frame->cursor_line < -1) {

        frame->dirty = 1;
    }

    if (frame->dirty && frame->buffer) {
        yed_mark_dirty_frames(frame->buffer);
    }

    memset(&update_event, 0, sizeof(update_event));
    memset(&buff_draw_event, 0, sizeof(buff_draw_event));

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
            &&  frame->cursor_line != frame->dirty_line
            &&  frame->cursor_line_is_dirty) {
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

    frame->dirty = frame->dirty_line = frame->cursor_line_is_dirty = 0;
    frame->last_cursor_line = frame->cursor_line;
}

#define NORM_SCROLL_OFF(f) (MIN((f)->scroll_off, (f)->height / 2))

void yed_move_cursor_once_y_within_frame(yed_frame *f, int dir, int buff_n_lines, int buff_big_enough_to_scroll) {
    int new_y,
        bot,
        scroll_off;

    if (dir > 0) {
        dir = 1;
    } else if (dir < 0) {
        dir = -1;
    }

    bot = f->top + f->height;

    if (dir) {
        scroll_off = NORM_SCROLL_OFF(f);
        new_y      = f->cur_y + dir;

        if (buff_big_enough_to_scroll) {
            if (f->buffer_y_offset < buff_n_lines - f->height
            && new_y >= bot - scroll_off) {

                f->buffer_y_offset += dir;
                f->dirty            = 1;
            } else if (f->buffer_y_offset >= 1
                   &&  new_y < f->top + scroll_off) {

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

    width = 1;

    if (dir > 0 && f->cursor_col <= line->visual_width) {
        dir   = 1;
        width = yed_get_glyph_width(*(yed_glyph*)array_item(line->chars, yed_line_col_to_idx(line, f->cursor_col)));
    } else if (dir < 0 && f->cursor_col > 1) {
        dir   = -1;
        width = yed_get_glyph_width(*(yed_glyph*)array_item(line->chars, yed_line_col_to_idx(line, f->cursor_col - 1)));
    }

    old_x_off  = f->buffer_x_offset;
    dir       *= width;

    if (dir) {
        new_x = f->cur_x + dir;

        if (new_x >= f->left + f->width) {
            if (f->buffer_x_offset <= line_width - f->width + f->gutter_width) {
                f->buffer_x_offset += dir;
                f->dirty            = 1;
            }
        } else if (new_x < f->left + f->gutter_width) {
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
    LIMIT(f->cur_x, f->left + f->gutter_width, f->left + f->gutter_width + line_width - f->buffer_x_offset);

    f->cursor_col  = f->buffer_x_offset + (f->cur_x - (f->left + f->gutter_width) + 1);
    f->desired_col = f->cursor_col;
}

void _yed_move_cursor_within_frame(yed_frame *f, int row, int n_glyphs) {
    int       i,
              dir,
              line_width,
              potential_new_x,
              buff_n_lines,
              buff_big_enough_to_scroll,
              save_cursor_line;
    yed_line *line;

    buff_n_lines = bucket_array_len(f->buffer->lines);
    if (buff_n_lines < 1) { return; }

    buff_big_enough_to_scroll = buff_n_lines > 2 * NORM_SCROLL_OFF(f);

    save_cursor_line = f->cursor_line;

    dir = row > 0 ? 1 : -1;
    for (i = 0; i < dir * row; i += 1) {
        yed_move_cursor_once_y_within_frame(f, dir, buff_n_lines, buff_big_enough_to_scroll);
    }

    if (save_cursor_line != f->cursor_line) {
        f->dirty_line           = save_cursor_line;
        f->cursor_line_is_dirty = 1;
    }

    line             = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width       = line->visual_width;

    if (line != NULL && row) {
        /*
         * Update x values tied to y.
         */
        potential_new_x = f->desired_col;

        if (f->desired_col > line_width) {
            potential_new_x = line_width + 1;
        }

        if (potential_new_x <= f->buffer_x_offset) {
            f->buffer_x_offset = MAX(0, potential_new_x - f->width);
            f->desired_col     = potential_new_x;
            f->dirty           = 1;
        }

        if (f->desired_col <= line->visual_width) {
            f->desired_col = yed_line_idx_to_col(line, yed_line_col_to_idx(line, f->desired_col));
        }

        f->cur_x = MIN(f->desired_col - 1, line_width) + f->left + f->gutter_width - f->buffer_x_offset;

        f->cursor_col = f->buffer_x_offset + (f->cur_x - (f->left + f->gutter_width) + 1);
    }

    dir = n_glyphs > 0 ? 1 : -1;
    for (i = 0; i < dir * n_glyphs; i += 1) {
        yed_move_cursor_once_x_within_frame(f, dir, line_width);
        f->cursor_line_is_dirty = 1;
    }

    /*
     * Do some more of this sanity checking in case something wacky
     * happens and yed_move_cursor_once_x_within_frame() never gets
     * called, but the cursor still isn't in the right place.
     */
    LIMIT(f->cur_x, f->left + f->gutter_width, f->left + f->gutter_width + line_width - f->buffer_x_offset);

    f->cursor_col = f->buffer_x_offset + (f->cur_x - (f->left + f->gutter_width) + 1);

    if (row > 1 || row < -1) {
        f->dirty = 1;
    }
}

void yed_move_cursor_within_frame(yed_frame *_f, int row, int n_glyphs) {
    yed_frame  tmp_frame;
    yed_frame *f;
    yed_event  event;

    f = &tmp_frame;
    memcpy(f, _f, sizeof(*f));

    if (f->buffer == NULL) { return; }

    _yed_move_cursor_within_frame(f, row, n_glyphs);

    memset(&event, 0, sizeof(event));
    event.kind    = EVENT_CURSOR_PRE_MOVE;
    event.frame   = _f;
    event.new_row = f->cursor_line;
    event.new_col = f->cursor_col;

    yed_trigger_event(&event);
    if (event.cancel) { return; }

    memcpy(_f, f, sizeof(*_f));

    if (_f->buffer->has_selection && !_f->buffer->selection.locked) {
        _f->buffer->selection.cursor_row = _f->cursor_line;
        _f->buffer->selection.cursor_col = _f->cursor_col;
    }

    event.kind = EVENT_CURSOR_POST_MOVE;
    yed_trigger_event(&event);
}

void _yed_set_cursor_within_frame(yed_frame *f, int new_row, int new_col) {
    int        dir, glyph_dist, row,
               line_width;
    yed_line  *line;
    yed_glyph *g, *new_g;

    if (new_row <= 0) { new_row = 1; }
    if (new_col <= 0) { new_col = 1; }

    if (yed_buff_n_lines(f->buffer) < 1) { return; }

    row = new_row - f->cursor_line;
    _yed_move_cursor_within_frame(f, row, 0);

    line       = yed_buff_get_line(f->buffer, f->cursor_line);
    line_width = line->visual_width;

    if (new_col > line_width + 1) {
        new_col = line_width + 1;
    }

    if (f->cursor_col != new_col) {
        dir        = new_col - f->cursor_col > 0 ? 1 : -1;
        glyph_dist = 0;
        g          = yed_line_col_to_glyph(line, f->cursor_col);
        new_g      = yed_line_col_to_glyph(line, new_col);

        if (dir == 1) {
            while (g != new_g) {
                g           = ((void*)g) + yed_get_glyph_len(*g);
                glyph_dist += 1;
            }
        } else {
            while (new_g != g) {
                new_g       = ((void*)new_g) + yed_get_glyph_len(*new_g);
                glyph_dist += 1;
            }
        }

        glyph_dist *= dir;
        _yed_move_cursor_within_frame(f, 0, glyph_dist);
    }
}

void yed_set_cursor_within_frame(yed_frame *f, int new_row, int new_col) {
    yed_event event;

    if (f->buffer == NULL) { return; }

    memset(&event, 0, sizeof(event));
    event.kind    = EVENT_CURSOR_PRE_MOVE;
    event.frame   = f;
    event.new_row = new_row;
    event.new_col = new_col;
    yed_trigger_event(&event);
    if (event.cancel) { return; }

    _yed_set_cursor_within_frame(f, new_row, new_col);

    if (f->buffer->has_selection && !f->buffer->selection.locked) {
        f->buffer->selection.cursor_row = f->cursor_line;
        f->buffer->selection.cursor_col = f->cursor_col;
    }

    event.kind    = EVENT_CURSOR_POST_MOVE;
    event.new_row = 0;
    event.new_col = 0;
    yed_trigger_event(&event);
}

void yed_set_cursor_far_within_frame(yed_frame *frame, int new_row, int new_col) {
    int buff_n_lines;

    if (frame->buffer) {
        if (new_row <= 0) { new_row = 1; }
        if (new_col <= 0) { new_col = 1; }

        buff_n_lines = bucket_array_len(frame->buffer->lines);

        if ((new_row <  frame->buffer_y_offset + 1)
        ||  (new_row >= frame->buffer_y_offset + frame->height)) {

            frame->buffer_x_offset = 0;
            frame->buffer_y_offset = MIN(new_row, MAX(0, buff_n_lines - frame->height));
            frame->desired_col     = 1;
            frame->cur_x           = frame->left + frame->gutter_width;
            frame->cur_y           = frame->top  + NORM_SCROLL_OFF(frame);
            LIMIT(frame->cur_y,
                    frame->top,
                    MIN(frame->top + frame->height - 1,
                        frame->top + buff_n_lines - frame->buffer_y_offset - 1));
            frame->cursor_col      = 1;
            frame->cursor_line     = frame->buffer_y_offset + (frame->cur_y - frame->top + 1);
        }

        yed_set_cursor_within_frame(frame, new_row, new_col);

        frame->dirty = 1;
    }
}

void yed_frame_reset_cursor(yed_frame *frame) {
    if (frame->cur_y < frame->top
    ||  frame->cur_y >= frame->top + frame->height) {
        frame->cur_y = frame->top;
    }
    if (frame->cur_x < frame->left + frame->gutter_width
    ||  frame->cur_x >= frame->left + frame->width) {
        frame->cur_x = frame->left + frame->gutter_width;
    }

    if (frame->buffer) {
        yed_set_cursor_far_within_frame(frame, frame->cursor_line, frame->cursor_col);
    } else {
        frame->cur_y = frame->top;
        frame->cur_x = frame->left + frame->gutter_width;
    }
}

void yed_frame_hard_reset_cursor_x(yed_frame *frame) {
    int dst_col;

    if (frame->buffer) {
        dst_col = frame->cursor_col;
        frame->buffer_x_offset = 0;
        frame->cur_x = frame->left + frame->gutter_width;
        frame->cursor_col = 1;
        yed_set_cursor_within_frame(frame, frame->cursor_line, dst_col);
    } else {
        frame->cur_y = frame->top;
        frame->cur_x = frame->left + frame->gutter_width;
    }
}

void yed_frame_scroll_buffer(yed_frame *frame, int rows) {
    int buff_n_lines;
    int old_off;

    if (frame         == NULL) { return; }
    if (frame->buffer == NULL) { return; }
    if (rows          == 0)    { return; }

    buff_n_lines = bucket_array_len(frame->buffer->lines);

    old_off = frame->buffer_y_offset;

    if (rows > 0
    &&  frame->cursor_line - frame->buffer_y_offset - 1 <= frame->scroll_off) {
        yed_move_cursor_within_frame(frame, rows, 0);
    } else if (rows < 0
    &&         frame->buffer_y_offset + frame->height - frame->cursor_line <= frame->scroll_off) {
        yed_move_cursor_within_frame(frame, rows, 0);
    }

    frame->buffer_y_offset += rows;
    LIMIT(frame->buffer_y_offset, 0, buff_n_lines - frame->height);

    if (frame->buffer_y_offset != old_off) {
        yed_move_cursor_within_frame(frame, -rows, 0);

        frame->dirty = 1;
    }
}

void yed_move_cursor_within_active_frame(int row, int col) {
    if (ys->active_frame) {
        yed_move_cursor_within_frame(ys->active_frame, row, col);
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

    /*
     * When more than one frame, (say A and B) share a buffer,
     * we update A, mark B's frame as dirty since they share a
     * dirty buffer, and set A's dirty flag to 0.
     * Then, we update B, RESET A's dirty flag to 1, and then set
     * B's to 0.
     * This causes at least one of the frames to always be dirty
     * and force a redraw every pump.
     * So, we just run through all the frames and make sure that
     * they are seen as clean after the update.
     *
     *                                       Brandon Kammerdiener
     *                                       June 30, 2020
     */
    array_traverse(ys->frames, frame) { (*frame)->dirty = 0; }
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

            yed_mark_dirty_direct_draws(y,           y,
                                        frame->left, frame->left + frame->width  - 1);
        }
    }
}

void yed_frame_update_cursor_line(yed_frame *frame) {
    yed_line *line;
    int       y;

    if (!frame->cursor_line || !frame->cursor_line_is_dirty) {
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

        yed_mark_dirty_direct_draws(y,           y,
                                    frame->left, frame->left + frame->width  - 1);
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
            if (dirty_row == (*frame)->cursor_line) {
                (*frame)->cursor_line_is_dirty = 1;
            } else if ((*frame)->dirty_line) {
                if ((*frame)->dirty_line != dirty_row) {
                    (*frame)->dirty = 1;
                }
            } else {
                (*frame)->dirty_line = dirty_row;
            }
        }
    }
}

int yed_cell_is_in_frame(int row, int col, yed_frame *frame) {
    return    (row >= frame->top  && row <= frame->top  + frame->height - 1)
           || (col >= frame->left && col <= frame->left + frame->width  - 1);
}

int yed_frame_is_tree_root(yed_frame *frame) {
    if (frame->tree == NULL) {
        return 0;
    }

    return !frame->tree->parent;
}
