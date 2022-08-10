static void yed_key_bind_handler(yed_event *event) {
    if (yed_buff_is_visible(yed_get_bindings_buffer())) {
        yed_update_bindings_buffer();
    }
}

static void yed_bindings_buffer_focus_handler(yed_event *event) {
    if (event->buffer == yed_get_or_create_special_rdonly_buffer("*bindings")) {
        yed_update_bindings_buffer();
    }
}

static void yed_var_change_handler(yed_event *event) {
    int old_tabw;

    if (strcmp(event->var_name, "tab-width") == 0) {
        old_tabw = ys->tabw;
        yed_get_var_as_int("tab-width", &ys->tabw);
        if (ys->tabw < 1) { ys->tabw = 1; }

        if (ys->tabw != old_tabw) {
            yed_update_line_visual_widths();
        }
    } else if (strcmp(event->var_name, "cursor-line") == 0) {
    } else if (strcmp(event->var_name, "fill-string") == 0) {
    }

    if (yed_buff_is_visible(yed_get_vars_buffer())) {
        yed_update_vars_buffer();
    }
}

static void yed_var_buffer_focus_handler(yed_event *event) {
    if (event->buffer == yed_get_or_create_special_rdonly_buffer("*vars")) {
        yed_update_vars_buffer();
    }
}

void yed_search_line_handler(yed_event *event) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    yed_attrs  *attr, search, search_cursor, *set;
    char       *line_data,
               *line_data_end,
               *scan;
    int         i, idx, col,
                search_len,
                search_width,
                data_len;

    if (!ys->current_search) {
        return;
    }

    if (!event->frame) {
        return;
    }

    frame = event->frame;

    if (frame != ys->active_frame) {
        return;
    }

    if (!frame->buffer) {
        return;
    }

    buff         = frame->buffer;
    line         = yed_buff_get_line(buff, event->row);
    data_len     = array_len(line->chars);
    search_len   = strlen(ys->current_search);
    search_width = yed_get_string_width(ys->current_search);

    if (!line->visual_width || !search_len)    { return; }

    line_data     = scan = array_data(line->chars);
    line_data_end = line_data + data_len;

    search        = yed_active_style_get_search();
    search_cursor = yed_active_style_get_search_cursor();

    while (scan < line_data_end && line_data_end - scan >= search_len) {
        if (!(scan = strnstr(scan, ys->current_search, line_data_end - scan))) {
            break;
        }

        idx = scan - line_data;
        col = yed_line_idx_to_col(line, idx);

        set = (event->row == frame->cursor_line
                &&    col == frame->cursor_col)
                    ? &search_cursor
                    : &search;

        for (i = 0; i < search_width; i += 1) {
            attr = array_item(event->line_attrs, col + i - 1);
            if (ys->active_style) {
                yed_combine_attrs(attr, set);
            } else {
                attr->flags ^= ATTR_INVERSE;
            }
        }

        scan += 1;
    }
}

void yed_replace_cursor_handler(yed_event *event) {
    if (ys->interactive_command != NULL
    &&  strcmp(ys->interactive_command, "replace-current-search") == 0) {

        event->cancel = 1;
    }
}
