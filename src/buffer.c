#include "internal.h"

/* color look-up table */
static int CLUT[] = {
    /* 8-bit, RGB hex */

    /* Primary 3-bit (8 colors). Unique representation! */
    0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,

    /* Equivalent "bright" versions of original 8 colors. */
    0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,

    /* Strictly ascending. */
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f,
    0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af,
    0x0087d7, 0x0087ff, 0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f,
    0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af,
    0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f,
    0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af,
    0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff, 0x875f00, 0x875f5f,
    0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af,
    0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f,
    0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af,
    0xaf00d7, 0xaf00ff, 0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f,
    0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700, 0xafd75f, 0xafd787, 0xafd7af,
    0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af,
    0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f,
    0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
    0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f,
    0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af,
    0xffd7d7, 0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,

    /* Gray-scale range. */
    0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
    0x585858, 0x626262, 0x6c6c6c, 0x767676, 0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e,
    0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee,
};

int rgb_to_256(unsigned rgb) {
    int         r,  g,  b;
    int         ri, gi, bi;
    int         smaller,    bigger;
    int         small_dist, big_dist;
    static int  cutoff_norm[] = { 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff };
    static int  cutoff_gs[]   = { 0x08, 0x12, 0x1c, 0x26, 0x30, 0x3a,
                                  0x44, 0x4e, 0x58, 0x62, 0x6c, 0x76,
                                  0x80, 0x8a, 0x94, 0x9e, 0xa8, 0xb2,
                                  0xbc, 0xc6, 0xd0, 0xda, 0xe4 };
    int        *cutoff, cutoff_len;

    r = RGB_32_r(rgb);
    g = RGB_32_g(rgb);
    b = RGB_32_b(rgb);

    if (r == b && b == g) {
        cutoff     = cutoff_gs;
        cutoff_len = sizeof(cutoff_gs) / sizeof(cutoff_gs[0]);
    } else {
        cutoff     = cutoff_norm;
        cutoff_len = sizeof(cutoff_norm) / sizeof(cutoff_norm[0]);
    }

    for (ri = 0; ri < cutoff_len - 1; ri += 1) {
        smaller = cutoff[ri];
        bigger  = cutoff[ri + 1];

        if (smaller <= r && r <= bigger) {
            small_dist = abs(smaller - r);
            big_dist   = abs(bigger  - r);

            if (big_dist <= small_dist) {
                ri = ri + 1;
            }

            break;
        }
    }

    /* Check for greyscale. */
    if (cutoff == cutoff_gs) {
        if (r < 5) { return 16; }
        if (r < cutoff[0]) {
            ri = 0;
        }
        return 232 + ri;
    }

    for (gi = 0; gi < cutoff_len - 1; gi += 1) {
        smaller = cutoff[gi];
        bigger  = cutoff[gi + 1];

        if (smaller <= g && g <= bigger) {
            small_dist = abs(smaller - g);
            big_dist   = abs(bigger  - g);

            if (big_dist <= small_dist) {
                gi = gi + 1;
            }

            break;
        }
    }

    for (bi = 0; bi < cutoff_len - 1; bi += 1) {
        smaller = cutoff[bi];
        bigger  = cutoff[bi + 1];

        if (smaller <= b && b <= bigger) {
            small_dist = abs(smaller - b);
            big_dist   = abs(bigger  - b);

            if (big_dist <= small_dist) {
                bi = bi + 1;
            }

            break;
        }
    }

    return 16 + (36 * ri) + (6 * gi) + (bi);
}

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
        LIMIT(attr.fg, 0, 255);
        LIMIT(attr.bg, 0, 255);

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

yed_line * yed_copy_line(yed_line *line) {
    yed_line *new_line;

    new_line  = malloc(sizeof(*new_line));
    *new_line = yed_new_line();

    new_line->visual_width = line->visual_width;
    array_copy(new_line->chars, line->chars);

    return new_line;
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

yed_buffer yed_new_buff(void) {
    yed_buffer  buff;

    buff.lines              = bucket_array_make(1024, yed_line);
    buff.get_line_cache     = NULL;
    buff.get_line_cache_row = 0;
    buff.path               = NULL;
    buff.has_selection      = 0;
    buff.flags              = 0;
    buff.undo_history       = yed_new_undo_history();

    yed_buffer_add_line_no_undo(&buff);

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

yed_buffer * yed_get_buffer(char *name) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    it = tree_lookup(ys->buffers, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
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

    yed_free_undo_history(&buffer->undo_history);

    free(buffer);
}







void yed_append_to_line_no_undo(yed_buffer *buff, int row, char c) {
    yed_line *line;

    line = yed_buff_get_line(buff, row);
    yed_line_append_char(line, c);
}

void yed_pop_from_line_no_undo(yed_buffer *buff, int row) {
    yed_line *line;

    line = yed_buff_get_line(buff, row);
    yed_line_pop_char(line);
}

void yed_line_clear_no_undo(yed_buffer *buff, int row) {
    yed_line *line;

    line = yed_buff_get_line(buff, row);
    array_clear(line->chars);
    line->visual_width = 0;
}

int yed_buffer_add_line_no_undo(yed_buffer *buff) {
    yed_line new_line;

    new_line = yed_new_line();

    bucket_array_push(buff->lines, new_line);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    return yed_buff_n_lines(buff);
}

void yed_buff_set_line_no_undo(yed_buffer *buff, int row, yed_line *line) {
    yed_line *old_line;

    old_line = yed_buff_get_line(buff, row);

    yed_free_line(old_line);
    old_line->visual_width = line->visual_width;
    old_line->chars        = array_make(char);
    array_copy(old_line->chars, line->chars);
}

yed_line * yed_buff_insert_line_no_undo(yed_buffer *buff, int row) {
    int      idx;
    yed_line new_line, *line;

    idx = row - 1;

    if (idx < 0 || idx > bucket_array_len(buff->lines)) {
        return NULL;
    }

    new_line = yed_new_line();
    line     = bucket_array_insert(buff->lines, idx, new_line);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    return line;
}

void yed_buff_delete_line_no_undo(yed_buffer *buff, int row) {
    int       idx;
    yed_line *line;

    idx = row - 1;

    LIMIT(idx, 0, bucket_array_len(buff->lines));

    line = yed_buff_get_line(buff, row);
    yed_free_line(line);
    bucket_array_delete(buff->lines, idx);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;
}

void yed_insert_into_line_no_undo(yed_buffer *buff, int row, int col, char c) {
    int       idx;
    yed_line *line;

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_add_char(line, c, idx);

    yed_mark_dirty_frames_line(buff, row);
}

void yed_delete_from_line_no_undo(yed_buffer *buff, int row, int col) {
    int       idx;
    yed_line *line;

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_delete_char(line, idx);

    yed_mark_dirty_frames_line(buff, row);
}

void yed_buff_clear_no_undo(yed_buffer *buff) {
    yed_line *line;

    bucket_array_traverse(buff->lines, line) {
        yed_free_line(line);
    }
    bucket_array_clear(buff->lines);

    yed_buffer_add_line_no_undo(buff);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;
}

/*
 * The following functions are the interface by which everything
 * else should modify buffers.
 * This is meant to preserve undo/redo behavior.
 */

void yed_append_to_line(yed_buffer *buff, int row, char c) {
    yed_undo_action uact;

    uact.kind = UNDO_CHAR_PUSH;
    uact.row  = row;
    uact.c    = c;

    yed_push_undo_action(buff, &uact);

    yed_append_to_line_no_undo(buff, row, c);
}

void yed_pop_from_line(yed_buffer *buff, int row) {
    yed_undo_action  uact;
    yed_line        *line;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_CHAR_POP;
    uact.row  = row;
    uact.c    = yed_line_col_to_char(line, line->visual_width);

    yed_push_undo_action(buff, &uact);

    yed_pop_from_line_no_undo(buff, row);
}

void yed_line_clear(yed_buffer *buff, int row) {
    yed_line        *line;
    yed_undo_action  uact;
    int              i;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_CHAR_POP;
    uact.row  = row;
    for (i = line->visual_width; i >= 1; i -= 1) {
        uact.c = yed_line_col_to_char(line, i);
        yed_push_undo_action(buff, &uact);
    }

    yed_line_clear_no_undo(buff, row);
}

int yed_buffer_add_line(yed_buffer *buff) {
    int             row;
    yed_undo_action uact;

    row = yed_buffer_add_line_no_undo(buff);

    uact.kind = UNDO_LINE_ADD;
    uact.row  = row;
    yed_push_undo_action(buff, &uact);

    return row;
}

void yed_buff_set_line(yed_buffer *buff, int row, yed_line *line) {
    yed_line        *old_line;
    yed_undo_action  uact;
    int              i;

    old_line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_CHAR_POP;
    uact.row  = row;
    for (i = old_line->visual_width; i >= 1; i -= 1) {
        uact.c = yed_line_col_to_char(old_line, i);
        yed_push_undo_action(buff, &uact);
    }

    uact.kind = UNDO_CHAR_PUSH;
    uact.row  = row;
    for (i = 1; i <= line->visual_width; i += 1) {
        uact.c = yed_line_col_to_char(line, i);
        yed_push_undo_action(buff, &uact);
    }

    yed_buff_set_line_no_undo(buff, row, line);
}

yed_line * yed_buff_insert_line(yed_buffer *buff, int row) {
    yed_line        *line;
    yed_undo_action  uact;

    line = yed_buff_insert_line_no_undo(buff, row);

    if (line) {
        uact.kind = UNDO_LINE_ADD;
        uact.row  = row;
        yed_push_undo_action(buff, &uact);
    }

    return line;
}

void yed_buff_delete_line(yed_buffer *buff, int row) {
    yed_undo_action  uact;
    yed_line        *line;
    int              i;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_CHAR_POP;
    uact.row  = row;
    for (i = line->visual_width; i >= 1; i -= 1) {
        uact.c = yed_line_col_to_char(line, i);
        yed_push_undo_action(buff, &uact);
    }

    uact.kind = UNDO_LINE_DEL;
    uact.row  = row;
    yed_push_undo_action(buff, &uact);

    yed_buff_delete_line_no_undo(buff, row);
}

void yed_insert_into_line(yed_buffer *buff, int row, int col, char c) {
    yed_undo_action uact;

    uact.kind = UNDO_CHAR_ADD;
    uact.row  = row;
    uact.col  = col;
    uact.c    = c;

    yed_push_undo_action(buff, &uact);

    yed_insert_into_line_no_undo(buff, row, col, c);
}

void yed_delete_from_line(yed_buffer *buff, int row, int col) {
    yed_line        *line;
    yed_undo_action  uact;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_CHAR_DEL;
    uact.row  = row;
    uact.col  = col;
    uact.c    = yed_line_col_to_char(line, col);

    yed_push_undo_action(buff, &uact);

    yed_delete_from_line_no_undo(buff, row, col);
}

void yed_buff_clear(yed_buffer *buff) {
    int              row, col;
    yed_line        *line;
    yed_undo_action  uact;

    for (row = bucket_array_len(buff->lines); row >= 1; row -= 1) {
        line = yed_buff_get_line(buff, row);

        uact.kind = UNDO_CHAR_POP;
        uact.row  = row;
        for (col = line->visual_width; col >= 1; col -= 1) {
            uact.c = yed_line_col_to_char(line, col);
            yed_push_undo_action(buff, &uact);
        }

        uact.kind = UNDO_LINE_DEL;
        uact.row  = row;
        yed_push_undo_action(buff, &uact);
    }

    uact.kind = UNDO_LINE_ADD;
    uact.row  = 1;
    yed_push_undo_action(buff, &uact);

    yed_buff_clear_no_undo(buff);
}








int yed_buff_n_lines(yed_buffer *buff) {
    return bucket_array_len(buff->lines);
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

yed_line * yed_buff_get_line(yed_buffer *buff, int row) {
    int       idx;
    yed_line *line;

    if (row == buff->get_line_cache_row) {
        return buff->get_line_cache;
    }

    idx = row - 1;

    if (idx < 0 || idx >= bucket_array_len(buff->lines)) {
        return NULL;
    }

    line = bucket_array_item(buff->lines, idx);

    buff->get_line_cache     = line;
    buff->get_line_cache_row = row;

    return line;
}



int yed_fill_buff_from_file(yed_buffer *buff, char *path) {
    char *mode;
    FILE *f;
    int   status;

    f = fopen(path, "r");
    if (!f) {
        return 0;
    }

    yed_buff_clear_no_undo(buff);

    if ((mode = yed_get_var("buffer-load-mode"))
    && (strcmp(mode, "map") == 0)) {
        status = yed_fill_buff_from_file_map(buff, f);
    } else {
        status = yed_fill_buff_from_file_stream(buff, f);
    }

    buff->path    = strdup(path);
    buff->file.ft = yed_get_ft(path);
    buff->kind    = BUFF_KIND_FILE;

    fclose(f);

    yed_mark_dirty_frames(buff);

    return status;
}

int yed_fill_buff_from_file_map(yed_buffer *buff, FILE *f) {
    int          fd, i, j, line_len, file_size, did_map, *idx_it;
    struct stat  fs;
    char        *file_data, c;
    yed_line    *last_line,
                 line;
    array_t      return_indices;

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

    return 1;
}

int yed_fill_buff_from_file_stream(yed_buffer *buff, FILE *f) {
    int          i, *idx_it;
    ssize_t      line_len;
    size_t       line_cap;
    char         c, *line_data;
    yed_line    *last_line,
                 line;
    array_t      return_indices;

    return_indices = array_make(int);

    last_line = bucket_array_last(buff->lines);
    yed_free_line(last_line);
    bucket_array_pop(buff->lines);

    while (line_data = NULL, (line_len = getline(&line_data, &line_cap, f)) > 0) {
        line.chars.data      = line_data;
        line.chars.elem_size = 1;
        line.chars.used      = line_len;
        line.chars.capacity  = line_cap;
        line.visual_width    = line_len;

        while (array_len(line.chars)
        &&    ((c = *(char*)array_last(line.chars)) == '\n' || c == '\r')) {
            array_pop(line.chars);
            line.visual_width -= 1;
        }

        for (i = 0; i < array_len(line.chars); i += 1) {
            c = line_data[i];
            if (c == '\r')    { array_push(return_indices, i); }
        }

        /* Remove '\r' from line. */
        array_traverse(return_indices, idx_it) {
            array_delete(line.chars, *idx_it);
            line.visual_width -= 1;
        }
        array_clear(return_indices);

        bucket_array_push(buff->lines, line);
    }

    array_free(return_indices);

    if (bucket_array_len(buff->lines) > 1) {
        last_line = bucket_array_last(buff->lines);
        if (array_len(last_line->chars) == 0) {
            bucket_array_pop(buff->lines);
        }
    } else if (bucket_array_len(buff->lines) == 0) {
        line = yed_new_line();
        bucket_array_push(buff->lines, line);
    }

    return 1;
}

void yed_write_buff_to_file(yed_buffer *buff, char *path) {
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

    if (!buff->path) {
        buff->path    = strdup(path);
        buff->file.ft = yed_get_ft(path);
    }
    buff->kind = BUFF_KIND_FILE;

    fclose(f);
}

void yed_range_sorted_points(yed_range *range, int *r1, int *c1, int *r2, int *c2) {
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
            yed_pop_from_line(buff, r1);
        }
        for (i = r1 + 1; i < r2; i += 1) {
            yed_buff_delete_line(buff, r1 + 1);
        }
        line2 = yed_buff_get_line(buff, r1 + 1);
        ASSERT(line2, "didn't get line2 in yed_buff_delete_selection()");
        n = line2->visual_width - c2 + 1;
        for (i = 0; i < n; i += 1) {
            c = yed_line_col_to_char(line2, c2 + i);
            yed_append_to_line(buff, r1, c);
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
