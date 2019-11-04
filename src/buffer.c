#include "internal.h"

void yed_init_buffers(void) {
    yed_buffer *yank_buff;

    ys->buffers = tree_make_c(yed_buffer_name_t, yed_buffer_ptr_t, strcmp);

    yank_buff         = yed_create_buffer("*yank");
    yank_buff->kind   = BUFF_KIND_YANK;
    yank_buff->flags |= BUFF_RD_ONLY;

    ys->yank_buff = yank_buff;
}

void yed_set_attr(yed_attrs attr) {

#define BUFFCATN(buff_p, str, n) \
do { memcpy((buff_p), (str), (n)); (buff_p) += (n); } while (0)

#define BUFFCAT(buff_p, str)                \
do {                                        \
    int __BUFFCAT_len;                      \
    __BUFFCAT_len = strlen(str);            \
    memcpy((buff_p), (str), __BUFFCAT_len); \
    (buff_p) += __BUFFCAT_len;              \
} while (0)

    char buff[128], *buff_p;
    int fr, fg, fb;
    int br, bg, bb;

    buff[0] = 0;
    buff_p  = buff;

    BUFFCATN(buff_p, "\e[0", 3);

    if (attr.flags & ATTR_BOLD) {
        BUFFCATN(buff_p, ";1", 2);
    }

    if (attr.flags & ATTR_INVERSE) {
        BUFFCATN(buff_p, ";7", 2);
    }

    if (attr.flags & ATTR_16) {
        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        if (attr.fg && attr.bg) {
            BUFFCAT(buff_p, u8_to_s[10 + attr.bg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[attr.fg]);
        } else if (attr.fg) {
            BUFFCAT(buff_p, u8_to_s[attr.fg]);
        } else if (attr.bg) {
            BUFFCAT(buff_p, u8_to_s[10 + attr.bg]);
        }
    } else if (attr.flags & ATTR_256) {
        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        if (attr.fg && attr.bg) {
            BUFFCATN(buff_p, "38;5;", 5);
            BUFFCAT(buff_p, u8_to_s[attr.fg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCATN(buff_p, "48;5;", 5);
            BUFFCAT(buff_p, u8_to_s[attr.bg]);
        } else if (attr.fg) {
            BUFFCATN(buff_p, "38;5;", 5);
            BUFFCAT(buff_p, u8_to_s[attr.fg]);
        } else if (attr.bg) {
            BUFFCATN(buff_p, "48;5;", 5);
            BUFFCAT(buff_p, u8_to_s[attr.bg]);
        }
    } else if (attr.flags & ATTR_RGB) {
        if (attr.fg || attr.bg) {
            BUFFCATN(buff_p, ";", 1);
        }

        fr = RGB_32_r(attr.fg);
        fg = RGB_32_g(attr.fg);
        fb = RGB_32_b(attr.fg);
        br = RGB_32_r(attr.bg);
        bg = RGB_32_g(attr.bg);
        bb = RGB_32_b(attr.bg);

        if (attr.fg && attr.bg) {
            BUFFCATN(buff_p, "38;2;", 5);
            BUFFCAT(buff_p, u8_to_s[fr]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[fg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[fb]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCATN(buff_p, "48;2;", 5);
            BUFFCAT(buff_p, u8_to_s[br]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[bg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[bb]);
        } else if (attr.fg) {
            BUFFCATN(buff_p, "38;2;", 5);
            BUFFCAT(buff_p, u8_to_s[fr]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[fg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[fb]);
        } else if (attr.bg) {
            BUFFCATN(buff_p, "48;2;", 5);
            BUFFCAT(buff_p, u8_to_s[br]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[bg]);
            BUFFCATN(buff_p, ";", 1);
            BUFFCAT(buff_p, u8_to_s[bb]);
        }
    }

    BUFFCATN(buff_p, "m", 1);
    *buff_p = 0;

    append_to_output_buff(buff);
}

int yed_attrs_eq(yed_attrs attr1, yed_attrs attr2) {
    return    (attr1.fg    == attr2.fg)
           && (attr1.bg    == attr2.bg)
           && (attr1.flags == attr2.flags);
}

yed_line yed_new_line(void) {
    yed_line line;

    memset(&line, 0, sizeof(line));

    line.chars = array_make(char);

    return line;
}

yed_line yed_new_line_with_cap(int len) {
    yed_line line;

    memset(&line, 0, sizeof(line));

    line.chars = array_make_with_cap(char, len);

    return line;
}

void yed_free_line(yed_line *line) {
    array_free(line->chars);
}

void yed_line_add_char(yed_line *line, char c, int idx) {
    array_insert(line->chars, idx, c); line->visual_width += 1;
}

void yed_line_append_char(yed_line *line, char c) {
    array_push(line->chars, c);
    line->visual_width += 1;
}

void yed_line_delete_char(yed_line *line, int idx) {
    line->visual_width -= 1;
    array_delete(line->chars, idx);
}

void yed_line_pop_char(yed_line *line) {
    yed_line_delete_char(line, array_len(line->chars) - 1);
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

yed_buffer *yed_create_buffer(char *name) {
    yed_buffer                                   *buff;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    it = tree_lookup(ys->buffers, name);

    if (tree_it_good(it)) {
        return NULL;
    }

    buff = malloc(sizeof(*buff));

    *buff = yed_new_buff();
    buff->name = strdup(name);

    tree_insert(ys->buffers, strdup(name), buff);

    return buff;
}

void yed_free_buffer(yed_buffer *buffer) {
    yed_line *line;

    yed_frames_remove_buffer(buffer);

    if (buffer->name) {
        tree_delete(ys->buffers, buffer->name);
        free(buffer->name);
    }

    if (buffer->path) {
        free(buffer->path);
    }

    bucket_array_traverse(buffer->lines, line) {
        yed_free_line(line);
    }

    bucket_array_free(buffer->lines);

    free(buffer);
}

void yed_append_to_line(yed_line *line, char c) {
    yed_line_append_char(line, c);
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

int yed_line_col_to_idx(yed_line *line, int col) {
    if (col > array_len(line->chars) + 1) {
        return -1;
    }
    return col - 1;
#if 0
    int       found;
    int       idx;
    char     *c_it;

    if (col == array_len(line->chars) + 1) {
        return col - 1;
    } else if (col == 1 && array_len(line->chars) == 0) {
        return 0;
    }

    idx   = 0;
    found = 0;

    array_traverse(line->chars, c_it) {
        if (col - 1 <= 0) {
            found = 1;
            break;
        }
        col -= 1;
        idx += 1;
    }

    if (!found) {
        ASSERT(0, "didn't compute a good idx");
        return -1;
    }

    return idx;
#endif
}

char yed_line_col_to_char(yed_line *line, int col) {
    int idx;

    idx = yed_line_col_to_idx(line, col);

    if (idx == -1) {
        return 0;
    }

    return *(char*)array_item(line->chars, idx);
}

void yed_line_clear(yed_line *line) {
    array_clear(line->chars);
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

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_add_char(line, c, idx);

    yed_mark_dirty_frames_line(buff, row);
}

void yed_delete_from_line(yed_buffer *buff, int row, int col) {
    int       idx;
    yed_line *line;

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_delete_char(line, idx);

    yed_mark_dirty_frames_line(buff, row);
}


int yed_fill_buff_from_file(yed_buffer *buff, const char *path) {
    FILE        *f;
    int          fd, i, j, line_len, file_size, did_map, *idx_it;
    struct stat  fs;
    char        *file_data, c;
    yed_line    *last_line,
                 line;
    array_t      return_indices;

    f = fopen(path, "r");
    if (!f) {
        return 0;
    }

    fd = fileno(f);

    if (fstat(fd, &fs) != 0) {
        ERR("unable to stat file");
    }

    file_size = fs.st_size;

    if (file_size != 0) {
        file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);

        if (file_data == MAP_FAILED) {
            ERR("mmap failed");
        }

        did_map = 1;
    } else {
        did_map = 0;
    }

    return_indices = array_make(int);

    last_line = bucket_array_last(buff->lines);
    yed_free_line(last_line);
    bucket_array_pop(buff->lines);

    for (i = 0, line_len = 0; i < file_size; i += 1) {
        c = file_data[i];

        if (c == '\n') {
            line = yed_new_line_with_cap(line_len);

            for (j = 0; j < line_len; j += 1) {
                c = file_data[i - line_len + j];
                if (c == '\r')    { array_push(return_indices, j); }
            }

            array_push_n(line.chars, file_data + i - line_len, line_len);

            /* Remove '\r' from line. */
            array_traverse(return_indices, idx_it) {
                array_delete(line.chars, *idx_it);
                line_len -= 1;
            }
            array_clear(return_indices);

            line.visual_width = line_len;

            bucket_array_push(buff->lines, line);

            line_len = 0;
        } else {
            line_len += 1;
        }
    }

    if (!bucket_array_len(buff->lines)) {
        if (file_size) {
            /* There's only one line in the file, but it doesn't have a newline. */

            line = yed_new_line_with_cap(line_len);

            for (j = 0; j < line_len; j += 1) {
                c = file_data[i - line_len + j];
                if (c == '\r')    { array_push(return_indices, j); }
            }

            array_push_n(line.chars, file_data + i - line_len, line_len);

            /* Remove '\r' from line. */
            array_traverse(return_indices, idx_it) {
                array_delete(line.chars, *idx_it);
                line_len -= 1;
            }
            array_clear(return_indices);

            line.visual_width = line_len;

            bucket_array_push(buff->lines, line);
        } else {
            line = yed_new_line();
            bucket_array_push(buff->lines, line);
        }
    }

    array_free(return_indices);

    if (did_map) {
        munmap(file_data, file_size);
    }

    if (bucket_array_len(buff->lines) > 1) {
        last_line = bucket_array_last(buff->lines);
        if (array_len(last_line->chars) == 0) {
            bucket_array_pop(buff->lines);
        }
    }

    buff->path = strdup(path);
    buff->kind = BUFF_KIND_FILE;

    fclose(f);

    yed_mark_dirty_frames(buff);

    return 1;
}

void yed_write_buff_to_file(yed_buffer *buff, const char *path) {
    FILE      *f;
    yed_line  *line;
    yed_event  event;

    event.kind   = EVENT_BUFFER_PRE_WRITE;
    event.buffer = buff;
    yed_trigger_event(&event);

    f = fopen(path, "w");
    if (!f) {
        ERR("unable to open file");
        return;
    }

    /*
     * @refactor?
     * Should we be doing something smarter with this buffer size?
     */
    setvbuf(f, NULL, _IOFBF, 64 * 1024);

    bucket_array_traverse(buff->lines, line) {
        fwrite(line->chars.data, 1, array_len(line->chars), f);
        fprintf(f, "\n");
    }

    buff->kind = BUFF_KIND_FILE;

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
    char       c;
    int        r1, c1, r2, c2,
               n, i;

    r1 = c1 = r2 = c2 = 0;

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
            yed_line_pop_char(line1);
        }
        for (i = r1 + 1; i < r2; i += 1) {
            yed_buff_delete_line(buff, r1 + 1);
        }
        line2 = yed_buff_get_line(buff, r1 + 1);
        ASSERT(line2, "didn't get line2 in yed_buff_delete_selection()");
        n = line2->visual_width - c2 + 1;
        for (i = 0; i < n; i += 1) {
            c = yed_line_col_to_char(line2, c2 + i);
            yed_line_append_char(line1, c);
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
