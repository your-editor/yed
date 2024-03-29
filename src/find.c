void yed_init_search(void) {
    ys->replace_markers       = array_make(array_t);
    ys->replace_save_lines    = array_make(yed_line*);
    ys->replace_working_lines = array_make(yed_line*);
}

int search_can_move_cursor(void) {
    return yed_var_is_truthy("enable-search-cursor-move");
}

int yed_find_next(int row, int col, int *row_out, int *col_out) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    char       *line_data,
               *line_data_end,
               *scan;
    int         idx,
                r,
                c,
                search_len,
                data_len,
                junk_row, junk_col;

    if (!ys->current_search)    { return 0; }
    if (!ys->active_frame)      { return 0; }

    frame = ys->active_frame;

    if (!frame->buffer)    { return 0; }

    buff = frame->buffer;

    if (buff->has_selection && !search_can_move_cursor()) {
        *row_out = row;
        *col_out = col;
        row_out  = &junk_row;
        col_out  = &junk_col;
    }

    search_len = strlen(ys->current_search);

    if (!search_len)    { return 0; }

    line = yed_buff_get_line(buff, row);
    if (col > line->visual_width) {
        row += 1;
        col = 1;
    }

    r = row;
    bucket_array_traverse_from(buff->lines, line, r - 1) {
        data_len = array_len(line->chars);

        if (!line->visual_width) {
            r += 1;
            continue;
        }

        line_data     = scan = array_data(line->chars);
        line_data_end = line_data + data_len;

        if (r == row) {
            scan += yed_line_col_to_idx(line, col + 1);
        }

        if (yed_var_is_truthy("use-boyer-moore")) {
            while (scan < line_data_end) {
                idx = yed_boyer_moore(scan, line_data_end - scan,
                                ys->current_search, search_len);
                if (idx >= 0) {
                    c = yed_line_idx_to_col(line, scan - line_data + idx);
                    if (r != row || c != col) {
                        *row_out = r;
                        *col_out = c;
                        return 1;
                    }
                    scan += idx + 1;
                } else {
                    break;
                }
            }
        } else {
            while (scan < line_data_end && line_data_end - scan >= search_len) {
                if ((scan = strnstr(scan, ys->current_search, line_data_end - scan))) {
                    idx = scan - line_data;
                    c   = yed_line_idx_to_col(line, idx);

                    if (r != row || c != col) {
                        *row_out = r;
                        *col_out = c;
                        return 1;
                    }

                    scan += 1;
                } else {
                    break;
                }
            }
        }

        r += 1;
    }

    r = 1;
    bucket_array_traverse(buff->lines, line) {
        data_len = array_len(line->chars);

        if (!line->visual_width) {
            r += 1;
            continue;
        }

        line_data     = scan = array_data(line->chars);
        line_data_end = line_data + array_len(line->chars);

        if (r == row) {
            line_data_end = line_data + yed_line_col_to_idx(line, col);
        }

        if (yed_var_is_truthy("use-boyer-moore")) {
            while (scan < line_data_end) {
                idx = yed_boyer_moore(scan, line_data_end - scan,
                                ys->current_search, search_len);
                if (idx >= 0) {
                    c = yed_line_idx_to_col(line, scan - line_data + idx);
                    if (r != row || c != col) {
                        *row_out = r;
                        *col_out = c;
                        return 1;
                    }
                    scan += idx + 1;
                } else {
                    break;
                }
            }
        } else {
            while (scan < line_data_end && line_data_end - scan >= search_len) {
                if ((scan = strnstr(scan, ys->current_search, line_data_end - scan))) {
                    idx = scan - line_data;
                    c   = yed_line_idx_to_col(line, idx);

                    if (r != row || c != col) {
                        *row_out = r;
                        *col_out = c;
                        return 1;
                    }

                    scan += 1;
                } else {
                    break;
                }
            }
        }

        r += 1;
    }

    return 0;
}

int yed_find_prev(int row, int col, int *row_out, int *col_out) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    char       *line_data,
               *line_data_end,
               *scan;
    int         idx,
                r,
                c,
                search_len,
                data_len,
                junk_row, junk_col;

    if (!ys->current_search)    { return 0; }
    if (!ys->active_frame)      { return 0; }

    frame = ys->active_frame;

    if (!frame->buffer)    { return 0; }

    buff = frame->buffer;

    if (buff->has_selection && !search_can_move_cursor()) {
        *row_out = row;
        *col_out = col;
        row_out  = &junk_row;
        col_out  = &junk_col;
    }

    search_len = strlen(ys->current_search);

    if (!search_len)    { return 0; }

    r = row;
    bucket_array_rtraverse_from(buff->lines, line, r - 1) {
        data_len = array_len(line->chars);

        if (!line->visual_width) {
            r -= 1;
            continue;
        }

        line_data     = array_data(line->chars);
        line_data_end = line_data + data_len;

        if (r == row) {
            if (col <= search_len) {
                r -= 1;
                continue;
            }
            line_data_end = line_data + yed_line_col_to_idx(line, col - 1);
        }

        while (line_data < line_data_end && line_data_end - line_data >= search_len) {
            if ((scan = last_strnstr(line_data, ys->current_search, line_data_end - line_data))) {
                idx = scan - line_data;
                c   = yed_line_idx_to_col(line, idx);

                if (r != row || c != col) {
                    *row_out = r;
                    *col_out = c;
                    return 1;
                }

                line_data_end = scan;
            } else {
                break;
            }
        }

        r -= 1;
    }

    r = bucket_array_len(buff->lines);
    bucket_array_rtraverse_from(buff->lines, line, r - 1) {
        data_len = array_len(line->chars);

        if (!line->visual_width) {
            r -= 1;
            continue;
        }

        line_data     = array_data(line->chars);
        line_data_end = line_data + data_len;

        while (line_data < line_data_end && line_data_end - line_data >= search_len) {
            if ((scan = last_strnstr(line_data, ys->current_search, line_data_end - line_data))) {
                idx = scan - line_data;
                c   = yed_line_idx_to_col(line, idx);

                if (r != row || c != col) {
                    *row_out = r;
                    *col_out = c;
                    return 1;
                }

                line_data_end = scan;
            } else {
                break;
            }
        }

        r -= 1;
    }

    return 0;
}
