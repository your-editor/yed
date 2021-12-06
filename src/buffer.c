void yed_init_buffers(void) {
    LOG_FN_ENTER();

    ys->buffers = tree_make(yed_buffer_name_t, yed_buffer_ptr_t);

    yed_get_yank_buffer();
    yed_get_log_buffer();
    yed_get_bindings_buffer();
    yed_get_vars_buffer();
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
    new_line->n_glyphs     = line->n_glyphs;
    array_copy(new_line->chars, line->chars);

    return new_line;
}

void yed_line_add_glyph(yed_line *line, yed_glyph g, int idx) {
    int len, i;

    len = yed_get_glyph_len(g);
    for (i = len - 1; i >= 0; i -= 1) {
        array_insert(line->chars, idx, g.bytes[i]);
    }
    line->visual_width += yed_get_glyph_width(g);
    line->n_glyphs     += 1;
}

void yed_line_append_glyph(yed_line *line, yed_glyph g) {
    int len, width, i;

    len   = yed_get_glyph_len(g);
    width = yed_get_glyph_width(g);
    for (i = 0; i < len; i += 1) {
        array_push(line->chars, g.bytes[i]);
    }
    line->visual_width += width;
    line->n_glyphs     += 1;
}

void yed_line_delete_glyph(yed_line *line, int idx) {
    yed_glyph *g;
    int        len, width, i;

    g     = array_item(line->chars, idx);
    len   = yed_get_glyph_len(*g);
    width = yed_get_glyph_width(*g);

    for (i = 0; i < len; i += 1) {
        array_delete(line->chars, idx);
    }

    line->visual_width -= width;
    line->n_glyphs     -= 1;
}

void yed_line_pop_glyph(yed_line *line) {
    yed_glyph *g;
    int        idx, len, width, i;

    idx   = yed_line_col_to_idx(line, line->visual_width);
    g     = array_item(line->chars, idx);
    len   = yed_get_glyph_len(*g);
    width = yed_get_glyph_width(*g);

    for (i = 0; i < len; i += 1) {
        array_pop(line->chars);
    }

    line->visual_width -= width;
    line->n_glyphs     -= 1;
}

static int yed_buffer_add_line_no_undo_no_events(yed_buffer *buff) {
    u32      n_lines;
    yed_line new_line;

    n_lines  = yed_buff_n_lines(buff);
    new_line = yed_new_line();

    bucket_array_push(buff->lines, new_line);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    return n_lines + 1;
}

yed_buffer yed_new_buff(void) {
    yed_buffer  buff;

    buff.kind                 = BUFF_KIND_UNKNOWN;
    buff.lines                = bucket_array_make(1024, yed_line);
    buff.get_line_cache       = NULL;
    buff.get_line_cache_row   = 0;
    buff.path                 = NULL;
    buff.mmap_underlying_buff = NULL;
    buff.has_selection        = 0;
    buff.flags                = 0;
    buff.undo_history         = yed_new_undo_history();
    buff.last_cursor_row      = 1;
    buff.last_cursor_col      = 1;
    buff.ft                   = FT_UNKNOWN;

    yed_buffer_add_line_no_undo_no_events(&buff);

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

yed_buffer * yed_get_buffer_by_path(char *path) {
    char                                          a_path[4096];
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    yed_buffer                                   *buff;

    if (abs_path(path, a_path)) { path = a_path; }

    tree_traverse(ys->buffers, it) {
        buff = tree_it_val(it);
        if (buff->path && strcmp(buff->path, path) == 0) {
            return buff;
        }
    }

    return NULL;
}

yed_buffer * yed_get_or_create_special_rdonly_buffer(char *name) {
    yed_buffer *buff;

    buff = yed_get_buffer(name);
    if (buff == NULL) {
        buff = yed_create_buffer(name);
        buff->flags |= BUFF_SPECIAL | BUFF_RD_ONLY;
    }

    return buff;
}

void yed_free_buffer(yed_buffer *buffer) {
    yed_event  event;
    yed_line  *line;

    memset(&event, 0, sizeof(event));
    event.kind   = EVENT_BUFFER_PRE_DELETE;
    event.buffer = buffer;
    yed_trigger_event(&event);

    yed_frames_remove_buffer(buffer);

    if (buffer->name) {
        tree_delete(ys->buffers, buffer->name);
        free(buffer->name);
    }

    if (buffer->path) {
        free(buffer->path);
    }

    if (buffer->mmap_underlying_buff) {
        free(buffer->mmap_underlying_buff);
    }

    bucket_array_traverse(buffer->lines, line) {
        yed_free_line(line);
    }

    bucket_array_free(buffer->lines);

    yed_free_undo_history(&buffer->undo_history);

    free(buffer);
}



yed_buffer *yed_get_yank_buffer(void) {
    yed_buffer *b;
    b = yed_get_or_create_special_rdonly_buffer("*yank");
    b->kind = BUFF_KIND_YANK;
    return b;
}

yed_buffer *yed_get_log_buffer(void) {
    yed_buffer *b;
    b = yed_get_or_create_special_rdonly_buffer("*log");
    b->kind = BUFF_KIND_LOG;
    return b;
}

yed_buffer *yed_get_bindings_buffer(void) {
    yed_buffer *b;
    b = yed_get_or_create_special_rdonly_buffer("*bindings");
    return b;
}

void yed_update_bindings_buffer(void) {
    yed_buffer                          *bind_buff;
    int                                  row;
    int                                  key;
    yed_key_binding                     *binding;
    char                                *key_str;
    int                                  i;
    char                                 line_buff[1024];
    tree_it(int, yed_key_binding_ptr_t)  it;

    bind_buff = yed_get_bindings_buffer();

    bind_buff->flags &= ~BUFF_RD_ONLY;

    yed_buff_clear_no_undo(bind_buff);

    row = 1;
    for (key = 0; key < REAL_KEY_MAX; key += 1) {
        binding = ys->real_key_map[key];
        if (binding == NULL) { continue; }

        key_str = yed_keys_to_string(1, &key);
        if (key_str == NULL) { continue; }

        snprintf(line_buff, sizeof(line_buff), "%-32s %s", key_str, binding->cmd);
        free(key_str);

        for (i = 0; i < binding->n_args; i += 1) {
            strcat(line_buff, " \"");
            strcat(line_buff, binding->args[i]);
            strcat(line_buff, "\"");
        }

        yed_buff_insert_string_no_undo(bind_buff, line_buff, row, 1);

        row += 1;
    }

    tree_traverse(ys->vkey_binding_map, it) {
        binding = tree_it_val(it);

        key_str = yed_keys_to_string(1, &binding->key);
        if (key_str == NULL) { continue; }

        snprintf(line_buff, sizeof(line_buff), "%-32s %s", key_str, binding->cmd);
        free(key_str);

        for (i = 0; i < binding->n_args; i += 1) {
            strcat(line_buff, " '");
            strcat(line_buff, binding->args[i]);
            strcat(line_buff, "'");
        }

        yed_buff_insert_string_no_undo(bind_buff, line_buff, row, 1);

        row += 1;
    }

    bind_buff->flags |= BUFF_RD_ONLY;
}

yed_buffer *yed_get_vars_buffer(void) {
    yed_buffer *b;
    b = yed_get_or_create_special_rdonly_buffer("*vars");
    return b;
}

void yed_update_vars_buffer(void) {
    yed_buffer                             *vars_buff;
    int                                     max_width;
    tree_it(yed_var_name_t, yed_var_val_t)  vit;
    char                                    line_buff[1024];
    int                                     row;

    vars_buff = yed_get_vars_buffer();

    vars_buff->flags &= ~BUFF_RD_ONLY;

    yed_buff_clear_no_undo(vars_buff);

    max_width = 0;
    tree_traverse(ys->vars, vit) {
        max_width = MAX(max_width, strlen(tree_it_key(vit)));
    }

    row = 1;
    tree_traverse(ys->vars, vit) {
        snprintf(line_buff, sizeof(line_buff), "%-*s = %s", max_width, tree_it_key(vit), tree_it_val(vit));
        yed_buff_insert_string_no_undo(vars_buff, line_buff, row, 1);
        row += 1;
    }

    vars_buff->flags |= BUFF_RD_ONLY;
}


void yed_buffer_set_ft(yed_buffer *buffer, int ft) {
    yed_event event;

    if (buffer->kind != BUFF_KIND_FILE
    ||  buffer->flags & BUFF_SPECIAL) {
        return;
    }

    memset(&event, 0, sizeof(event));

    event.kind   = EVENT_BUFFER_PRE_SET_FT;
    event.buffer = buffer;
    yed_trigger_event(&event);
    buffer->ft = ft;
    event.kind = EVENT_BUFFER_POST_SET_FT;
    yed_trigger_event(&event);
}




void yed_buff_insert_string_no_undo(yed_buffer *buff, const char *str, int row, int col) {
    yed_glyph *g;
    yed_line  *line;
    yed_glyph *line_last_glyph;

    if (strlen(str) == 0) { return; }

    if (row <= 0) { row = 1; }
    if (col <= 0) { col = 1; }

    while (yed_buff_n_lines(buff) < row) {
        yed_buffer_add_line_no_undo(buff);
    }

    line = yed_buff_get_line(buff, row);
    while (line->visual_width < col - 1) {
        yed_append_to_line_no_undo(buff, row, G(' '));
    }

    while (*str) {
        g = (yed_glyph*)(void*)str;

        if (g->c == '\n') {
            line = yed_buff_get_line(buff, row);
            yed_buff_insert_line_no_undo(buff, row + 1);
            while (line->visual_width > col - 1) {
                line_last_glyph = yed_line_last_glyph(line);
                yed_insert_into_line_no_undo(buff, row + 1, 1, *line_last_glyph);
                yed_pop_from_line_no_undo(buff, row);
            }
            row += 1;
            col  = 1;
        } else if (!G_IS_ASCII(*g) || isprint(g->c)) {
            yed_insert_into_line_no_undo(buff, row, col, *g);
            col += yed_get_glyph_width(*g);
        }

        str += yed_get_glyph_len(*g);
    }
}

#define DO_RD_ONLY_CHECK(_buff)                      \
do {                                                 \
    if ((_buff)->flags & BUFF_RD_ONLY) { goto out; } \
} while (0)

#define DO_PRE_MOD_EVT(_buff, _kind, _row, _col)     \
do {                                                 \
    yed_event _event;                                \
    memset(&_event, 0, sizeof(_event));              \
    _event.kind           = EVENT_BUFFER_PRE_MOD;    \
    _event.buffer         = (_buff);                 \
    _event.buff_mod_event = (_kind);                 \
    _event.row            = (_row);                  \
    _event.col            = (_col);                  \
    yed_trigger_event(&_event);                      \
    if (_event.cancel) { goto out; }                 \
} while (0)
#define DO_POST_MOD_EVT(_buff, _kind, _row, _col)    \
do {                                                 \
    yed_event _event;                                \
    memset(&_event, 0, sizeof(_event));              \
    _event.kind           = EVENT_BUFFER_POST_MOD;   \
    _event.buffer         = (_buff);                 \
    _event.buff_mod_event = (_kind);                 \
    _event.row            = (_row);                  \
    _event.col            = (_col);                  \
    yed_trigger_event(&_event);                      \
    (_buff)->flags |= BUFF_MODIFIED;                 \
} while (0)

void yed_append_to_line_no_undo(yed_buffer *buff, int row, yed_glyph g) {
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_APPEND_TO_LINE, row, 0);

    line = yed_buff_get_line(buff, row);
    yed_line_append_glyph(line, g);

    DO_POST_MOD_EVT(buff, BUFF_MOD_APPEND_TO_LINE, row, 0);
out:;
}

void yed_pop_from_line_no_undo(yed_buffer *buff, int row) {
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_POP_FROM_LINE, row, 0);

    line = yed_buff_get_line(buff, row);
    yed_line_pop_glyph(line);

    DO_POST_MOD_EVT(buff, BUFF_MOD_POP_FROM_LINE, row, 0);
out:;
}

void yed_line_clear_no_undo(yed_buffer *buff, int row) {
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_CLEAR, row, 0);

    line = yed_buff_get_line(buff, row);
    array_clear(line->chars);
    line->visual_width = 0;

    DO_POST_MOD_EVT(buff, BUFF_MOD_CLEAR, row, 0);
out:;
}

int yed_buffer_add_line_no_undo(yed_buffer *buff) {
    u32      n_lines;
    yed_line new_line;

    n_lines = yed_buff_n_lines(buff);

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_ADD_LINE, n_lines + 1, 0);

    new_line = yed_new_line();

    bucket_array_push(buff->lines, new_line);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    DO_POST_MOD_EVT(buff, BUFF_MOD_ADD_LINE, n_lines + 1, 0);

out:;
    return n_lines + 1;
}

void yed_buff_set_line_no_undo(yed_buffer *buff, int row, yed_line *line) {
    yed_line *old_line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_SET_LINE, row, 0);

    old_line = yed_buff_get_line(buff, row);

    yed_free_line(old_line);
    old_line->visual_width = line->visual_width;
    old_line->chars        = array_make(char);
    array_copy(old_line->chars, line->chars);

    DO_POST_MOD_EVT(buff, BUFF_MOD_SET_LINE, row, 0);
out:;
}

yed_line * yed_buff_insert_line_no_undo(yed_buffer *buff, int row) {
    int      idx;
    yed_line new_line, *line;

    idx = row - 1;

    if (idx < 0 || idx > bucket_array_len(buff->lines)) {
        return NULL;
    }

    line = NULL;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_INSERT_LINE, row, 0);

    new_line = yed_new_line();
    line     = bucket_array_insert(buff->lines, idx, new_line);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    DO_POST_MOD_EVT(buff, BUFF_MOD_INSERT_LINE, row, 0);

out:;
    return line;
}

void yed_buff_delete_line_no_undo(yed_buffer *buff, int row) {
    int       idx;
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_DELETE_LINE, row, 0);

    idx = row - 1;

    LIMIT(idx, 0, bucket_array_len(buff->lines));

    line = yed_buff_get_line(buff, row);
    yed_free_line(line);
    bucket_array_delete(buff->lines, idx);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

    DO_POST_MOD_EVT(buff, BUFF_MOD_DELETE_LINE, row, 0);

out:;
}

void yed_insert_into_line_no_undo(yed_buffer *buff, int row, int col, yed_glyph g) {
    int       idx;
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_INSERT_INTO_LINE, row, col);

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_add_glyph(line, g, idx);

    yed_mark_dirty_frames_line(buff, row);

    DO_POST_MOD_EVT(buff, BUFF_MOD_INSERT_INTO_LINE, row, col);

out:;
}

void yed_delete_from_line_no_undo(yed_buffer *buff, int row, int col) {
    int       idx;
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_DELETE_FROM_LINE, row, col);

    line = yed_buff_get_line(buff, row);

    idx = yed_line_col_to_idx(line, col);
    yed_line_delete_glyph(line, idx);

    yed_mark_dirty_frames_line(buff, row);

    DO_POST_MOD_EVT(buff, BUFF_MOD_DELETE_FROM_LINE, row, col);

out:;
}

void yed_buff_clear_no_undo(yed_buffer *buff) {
    yed_line *line;

    DO_RD_ONLY_CHECK(buff);

    DO_PRE_MOD_EVT(buff, BUFF_MOD_CLEAR, 0, 0);

    bucket_array_traverse(buff->lines, line) {
        yed_free_line(line);
    }
    bucket_array_clear(buff->lines);

    DO_POST_MOD_EVT(buff, BUFF_MOD_CLEAR, 0, 0);

    yed_buffer_add_line_no_undo(buff);

    yed_mark_dirty_frames(buff);
    buff->get_line_cache     = NULL;
    buff->get_line_cache_row = 0;

out:;
}

/*
 * The following functions are the interface by which everything
 * else should modify buffers.
 * This is meant to preserve undo/redo behavior.
 */

void yed_buff_insert_string(yed_buffer *buff, const char *str, int row, int col) {
    yed_frame  *frame;
    yed_frame **fit;
    int         num_orig_undo_records;
    yed_glyph  *g;
    yed_line   *line;
    yed_glyph  *line_last_glyph;

    if (strlen(str) == 0) { return; }

    if (row <= 0) { row = 1; }
    if (col <= 0) { col = 1; }

    /* Can we find a good frame? */
    frame = NULL;
    if (ys->active_frame && ys->active_frame->buffer == buff) {
        frame = ys->active_frame;
    } else {
        array_traverse(ys->frames, fit) {
            if ((*fit)->buffer == buff) {
                frame = *fit;
                break;
            }
        }
    }

    yed_start_undo_record(frame, buff);

    num_orig_undo_records = yed_get_undo_num_records(buff);

    while (yed_buff_n_lines(buff) < row) {
        yed_buffer_add_line(buff);
    }

    line = yed_buff_get_line(buff, row);
    while (line->visual_width < col - 1) {
        yed_append_to_line(buff, row, G(' '));
    }

    while (*str) {
        g = (yed_glyph*)(void*)str;

        if (g->c == '\n') {
            line = yed_buff_get_line(buff, row);
            yed_buff_insert_line(buff, row + 1);
            while (line->visual_width > col - 1) {
                line_last_glyph = yed_line_last_glyph(line);
                yed_insert_into_line(buff, row + 1, 1, *line_last_glyph);
                yed_pop_from_line(buff, row);
            }
            row += 1;
            col  = 1;
        } else if (!G_IS_ASCII(*g) || isprint(g->c)) {
            yed_insert_into_line(buff, row, col, *g);
            col += yed_get_glyph_width(*g);
        }

        str += yed_get_glyph_len(*g);
    }

    yed_end_undo_record(frame, buff);

    while (yed_get_undo_num_records(buff) > num_orig_undo_records) {
        yed_merge_undo_records(buff);
    }
}

void yed_append_to_line(yed_buffer *buff, int row, yed_glyph g) {
    yed_undo_action uact;

    uact.kind = UNDO_GLYPH_PUSH;
    uact.row  = row;
    uact.g    = g;

    yed_push_undo_action(buff, &uact);

    yed_append_to_line_no_undo(buff, row, g);
}

void yed_pop_from_line(yed_buffer *buff, int row) {
    yed_undo_action  uact;
    yed_line        *line;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_GLYPH_POP;
    uact.row  = row;
    uact.g    = *yed_line_col_to_glyph(line, line->visual_width);

    yed_push_undo_action(buff, &uact);

    yed_pop_from_line_no_undo(buff, row);
}

void yed_line_clear(yed_buffer *buff, int row) {
    yed_line        *line;
    yed_undo_action  uact;
    yed_glyph       *git;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_GLYPH_POP;
    uact.row  = row;
    yed_line_glyph_rtraverse(*line, git) {
        uact.g = *git;
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
    yed_glyph       *git;

    old_line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_GLYPH_POP;
    uact.row  = row;
    yed_line_glyph_rtraverse(*old_line, git) {
        uact.g = *git;
        yed_push_undo_action(buff, &uact);
    }

    uact.kind = UNDO_GLYPH_PUSH;
    uact.row  = row;
    yed_line_glyph_traverse(*line, git) {
        uact.g = *git;
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
    yed_glyph       *git;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_GLYPH_POP;
    uact.row  = row;
    yed_line_glyph_rtraverse(*line, git) {
        uact.g = *git;
        yed_push_undo_action(buff, &uact);
    }

    uact.kind = UNDO_LINE_DEL;
    uact.row  = row;
    yed_push_undo_action(buff, &uact);

    yed_buff_delete_line_no_undo(buff, row);
}

void yed_insert_into_line(yed_buffer *buff, int row, int col, yed_glyph g) {
    yed_undo_action uact;

    uact.kind = UNDO_GLYPH_ADD;
    uact.row  = row;
    uact.col  = yed_line_normalize_col(yed_buff_get_line(buff, row),  col);
    uact.g    = g;

    yed_push_undo_action(buff, &uact);

    yed_insert_into_line_no_undo(buff, row, col, g);
}

void yed_delete_from_line(yed_buffer *buff, int row, int col) {
    yed_line        *line;
    yed_undo_action  uact;

    line = yed_buff_get_line(buff, row);

    uact.kind = UNDO_GLYPH_DEL;
    uact.row  = row;
    uact.col  = yed_line_normalize_col(yed_buff_get_line(buff, row),  col);
    uact.g    = *yed_line_col_to_glyph(line, col);

    yed_push_undo_action(buff, &uact);

    yed_delete_from_line_no_undo(buff, row, col);
}

void yed_buff_clear(yed_buffer *buff) {
    int              row;
    yed_line        *line;
    yed_undo_action  uact;
    yed_glyph       *git;

    for (row = bucket_array_len(buff->lines); row >= 1; row -= 1) {
        line = yed_buff_get_line(buff, row);

        uact.kind = UNDO_GLYPH_POP;
        uact.row  = row;
        yed_line_glyph_rtraverse(*line, git) {
            uact.g = *git;
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




int yed_line_idx_to_col(yed_line *line, int idx) {
    yed_glyph *g;
    int        i, col, len, n_bytes;

    len = array_len(line->chars);

    if (line->n_glyphs == line->visual_width
    &&  line->n_glyphs == array_len(line->chars)) {
        return idx + 1;
    }

    col = 1;
    for (i = 0; i < idx && i < len;) {
        g       = array_item(line->chars, i);
        n_bytes = yed_get_glyph_len(*g);

        if (i + n_bytes > idx) { break; }

        col += yed_get_glyph_width(*g);
        i   += n_bytes;
    }

    return col;
}

int yed_line_col_to_idx(yed_line *line, int col) {
    yed_glyph *g;
    int        c, i;

    if (col == line->visual_width + 1) {
        return array_len(line->chars);
    }

    if (line->n_glyphs == line->visual_width
    &&  line->n_glyphs == array_len(line->chars)) {
        return col - 1;
    }

    ASSERT(col <= line->visual_width, "unable to convert column to glyph index");

    i = 0;
    for (c = 1; c <= line->visual_width;) {
        g  = array_item(line->chars, i);
        c += yed_get_glyph_width(*g);

        if (col < c) { return i; }

        i += yed_get_glyph_len(*g);
    }

    return i;
}

yed_glyph * yed_line_col_to_glyph(yed_line *line, int col) {
    int idx;

    idx = yed_line_col_to_idx(line, col);

    if (idx == -1) {
        return NULL;
    }

    return (yed_glyph*)array_item(line->chars, idx);
}

yed_glyph * yed_line_last_glyph(yed_line *line) {
    if (line->visual_width == 0) { return NULL; }
    return yed_line_col_to_glyph(line, line->visual_width);
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

int yed_line_normalize_col(yed_line *line, int col) {
    int idx;

    idx = yed_line_col_to_idx(line, col);
    return yed_line_idx_to_col(line, idx);
}

yed_glyph * yed_buff_get_glyph(yed_buffer *buff, int row, int col) {
    yed_line *line;
    int       idx;

    line = yed_buff_get_line(buff, row);
    if (!line) { return NULL; }

    idx = yed_line_col_to_idx(line, col);
    if (idx == -1) { return NULL; }

    return array_item(line->chars, idx);
}



int yed_fill_buff_from_file(yed_buffer *buff, char *path) {
    char        *mode;
    FILE        *f;
    struct stat  fs;
    int          fd;
    int          status;
    char         a_path[4096];

    status = BUFF_FILL_STATUS_SUCCESS;
    errno  = 0;
    f      = fopen(path, "r");
    if (f) {
        fd = fileno(f);

        errno = 0;
        if (fstat(fd, &fs) != 0) {
            errno = 0;
            return BUFF_FILL_STATUS_ERR_NOF;
        } else if (S_ISDIR(fs.st_mode)) {
            errno = 0;
            return BUFF_FILL_STATUS_ERR_DIR;
        }
    }

    if (errno) {
        if (errno == ENOENT) {
            status = BUFF_FILL_STATUS_ERR_NOF;
        } else if (errno == EISDIR) {
            status = BUFF_FILL_STATUS_ERR_DIR;
        } else if (errno == EACCES) {
            status = BUFF_FILL_STATUS_ERR_PER;
        } else {
            status = BUFF_FILL_STATUS_ERR_UNK;
        }

        errno = 0;
        return status;
    }

    if ((mode = yed_get_var("buffer-load-mode"))
    && (strcmp(mode, "map") == 0)) {
        status = yed_fill_buff_from_file_map(buff, fd, fs.st_size);
    } else {
        status = yed_fill_buff_from_file_stream(buff, f);
    }

    if (status != BUFF_FILL_STATUS_SUCCESS) {
        goto cleanup;
    }

    if (abs_path(path, a_path)) {
        buff->path = strdup(a_path);
    } else {
        buff->path = strdup(path);
    }

    yed_buffer_set_ft(buff, FT_UNKNOWN);

    yed_reset_undo_history(&buff->undo_history);

    buff->kind   = BUFF_KIND_FILE;
    buff->flags &= ~BUFF_MODIFIED;
    yed_mark_dirty_frames(buff);

cleanup:
    fclose(f);

    return status;
}

int yed_fill_buff_from_file_map(yed_buffer *buff, int fd, unsigned long long file_size) {
    int          i, line_len;
    char        *file_data, *underlying_buff, *end, *scan, *tmp, c;
    yed_line    *last_line,
                 line;
    yed_glyph   *g;


    yed_buff_clear_no_undo(buff);

    if (file_size == 0) {
        return BUFF_FILL_STATUS_SUCCESS;
    }

    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);

    if (file_data == MAP_FAILED) {
        errno = 0;
        return BUFF_FILL_STATUS_ERR_MAP;
    }

    /*
     * Add 3 bytes of padding so that we don't violate anything
     * when we call yed_get_string_info().
     * See the comment there (src/utf8.c) for more info.
     */
    underlying_buff = malloc(file_size + 3);

    memcpy(underlying_buff, file_data, file_size);

    /*
     * This buffer is going to come to us with a pre-made
     * empty line.
     * We don't need it though.
     */
    last_line = bucket_array_last(buff->lines);
    yed_free_line(last_line);
    bucket_array_pop(buff->lines);


    scan = underlying_buff;
    end  = underlying_buff + file_size;

    while (scan < end) {
        tmp      = memchr(scan, '\n', end - scan);
        line_len = (tmp ? tmp : end) - scan;

        line                   = yed_new_line_with_cap(line_len);
        line.chars.should_free = 0;
        line.chars.data        = scan;
        line.chars.used        = line_len;

        /* Remove '\r' from line. */
        while (array_len(line.chars)
        &&    ((c = *(char*)array_last(line.chars)) == '\r')) {
            array_pop(line.chars);
            line_len -= 1;
        }

        (void)g;
        (void)i;
        yed_get_string_info(line.chars.data, line_len, &line.n_glyphs, &line.visual_width);

        bucket_array_push(buff->lines, line);

        if (unlikely(!tmp)) { break; }

        scan = tmp + 1;
    }

    munmap(file_data, file_size);
    buff->mmap_underlying_buff = underlying_buff;

    if (bucket_array_len(buff->lines) > 1) {
        last_line = bucket_array_last(buff->lines);
        if (array_len(last_line->chars) == 0) {
            bucket_array_pop(buff->lines);
        }
    } else if (bucket_array_len(buff->lines) == 0) {
        line = yed_new_line();
        bucket_array_push(buff->lines, line);
    }

    return BUFF_FILL_STATUS_SUCCESS;
}

int yed_fill_buff_from_file_stream(yed_buffer *buff, FILE *f) {
    ssize_t      line_len;
    size_t       line_cap;
    char         c, *line_data;
    yed_line    *last_line,
                 line;
    yed_glyph   *g;
    int          j;

    yed_buff_clear_no_undo(buff);

    last_line = bucket_array_last(buff->lines);
    yed_free_line(last_line);
    bucket_array_pop(buff->lines);

    while (line_data = NULL, (line_len = getline(&line_data, &line_cap, f)) > 0) {
        line.chars.data      = line_data;
        line.chars.elem_size = 1;
        line.chars.used      = line_len;
        line.chars.capacity  = line_cap;
        line.visual_width    = 0;

        while (array_len(line.chars)
        &&    ((c = *(char*)array_last(line.chars)) == '\n' || c == '\r')) {
            array_pop(line.chars);
            line_len -= 1;
        }

        for (j = 0; j < line_len;) {
            g = array_item(line.chars, j);
            line.visual_width += yed_get_glyph_width(*g);
            line.n_glyphs     += 1;
            j += yed_get_glyph_len(*g);
        }

        bucket_array_push(buff->lines, line);
    }

    if (bucket_array_len(buff->lines) > 1) {
        last_line = bucket_array_last(buff->lines);
        if (array_len(last_line->chars) == 0) {
            bucket_array_pop(buff->lines);
        }
    } else if (bucket_array_len(buff->lines) == 0) {
        line = yed_new_line();
        bucket_array_push(buff->lines, line);
    }

    return BUFF_FILL_STATUS_SUCCESS;
}

int yed_write_buff_to_file(yed_buffer *buff, char *path) {
    FILE      *f;
    yed_line  *line;
    yed_event  event;
    int        status;
    char       a_path[4096];

    memset(&event, 0, sizeof(event));
    event.kind   = EVENT_BUFFER_PRE_WRITE;
    event.buffer = buff;
    yed_trigger_event(&event);

    status = BUFF_WRITE_STATUS_SUCCESS;
    errno  = 0;
    f      = fopen(path, "w");
    if (!f) {
        if (errno == EISDIR) {
            status = BUFF_WRITE_STATUS_ERR_DIR;
        } else if (errno == EACCES) {
            status = BUFF_WRITE_STATUS_ERR_PER;
        } else {
            status = BUFF_WRITE_STATUS_ERR_UNK;
        }

        errno = 0;
        return status;
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

    fclose(f);

    if (!(buff->flags & BUFF_SPECIAL)) {
        if (buff->path) {
            free(buff->path);
        }

        if (abs_path(path, a_path)) {
            buff->path = strdup(a_path);
        } else {
            buff->path = strdup(path);
        }
    }

    if (!(buff->flags & BUFF_SPECIAL)) {
        buff->kind = BUFF_KIND_FILE;
    }

    if (status == BUFF_WRITE_STATUS_SUCCESS) {
        event.kind   = EVENT_BUFFER_POST_WRITE;
        event.buffer = buff;
        yed_trigger_event(&event);
        buff->flags &= ~BUFF_MODIFIED;
    }

    return status;
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
    yed_glyph *g;
    int        r1, c1, r2, c2, ctmp,
               n, i, width;

    r1 = c1 = r2 = c2 = 0;

    range = &buff->selection;

    yed_range_sorted_points(range, &r1, &c1, &r2, &c2);

    if (range->kind == RANGE_LINE) {
        for (i = r1; i <= r2; i += 1) {
            yed_buff_delete_line(buff, r1);
        }
    } else if (r1 == r2) {
        line1 = yed_buff_get_line(buff, r1);
        ctmp = c1;
        while (ctmp < c2) {
            g     = yed_line_col_to_glyph(line1, c1);
            width = yed_get_glyph_width(*g);
            yed_delete_from_line(buff, r1, c1);
            ctmp += width;
        }
    } else {
        line1 = yed_buff_get_line(buff, r1);
        ASSERT(line1, "didn't get line1 in yed_buff_delete_selection()");
        n = 0;
        while (c1 <= line1->visual_width) {
            g      = yed_line_col_to_glyph(line1, c1);
            width  = yed_get_glyph_width(*g);
            c1    += width;
            n     += 1;
        }
        for (i = 0; i < n; i += 1) {
            yed_pop_from_line(buff, r1);
        }
        for (i = r1 + 1; i < r2; i += 1) {
            yed_buff_delete_line(buff, r1 + 1);
        }
        line2 = yed_buff_get_line(buff, r1 + 1);
        ASSERT(line2, "didn't get line2 in yed_buff_delete_selection()");
        ctmp = c2;
        while (ctmp <= line2->visual_width) {
            g = yed_line_col_to_glyph(line2, ctmp);
            yed_append_to_line(buff, r1, *g);
            ctmp += yed_get_glyph_width(*g);
        }
        yed_buff_delete_line(buff, r1 + 1);
    }

    if (!bucket_array_len(buff->lines)) {
        yed_buffer_add_line(buff);
    }

    buff->has_selection = 0;
}

int yed_buff_is_visible(yed_buffer *buff) {
    yed_frame **frame_it;

    array_traverse(ys->frames, frame_it) {
        if ((*frame_it)->buffer == buff) {
            return 1;
        }
    }

    return 0;
}

void yed_update_line_visual_widths(void) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  bit;
    yed_buffer                                   *buff;
    yed_line                                     *line;
    yed_glyph                                    *glyph;

    tree_traverse(ys->buffers, bit) {
        buff = tree_it_val(bit);
        bucket_array_traverse(buff->lines, line) {
            line->visual_width = 0;
            yed_line_glyph_traverse(*line, glyph) {
                line->visual_width += yed_get_glyph_width(*glyph);
            }
        }
    }
}

char *yed_get_selection_text(yed_buffer *buffer) {
    char      nl;
    array_t   chars;
    int       r1;
    int       c1;
    int       r2;
    int       c2;
    int       r;
    yed_line *line;
    int       cstart;
    int       cend;
    int       i;
    int       n;
    char     *data;

    if (!buffer->has_selection) { return NULL; }

    nl    = '\n';
    chars = array_make(char);

    yed_range_sorted_points(&buffer->selection, &r1, &c1, &r2, &c2);

    if (buffer->selection.kind == RANGE_LINE) {
        for (r = r1; r <= r2; r += 1) {
            line = yed_buff_get_line(buffer, r);
            if (line == NULL) { break; } /* should not happen */

            data = (char*)array_data(line->chars);
            array_push_n(chars, data, array_len(line->chars));
            array_push(chars, nl);
        }
    } else {
        for (r = r1; r <= r2; r += 1) {
            line = yed_buff_get_line(buffer, r);
            if (line == NULL) { break; } /* should not happen */

            if (line->visual_width > 0) {
                cstart = r == r1 ? c1 : 1;
                cend   = r == r2 ? c2 : line->visual_width + 1;

                i    = yed_line_col_to_idx(line, cstart);
                n    = yed_line_col_to_idx(line, cend) - i;
                data = array_item(line->chars, i);

                array_push_n(chars, data, n);
            }

            if (r < r2) {
                array_push(chars, nl);
            }
        }
    }

    array_zero_term(chars);

    return (char*)array_data(chars);
}

char *yed_get_line_text(yed_buffer *buffer, int row) {
    yed_line *line;

    line = yed_buff_get_line(buffer, row);

    if (line == NULL) { return NULL; }

    array_zero_term(line->chars);

    return strdup(line->chars.data);
}

char *yed_get_buffer_text(yed_buffer *buffer) {
    char      nl;
    array_t   chars;
    int       n_lines;
    int       row;
    yed_line *line;
    char     *data;

    nl      = '\n';
    chars   = array_make(char);
    n_lines = yed_buff_n_lines(buffer);
    row     = 1;

    bucket_array_traverse(buffer->lines, line) {
        data = (char*)array_data(line->chars);
        array_push_n(chars, data, array_len(line->chars));
        if (row < n_lines) {
            array_push(chars, nl);
        }
        row += 1;
    }

    array_zero_term(chars);

    return (char*)array_data(chars);
}
