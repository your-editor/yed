void yed_init_log(void) {
    ys->log_name_stack = array_make(char*);

    LOG_FN_ENTER();
    yed_log("logging initialized");
    LOG_EXIT();
}

void yed__log_prints(char *s, int len) {
    int       row, i;
    yed_glyph g;

    row = yed_buff_n_lines(ys->log_buff);

    for (i = 0; i < len; i += 1) {
        g.c = s[i];
        if (g.c == '\n') {
            row = yed_buffer_add_line_no_undo(ys->log_buff);
        } else {
            yed_append_to_line_no_undo(ys->log_buff, row, g);
        }
    }

    yed_mark_dirty_frames(ys->log_buff);
}

int yed_vlog(char *fmt, va_list args) {
    char       tm_buff[128], nm_tm_buff[512], buff[1024];
    time_t     t;
    struct tm *tm;
    char      *log_name, *header_fmt;
    int        len, new_header;

    log_name   = *(char**)array_last(ys->log_name_stack);
    new_header = 0;

    /* If we don't have the names, do the new header. */
    if (!log_name || !ys->cur_log_name) {
        new_header = 1;
    }

    /* Or if there's a mismatch. */
    if (!new_header && strcmp(log_name, ys->cur_log_name) == 0) {
        new_header = 1;
    }

    if (new_header) {
        ys->cur_log_name = log_name;

        if (!log_name) { log_name = "???"; }

        t  = time(NULL);
        tm = localtime(&t);
        strftime(tm_buff, sizeof(tm_buff), "%D %I:%M:%S", tm);

        if (yed_buff_n_lines(ys->log_buff) == 1
        &&  yed_buff_get_line(ys->log_buff, 1)->visual_width == 0) {
            header_fmt = "[%s](%s) ";
        } else {
            header_fmt = "\n[%s](%s) ";
        }

        len = snprintf(nm_tm_buff, sizeof(nm_tm_buff), header_fmt, tm_buff, log_name);

        if (len > sizeof(nm_tm_buff) - 1) {
            len = sizeof(nm_tm_buff) - 1;
        }

        yed__log_prints(nm_tm_buff, len);
    }

    len = vsnprintf(buff, sizeof(buff), fmt, args);

    if (len > sizeof(buff) - 1) {
        len = sizeof(buff) - 1;
    }

    if (new_header && len && buff[0] == '\n') {
        yed__log_prints(buff + 1, len - 1);
    } else {
        yed__log_prints(buff, len);
    }

    return new_header;
}

int yed_log(char *fmt, ...) {
    va_list va;
    int     r;

    va_start(va, fmt);
    r = yed_vlog(fmt, va);
    va_end(va);

    return r;
}
