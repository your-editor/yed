#include "internal.h"

yed_line yed_new_line(void) {
    yed_line line;

    memset(&line, 0, sizeof(line));

    line.cells = array_make(yed_cell);

    return line;
}

yed_line yed_new_line_with_cap(int len) {
    yed_line line;

    memset(&line, 0, sizeof(line));

    line.cells = array_make_with_cap(yed_cell, len);

    return line;
}

void yed_free_line(yed_line *line) {
    array_free(line->cells);
}

void yed_line_add_cell(yed_line *line, yed_cell *cell, int idx) {
    array_insert(line->cells, idx, *cell);
    line->visual_width += 1;
}

void yed_line_append_cell(yed_line *line, yed_cell *cell) {
    array_push(line->cells, *cell);
    line->visual_width += 1;
}

void yed_line_delete_cell(yed_line *line, int idx) {
    line->visual_width -= 1;
    array_delete(line->cells, idx);
}

void yed_line_pop_cell(yed_line *line) {
    yed_line_delete_cell(line, array_len(line->cells) - 1);
}

yed_line * yed_buffer_add_line(yed_buffer *buff) {
    yed_line new_line,
             *line;

    new_line = yed_new_line();

    line = bucket_array_push(buff->lines, new_line);

    yed_mark_dirty_frames(buff);

    return line;
}

yed_buffer yed_new_buff(void) {
    yed_buffer  buff;

    buff.lines         = bucket_array_make(1024, yed_line);
    buff.path          = NULL;
    buff.has_selection = 0;
    buff.flags         = 0;

    yed_buffer_add_line(&buff);

    return buff;
}

void yed_append_to_line(yed_line *line, char c) {
    yed_cell cell;

    cell.__data = YED_NEW_CELL__DATA(c);
    yed_line_append_cell(line, &cell);
}

void yed_append_to_new_buff(yed_buffer *buff, char c) {
    yed_line *line;

    if (c == '\r') {
        return;
    }

    if (c == '\n') {
        yed_buffer_add_line(buff);
    } else {
        line = bucket_array_last(buff->lines);

        yed_append_to_line(line, c);
    }
}


int yed_line_col_to_cell_idx(yed_line *line, int col) {
    int       found;
    int       cell_idx;
    yed_cell *cell_it;

    if (col == array_len(line->cells) + 1) {
        return col - 1;
    } else if (col == 1 && array_len(line->cells) == 0) {
        return 0;
    }

    cell_idx = 0;
    found    = 0;

    array_traverse(line->cells, cell_it) {
        if (col - 1 <= 0) {
            found = 1;
            break;
        }
        col      -= 1;
        cell_idx += 1;
    }

    if (!found) {
        ASSERT(0, "didn't compute a good cell idx");
        return -1;
    }

    return cell_idx;
}

yed_cell * yed_line_col_to_cell(yed_line *line, int col) {
    int idx;

    idx = yed_line_col_to_cell_idx(line, col);

    if (idx == -1) {
        return NULL;
    }

    return array_item(line->cells, idx);
}

void yed_line_clear(yed_line *line) {
    array_clear(line->cells);
    line->visual_width = 0;
}

yed_line * yed_buff_get_line(yed_buffer *buff, int row) {
    int idx;

    idx = row - 1;

    if (idx < 0 || idx >= bucket_array_len(buff->lines)) {
        return NULL;
    }

    return bucket_array_item(buff->lines, idx);
}

yed_line * yed_buff_insert_line(yed_buffer *buff, int row) {
    int      idx;
    yed_line new_line, *line;

    idx = row - 1;

    if (idx < 0 || idx > bucket_array_len(buff->lines)) {
        return NULL;
    }

    new_line = yed_new_line();
    line     = bucket_array_insert(buff->lines, idx, new_line);

    yed_mark_dirty_frames(buff);

    return line;
}

void yed_buff_delete_line(yed_buffer *buff, int row) {
    int       idx;
    yed_line *line;

    idx = row - 1;

    LIMIT(idx, 0, bucket_array_len(buff->lines));

    line = yed_buff_get_line(buff, row);
    yed_free_line(line);
    bucket_array_delete(buff->lines, idx);

    yed_mark_dirty_frames(buff);
}

void yed_insert_into_line(yed_buffer *buff, int row, int col, char c) {
    int       idx;
    yed_line *line;
    yed_cell  cell;

    line = yed_buff_get_line(buff, row);

    idx = col - 1;

    LIMIT(idx, 0, line->visual_width);

    cell.__data = YED_NEW_CELL__DATA(c);
    idx         = yed_line_col_to_cell_idx(line, col);
    yed_line_add_cell(line, &cell, idx);

    yed_mark_dirty_frames_line(buff, row);
}

void yed_delete_from_line(yed_buffer *buff, int row, int col) {
    int       idx;
    yed_line *line;

    line = yed_buff_get_line(buff, row);

    idx = col - 1;

    LIMIT(idx, 0, line->visual_width);

    idx = yed_line_col_to_cell_idx(line, col);
    yed_line_delete_cell(line, idx);

    yed_mark_dirty_frames_line(buff, row);
}


void yed_fill_buff_from_file(yed_buffer *buff, const char *path) {
    FILE        *f;
    int          fd, i, j, k, line_len, file_size;
    struct stat  fs;
    char        *file_data, c;
    yed_line    *last_line,
                 line;

    f = fopen(path, "r");
    if (!f) {
        ERR("unable to open file");
    }

    fd = fileno(f);

    if (fstat(fd, &fs) != 0) {
        ERR("unable to stat file");
    }

    file_size = fs.st_size;
    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);

    if (file_data == MAP_FAILED) {
        ERR("mmap failed");
    }

    last_line = bucket_array_last(buff->lines);
    yed_free_line(last_line);
    bucket_array_pop(buff->lines);

    for (i = 0, line_len = 0; i < file_size; i += 1) {
        c = file_data[i];

        if (c == '\n') {
            line = yed_new_line_with_cap(line_len);
            array_grow_if_needed(line.cells);
            line.cells.used = line.visual_width = line_len;

            for (j = 0, k = 0; j < line_len; j += 1) {
                c = file_data[i - line_len + j];

                if (c == '\r')    { continue; }

                (*(yed_cell*)(line.cells.data + (k * sizeof(yed_cell)))).__data = YED_NEW_CELL__DATA(c);
                k += 1;
            }

            bucket_array_push(buff->lines, line);

            line_len = 0;
        } else {
            line_len += 1;
        }
    }

    if (!bucket_array_len(buff->lines) && file_size) {
        /* There's only one line in the file, but it doesn't have a newline. */

        line = yed_new_line_with_cap(line_len);
        array_grow_if_needed(line.cells);
        line.cells.used = line.visual_width = line_len;
        for (j = 0, k = 0; j < line_len; j += 1) {
            c = file_data[i - line_len + j];

            if (c == '\r')    { continue; }

            (*(yed_cell*)(line.cells.data + (k * sizeof(yed_cell)))).__data = YED_NEW_CELL__DATA(c);
            k += 1;
        }

        bucket_array_push(buff->lines, line);
    }

    munmap(file_data, file_size);

    if (bucket_array_len(buff->lines) > 1) {
        last_line = bucket_array_last(buff->lines);
        if (array_len(last_line->cells) == 0) {
            bucket_array_pop(buff->lines);
        }
    }

    buff->path = strdup(path);

    fclose(f);

    yed_mark_dirty_frames(buff);
}

void yed_write_buff_to_file(yed_buffer *buff, const char *path) {
    FILE     *f;
    yed_line *line;
    yed_cell *cell;

    f = fopen(path, "w");
    if (!f) {
        ERR("unable to open file");
        return;
    }

    bucket_array_traverse(buff->lines, line) {
        array_traverse(line->cells, cell) {
            fwrite(&cell->c, 1, 1, f);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

static void yed_range_sorted_points(yed_range *range, int *r1, int *c1, int *r2, int *c2) {
    *r1 = MIN(range->anchor_row, range->cursor_row);
    *r2 = MAX(range->anchor_row, range->cursor_row);

    if (range->kind == RANGE_NORMAL) {
        if (range->anchor_row == range->cursor_row) {
            *c1 = MIN(range->anchor_col, range->cursor_col);
            *c2 = MAX(range->anchor_col, range->cursor_col);
        } else {
            if (*r1 == range->anchor_row) {
                *c1 = range->anchor_col;
                *c2 = range->cursor_col;
            } else {
                *c1 = range->cursor_col;
                *c2 = range->anchor_col;
            }
        }
    }
}

int yed_is_in_range(yed_range *range, int row, int col) {
    int r1, c1, r2, c2;

    yed_range_sorted_points(range, &r1, &c1, &r2, &c2);

    if (row < r1 || row > r2) {
        return 0;
    }

    if (range->kind == RANGE_NORMAL) {
        if (range->anchor_row == range->cursor_row) {
            if (col < c1 || col >= c2) {
                return 0;
            }
        } else {
            if (row < r1 || row > r2) {
                return 0;
            }

            if (row == r1) {
                if (col < c1) {
                    return 0;
                }
            } else if (row == r2) {
                if (col >= c2) {
                    return 0;
                }
            }
        }
    }

    return 1;
}

void yed_buff_delete_selection(yed_buffer *buff) {
    yed_range *range;
    yed_line  *line1,
              *line2;
    yed_cell  *cell;
    int        r1, c1, r2, c2,
               n, i;

    range = &buff->selection;

    yed_range_sorted_points(range, &r1, &c1, &r2, &c2);

    if (range->kind == RANGE_LINE) {
        for (i = r1; i <= r2; i += 1) {
            yed_buff_delete_line(buff, r1);
        }
    } else if (r1 == r2) {
        for (i = c1; i < c2; i += 1) {
            yed_delete_from_line(buff, r1, c1);
        }
    } else {
        line1 = yed_buff_get_line(buff, r1);
        ASSERT(line1, "didn't get line1 in yed_buff_delete_selection()");
        n = line1->visual_width - c1 + 1;
        for (i = 0; i < n; i += 1) {
            yed_line_pop_cell(line1);
        }
        for (i = r1 + 1; i < r2; i += 1) {
            yed_buff_delete_line(buff, r1 + 1);
        }
        line2 = yed_buff_get_line(buff, r1 + 1);
        ASSERT(line2, "didn't get line2 in yed_buff_delete_selection()");
        n = line2->visual_width - c2 + 1;
        for (i = 0; i < n; i += 1) {
            cell = yed_line_col_to_cell(line2, c2 + i);
            yed_line_append_cell(line1, cell);
        }
        for (i = c2 - 1; i < n; i += 1) {
            yed_delete_from_line(buff, r1 + 1, 1);
        }
        yed_buff_delete_line(buff, r1 + 1);
    }

    if (!bucket_array_len(buff->lines)) {
        yed_buffer_add_line(buff);
    }

    buff->has_selection = 0;
}
