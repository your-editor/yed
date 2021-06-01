#include <yed/plugin.h>

static yed_plugin        *Self;
static yed_event_handler  pump_handler;
static yed_event_handler  style_handler;
static yed_event_handler  line_handler;
static yed_event_handler  buff_post_mod_handler;
static yed_event_handler  buff_post_write_handler;
static yed_event_handler  cursor_moved_handler;
static int                build_is_running;
static int                build_failed;
static char              *build_cmd;
static int                builder_run_before;
static int                notif_up;
static u64                notif_start_ms;
static u64                notif_stay_ms;
static array_t            dd_lines;
static int                has_err;
static int                err_fixed;
static char               err_msg[1024];
static yed_direct_draw_t *err_dd;
static char               err_file[512];
static int                err_line;
static int                err_col;
static int                err_has_loc;
static char               builder_notif_cmd[1024];
static int                builder_notif_cmd_status;

static yed_nb_subproc_t   nb_subproc;

#define LOCKED(_m)                                 \
for (int _foozle = (pthread_mutex_lock(&(_m)), 1); \
     _foozle;                                      \
     _foozle = (pthread_mutex_unlock(&(_m)), 0))

static yed_buffer * get_or_make_buffer(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*builder-output");
    if (buff == NULL) {
        buff = yed_create_buffer("*builder-output");
        buff->flags |= BUFF_SPECIAL | BUFF_RD_ONLY;
    }

    return buff;
}

static void builder_unload(yed_plugin *self);
static void builder_pump_handler(yed_event *event);
static void builder_line_handler(yed_event *event);
static void builder_buff_post_mod_handler(yed_event *event);
static void builder_buff_post_write_handler(yed_event *event);
static void builder_cursor_moved_handler(yed_event *event);
static void builder_style_handler(yed_event *event);
static void builder_jump_to_error_location(int n_args, char **args);
static void builder_print_error(int n_args, char **args);
static void builder_view_output(int n_args, char **args);
static void builder_echo_status(int n_args, char **args);
static void notif_start(void);
static void notif_stop(void);

static void builder_start(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    Self = self;

    YED_PLUG_VERSION_CHECK();

    get_or_make_buffer();

    yed_plugin_set_unload_fn(self, builder_unload);

    yed_plugin_set_command(self, "builder-start",         builder_start);
    yed_plugin_set_command(self, "builder-jump-to-error", builder_jump_to_error_location);
    yed_plugin_set_command(self, "builder-print-error",   builder_print_error);
    yed_plugin_set_command(self, "builder-view-output",   builder_view_output);
    yed_plugin_set_command(self, "builder-echo-status",   builder_echo_status);

    pump_handler.kind            = EVENT_PRE_PUMP;
    pump_handler.fn              = builder_pump_handler;
    line_handler.kind            = EVENT_LINE_PRE_DRAW;
    line_handler.fn              = builder_line_handler;
    buff_post_mod_handler.kind   = EVENT_BUFFER_POST_MOD;
    buff_post_mod_handler.fn     = builder_buff_post_mod_handler;
    buff_post_write_handler.kind = EVENT_BUFFER_POST_WRITE;
    buff_post_write_handler.fn   = builder_buff_post_write_handler;
    cursor_moved_handler.kind    = EVENT_CURSOR_MOVED;
    cursor_moved_handler.fn      = builder_cursor_moved_handler;
    style_handler.kind           = EVENT_STYLE_CHANGE;
    style_handler.fn             = builder_style_handler;

    yed_plugin_add_event_handler(self, pump_handler);
    yed_plugin_add_event_handler(self, style_handler);

    notif_up      = 0;
    notif_stay_ms = 6000;
    dd_lines      = array_make(yed_direct_draw_t*);

    return 0;
}

static void builder_cleanup(void) {
    if (builder_run_before) {
        notif_stop();
        free(build_cmd);
        build_cmd = NULL;
    }
}

static void builder_unload(yed_plugin *self) {
    yed_buffer *buff;

    buff = yed_get_buffer("*builder-output");
    if (buff != NULL) {
        yed_free_buffer(buff);
    }

    builder_cleanup();
}

static void notif_start(void) {
    char              *third_line;
    char               test_buff[256];
    int                line_len;
    int                side_padding;
    yed_attrs          inv;
    char               line_buff[512];
    char              *line;
    int                i;
    yed_direct_draw_t *dd;

    if (notif_up) { return; }

    third_line = build_failed
                    ? "builder-jump-to-error and builder-view-output are available."
                    : "builder-view-output is available.";

    sprintf(test_buff, "Build command '%s':", build_cmd);

    line_len      = MAX(strlen(test_buff), strlen(third_line));
    side_padding  = 2;
    line_len     += 2 * side_padding;

    inv             = yed_active_style_get_active();
    inv.flags      |= ATTR_INVERSE;

    line         = test_buff;
    line_buff[0] = 0;
    for (i = 0; i < side_padding; i += 1) { strcat(line_buff, " "); }
    strcat(line_buff, line);
    for (i = 0; i < line_len - side_padding - strlen(line); i += 1) { strcat(line_buff, " "); }
    dd = yed_direct_draw_style(ys->term_rows - 2 - 3/* 2 */,
                               ys->term_cols - line_len,
                               STYLE_status_line,
                               line_buff);
    array_push(dd_lines, dd);

    line         = build_failed ? "FAILED" : "SUCCEEDED";
    line_buff[0] = 0;
    for (i = 0; i < side_padding; i += 1) { strcat(line_buff, " "); }
    strcat(line_buff, line);
    for (i = 0; i < line_len - side_padding - strlen(line); i += 1) { strcat(line_buff, " "); }
    dd = yed_direct_draw_style(ys->term_rows - 2 - 2/* 3 */,
                               ys->term_cols - line_len,
                               STYLE_status_line,
                               line_buff);
    array_push(dd_lines, dd);

    line         = third_line;
    line_buff[0] = 0;
    for (i = 0; i < side_padding; i += 1) { strcat(line_buff, " "); }
    strcat(line_buff, line);
    for (i = 0; i < line_len - side_padding - strlen(line); i += 1) { strcat(line_buff, " "); }
    dd = yed_direct_draw_style(ys->term_rows - 2 - 1/* 4 */,
                               ys->term_cols - line_len,
                               STYLE_status_line,
                               line_buff);
    array_push(dd_lines, dd);



    notif_start_ms  = measure_time_now_ms();
    notif_up        = 1;
}

static void notif_stop(void) {
    yed_direct_draw_t **dit;
    yed_direct_draw_t  *dd;

    if (!notif_up) { return; }

    array_traverse(dd_lines, dit) {
        dd = *dit;
        yed_kill_direct_draw(dd);
    }
    notif_up = 0;
    array_clear(dd_lines);
}

static const char * builder_get_status_string(void) {
    if (build_is_running) {
        return "a build is currently running";
    } else if (builder_run_before) {
        if (build_failed) {
            return "the build FAILED";
        }
        return "the build SUCCEEDED";
    }

    return "a build has not been started";
}

static void * builder_bg_cmd_thread_fn(void *arg) {
    char *notif_cmd;
    char  message_buff[512];
    char  expand_buff[1024];
    char  cmd_buff[1200];
    char *output;
    int   output_len;
    int   status;


    notif_cmd = arg;
    snprintf(message_buff, sizeof(message_buff), "'%s (builder-build-command: %s)'",
             builder_get_status_string(), build_cmd);

    perc_subst(notif_cmd, message_buff, expand_buff, sizeof(expand_buff));

    sprintf(cmd_buff, "(%s) 2>&1", expand_buff);

    output = yed_run_subproc(cmd_buff, &output_len, &status);

    if (output) { free(output); }

    builder_notif_cmd_status = status;

    return NULL;
}

static void builder_report(void) {
    char      *notif_cmd;
    pthread_t  p;

    if (notif_up) {
        notif_stop();
    }

    if ((notif_cmd = yed_get_var("builder-notify-command"))) {
        strncpy(builder_notif_cmd, notif_cmd, sizeof(builder_notif_cmd));
        pthread_create(&p, NULL, builder_bg_cmd_thread_fn, notif_cmd);
    } else {
        notif_start();
    }

    YEXE("builder-echo-status");
}

static yed_attrs get_err_attrs(void) {

    yed_attrs a;
    yed_attrs active;
    yed_attrs attn;
    float     brightness;

    active = yed_active_style_get_active();

    a = active;

    if (a.flags & ATTR_RGB) {
        brightness = ((RGB_32_r(active.bg) + RGB_32_g(active.bg) + RGB_32_b(active.bg)) / 3) / 255.0f;

        a.bg = RGB_32(0x7f + (u32)(brightness * 0x7f),
                        0x0  + (u32)(brightness * 0x7f),
                        0x0  + (u32)(brightness * 0x7f));
    } else {
        attn    = yed_active_style_get_attention();
        a.flags = attn.flags & ~(ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG | ATTR_INVERSE);
        if (attn.flags & ATTR_16_LIGHT_FG) {
            a.flags |= ATTR_16_LIGHT_BG;
        }
        a.bg = attn.fg;
    }

    return a;
}

static void builder_draw_error_message(int do_draw) {
    char      line_buff[sizeof(err_msg) + 8];
    int       line_len;
    int       n_glyphs;
    int       line_width;

    if ((do_draw && err_dd) || (!do_draw && !err_dd)) {
        return;
    }

    if (do_draw) {
        if (err_has_loc) {
            sprintf(line_buff, "  %s  ", err_msg);
            line_len = strlen(line_buff);
            yed_get_string_info(line_buff, line_len, &n_glyphs, &line_width);

            err_dd = yed_direct_draw(ys->term_rows - 3,
                                     ys->term_cols - line_width,
                                     get_err_attrs(),
                                     line_buff);
        }
    } else {
        yed_kill_direct_draw(err_dd);
        err_dd = 0;
    }
}

void builder_update_running(void) {
    yed_frame **fit;
    int         last_row;

LOG_FN_ENTER();

    build_is_running = yed_read_subproc_into_buffer_nb(&nb_subproc);
    if (!build_is_running) {
        builder_run_before = 1;
        if (nb_subproc.err && nb_subproc.err != ECHILD) {
            yed_cerr("something went wrong -- errno = %d\n", nb_subproc.err);
            build_failed = 1;
        } else {
            build_failed = nb_subproc.exit_status != 0 || nb_subproc.err == ECHILD;
            builder_report();
        }
        err_fixed = 0;
    }

    array_traverse(ys->frames, fit) {
        if (*fit == ys->active_frame) { continue; }
        if ((*fit)->buffer == get_or_make_buffer()) {
            last_row = yed_buff_n_lines((*fit)->buffer);
            yed_set_cursor_far_within_frame(*fit, last_row, 1);
        }
    }

LOG_EXIT();
}

void builder_pump_handler(yed_event *event) {
    LOG_FN_ENTER();

    u64 now_ms;

    if (build_is_running) {
        builder_update_running();
    }

    if (notif_up) {
        now_ms = measure_time_now_ms();
        if (now_ms - notif_start_ms >= notif_stay_ms) {
            notif_stop();
        }
    } else if (builder_notif_cmd_status != 0) {
        yed_cerr("builder-notify-command '%s' failed with error code %d", builder_notif_cmd, builder_notif_cmd_status);
        builder_notif_cmd_status = 0;
    }

    LOG_EXIT();
}

void builder_line_handler(yed_event *event) {
    yed_buffer *buff;
    yed_attrs  *ait;
    yed_attrs   a;

    if (!has_err)               { return; }
    if (event->row != err_line) { return; }

    buff = yed_get_buffer_by_path(err_file);

    if (event->frame->buffer != buff) { return; }

    a = get_err_attrs();

    array_traverse(event->line_attrs, ait) {
        *ait = a;
    }
}

static void builder_buff_post_mod_handler(yed_event *event) {
    yed_buffer *buff;

    buff = yed_get_buffer_by_path(err_file);

    if (event->frame->buffer != buff) { return; }

    builder_draw_error_message(0);

    yed_delete_event_handler(line_handler);
    yed_delete_event_handler(buff_post_mod_handler);
    yed_delete_event_handler(buff_post_write_handler);
    yed_delete_event_handler(cursor_moved_handler);

    has_err    = 0;
    err_fixed  = 1;
    ys->redraw = 1;
}

static void builder_buff_post_write_handler(yed_event *event) {
    yed_buffer *buff;

    buff = yed_get_buffer_by_path(err_file);

    if (event->buffer != buff) { return; }

    builder_draw_error_message(0);

    yed_delete_event_handler(line_handler);
    yed_delete_event_handler(buff_post_mod_handler);
    yed_delete_event_handler(buff_post_write_handler);
    yed_delete_event_handler(cursor_moved_handler);

    has_err    = 0;
    err_fixed  = 1;
    ys->redraw = 1;
}

static void builder_cursor_moved_handler(yed_event *event) {
    yed_buffer *buff;

    if (has_err && event->frame->cursor_line == err_line) {
        buff = yed_get_buffer_by_path(err_file);
        if (event->frame->buffer == buff) {
            builder_draw_error_message(1);
            return;
        }
    }

    builder_draw_error_message(0);
}

static void builder_style_handler(yed_event *event) {
    if (err_dd) {
        builder_draw_error_message(0);
        builder_draw_error_message(1);
    }
}

static void builder_set_err(int has_loc, char *file, int line, int col, char *msg) {
    int max_err_len;

    if (!has_err && has_loc) {
        yed_plugin_add_event_handler(Self, line_handler);
        yed_plugin_add_event_handler(Self, buff_post_mod_handler);
        yed_plugin_add_event_handler(Self, buff_post_write_handler);
        yed_plugin_add_event_handler(Self, cursor_moved_handler);
    }

    has_err      = 1;

    err_has_loc = has_loc;
    err_file[0] = 0;
    if (err_has_loc) {
        strcat(err_file, file);
        err_line    = line;
        err_col     = MAX(col, 1);
    }
    err_fixed   = 0;
    err_msg[0]  = 0;

    max_err_len = (ys->term_cols / 4 * 3) - 4; /* 4 is the padding on both sides. */

    strncat(err_msg, msg, max_err_len - 3);
    if (strlen(msg) > max_err_len) {
        strcat(err_msg, "...");
    }


    builder_draw_error_message(1);
}

static void builder_jump_to_error_location(int n_args, char **args) {
    char        *err_parse_cmd;
    char         cmd_buff[512];
    char        *parse_output;
    char        *parse_output_cpy;
    int          subproc_status;
    int          parse_status;
    char         c;
    char        *scan;
    char        *bump;
    char         file_buff[256];
    int          line;
    int          col;
    yed_buffer  *buff;
    yed_frame  **fit;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!builder_run_before) {
        yed_cerr("builder has not been run!");
        return;
    }

    if (!build_failed || err_fixed) {
        yed_cerr("no errors to jump to");
        return;
    }

    notif_stop();

    if (!(err_parse_cmd = yed_get_var("builder-error-parse-command"))) {
        err_parse_cmd = "awk -F':' '"
                        "    /:( fatal)? error:/ { print $0; }"
                        "    /: undefined reference/ { printf(\"%s:%s:1:%s\\n\", $1, $2, $3); }"
                        "'";
    }

    sprintf(cmd_buff, "(%s | head -n1) 2>&1", err_parse_cmd);
    parse_output     = NULL;
    parse_output_cpy = NULL;
    subproc_status = yed_write_buffer_to_subproc(get_or_make_buffer(), cmd_buff, &parse_status, &parse_output);
    if (subproc_status != 0) {
        yed_cerr("encountered an error when running builder-error-parse-command -- errno = %d", subproc_status);
        goto out;
    }

    if (parse_status != 0) {
        yed_cerr("'%s' exited with non-zero status %d", cmd_buff, parse_status);
        goto out;
    }

    while ((c = parse_output[strlen(parse_output) - 1]) == '\n'
    ||     c == '\r') {
        parse_output[strlen(parse_output) - 1] = 0;
    }

    parse_output_cpy = strdup(parse_output);

    file_buff[0] = 0;
    scan         = parse_output;

    if (!(bump = strchr(scan, ':'))) { goto invalid; }    *bump = 0;
    strcat(file_buff, scan);
    scan = bump + 1;

    if (!(bump = strchr(scan, ':'))) { goto invalid; }    *bump = 0;
    line = 1;
    sscanf(scan, "%d", &line);
    scan = bump + 1;

    if (!(bump = strchr(scan, ':'))) { goto invalid; }    *bump = 0;
    col = 1;
    sscanf(scan, "%d", &col);
    col = MAX(1, col);
    scan = bump + 2;

    buff = yed_get_buffer_by_path(file_buff);
    if (buff) {
        if (yed_buff_is_visible(buff)) {
            /*
             * Pre-pass to make sure that if the active frame has the buffer,
             * we don't switch to another frame that might also have the buffer.
             */
            array_traverse(ys->frames, fit) {
                if ((*fit)->buffer == buff
                &&  *fit == ys->active_frame) {
                    goto done;
                }
            }

            array_traverse(ys->frames, fit) {
                if ((*fit)->buffer == buff) {
                    yed_activate_frame(*fit);
                    goto done;
                }
            }
done:;
        } else {
            YEXE("buffer", file_buff);
        }
    } else {
        if (!ys->active_frame) {
            YEXE("frame-new");
        }

        YEXE("buffer", file_buff);
    }

    builder_set_err(1, file_buff, line, col, scan);

    yed_set_cursor_far_within_frame(ys->active_frame, line, col);

    YEXE("builder-print-error");

    goto out;

invalid:
    yed_cerr("Couldn't parse location from \"%s\"", parse_output_cpy);
    builder_set_err(0, "", 0, 0, parse_output_cpy);

out:
    if (parse_output) {
        free(parse_output);
    }
    if (parse_output_cpy) {
        free(parse_output_cpy);
    }
}

static void builder_print_error(int n_args, char **args) {
    char *err_parse_cmd;
    char  cmd_buff[512];
    char *parse_output;
    int   subproc_status;
    int   parse_status;
    char  c;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!builder_run_before) {
        yed_cerr("builder has not been run!");
        return;
    }

    if (!build_failed || err_fixed) {
        yed_cerr("no errors to report");
        return;
    }

    if (err_has_loc) {
        yed_cprint("%s:%d:%d: %s", err_file, err_line, err_col, err_msg);
        return;
    }

    if (!(err_parse_cmd = yed_get_var("builder-error-parse-command"))) {
        err_parse_cmd = "awk -F':' '"
                        "    /:( fatal)? error:/ { print $0; }"
                        "    /: undefined reference/ { printf(\"%s:%s:1:%s\\n\", $1, $2, $3); }"
                        "'";
    }

    sprintf(cmd_buff, "(%s | head -n1) 2>&1", err_parse_cmd);

    parse_output   = NULL;
    subproc_status = yed_write_buffer_to_subproc(get_or_make_buffer(), cmd_buff, &parse_status, &parse_output);
    if (subproc_status != 0) {
        yed_cerr("encountered an error when running builder-error-parse-command -- errno = %d", subproc_status);
        goto out;
    }

    if (parse_status != 0) {
        yed_cerr("'%s' exited with non-zero status %d", err_parse_cmd, parse_status);
        goto out;
    }

    while ((c = parse_output[strlen(parse_output) - 1]) == '\n'
    ||     c == '\r') {
        parse_output[strlen(parse_output) - 1] = 0;
    }

    yed_cprint("%s", parse_output);

out:;
    if (parse_output) { free(parse_output); }
}

static void builder_view_output(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    notif_stop();

    YEXE("special-buffer-prepare-focus", "*builder-output");
    YEXE("buffer", "*builder-output");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
}

static void builder_echo_status(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint((char*)builder_get_status_string());
}

static void builder_start(int n_args, char **args) {
    char *cmd;
    yed_buffer *buff;
    char cmd_buff[1024];

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (n_args == 1) {
        cmd = args[0];
    } else if (!(cmd = yed_get_var("builder-build-command"))) {
        yed_cerr("'builder-build-command' is not set!", n_args);
        return;
    }

    if (build_is_running) {
        yed_cerr("a build is already running!", n_args);
        return;
    }

    yed_cprint("running build command: '%s'", cmd);

    builder_cleanup();

    build_cmd = strdup(cmd);

    has_err           = 0;
    err_fixed         = 1;
    buff              = get_or_make_buffer();

    yed_buff_clear_no_undo(buff);

    snprintf(cmd_buff, sizeof(cmd_buff),
             "(%s) 2>&1", cmd);

    if (yed_start_read_subproc_into_buffer_nb(cmd_buff, buff, &nb_subproc)) {
        yed_cerr("there was an error when calling yed_start_read_subproc_into_buffer_nb()");
        return;
    }

    build_is_running  = 1;
    /*
    ** Wait 15 milliseconds so that the subprocess has time to start and
    ** do _something_.
    ** This makes it so that we can start to produce output before the next
    ** pump (which could be ~300 ms if there aren't any keypresses).
    */
    usleep(15000);
    builder_update_running();
}
