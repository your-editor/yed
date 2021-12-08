#include <yed/plugin.h>

static yed_event_handler  pump_handler;
static int                cmd_is_running;
static yed_nb_subproc_t   nb_subproc;
static char              *cmd_string;

static yed_buffer * get_or_make_buffer(void);

static void shell_run_unload(yed_plugin *self);
static void shell_run_pump_handler(yed_event *event);

static void shell_run(int n_args, char **args);
static void shell_run_silent(int n_args, char **args);
static void shell_view_output(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    get_or_make_buffer();

    yed_plugin_set_command(self, "shell-run",         shell_run);
    yed_plugin_set_command(self, "shell-run-silent",  shell_run_silent);
    yed_plugin_set_command(self, "shell-view-output", shell_view_output);

    pump_handler.kind = EVENT_PRE_PUMP;
    pump_handler.fn   = shell_run_pump_handler;

    yed_plugin_add_event_handler(self, pump_handler);

    yed_plugin_set_unload_fn(self, shell_run_unload);

    return 0;
}

static yed_buffer * get_or_make_buffer(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*shell-output");
    if (buff == NULL) {
        buff = yed_create_buffer("*shell-output");
        buff->flags |= BUFF_SPECIAL | BUFF_RD_ONLY;
    }

    return buff;
}

static void shell_run_unload(yed_plugin *self) {
    yed_buffer *buff;

    buff = yed_get_buffer("*shell-output");
    if (buff != NULL) {
        yed_free_buffer(buff);
    }

    if (cmd_string != NULL) {
        free(cmd_string);
        cmd_string = NULL;
    }
}

static void shell_run_update(void) {
    yed_frame **fit;
    int         last_row;

LOG_CMD_ENTER("shell-run");

    get_or_make_buffer()->flags &= ~BUFF_RD_ONLY;
    cmd_is_running = yed_read_subproc_into_buffer_nb(&nb_subproc);
    get_or_make_buffer()->flags |= BUFF_RD_ONLY;

    if (!cmd_is_running) {
        if (nb_subproc.err) {
            yed_cerr("something went wrong -- errno = %d\n", nb_subproc.err);
        } else {
            if (nb_subproc.exit_status != 0) {
                yed_cerr("'%s' failed with error code %d", cmd_string, nb_subproc.exit_status);
            }
        }
        free(cmd_string);
        cmd_string = NULL;
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

static void shell_run_pump_handler(yed_event *event) {
    if (cmd_is_running) {
        shell_run_update();
    }
}

static void do_shell_run(int n_args, char **args) {
    array_t     string_build;
    const char *lazy_space;
    int         i;
    char       *full_cmd;

    string_build = array_make(char);

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        array_push_n(string_build, (char*)lazy_space, strlen(lazy_space));
        array_push_n(string_build, args[i], strlen(args[i]));
        lazy_space = " ";
    }

    array_zero_term(string_build);

    full_cmd = malloc(array_len(string_build) + 16);
    snprintf(full_cmd, array_len(string_build) + 16,
             "(%s) 2>&1", (char*)array_data(string_build));


    get_or_make_buffer()->flags &= ~BUFF_RD_ONLY;

    if (yed_start_read_subproc_into_buffer_nb(full_cmd, get_or_make_buffer(), &nb_subproc)) {
        get_or_make_buffer()->flags |= BUFF_RD_ONLY;

        yed_cerr("there was an error when calling yed_start_read_subproc_into_buffer_nb()");
        free(full_cmd);
        array_free(string_build);
        return;
    }

    get_or_make_buffer()->flags |= BUFF_RD_ONLY;

    free(full_cmd);

    cmd_string      = array_data(string_build);
    cmd_is_running  = 1;

    /*
    ** Wait 15 milliseconds so that the subprocess has time to start and
    ** do _something_.
    ** This makes it so that we can start to produce output before the next
    ** pump (which could be ~300 ms if there aren't any keypresses).
    */
    usleep(15000);
    shell_run_update();
}

static void shell_run(int n_args, char **args) {
    if (cmd_is_running) {
        yed_cerr("a command is already running!");
        return;
    }

    if (n_args == 0) {
        yed_cerr("expected at least 1 argument, but got %d", n_args);
        return;
    }

    do_shell_run(n_args, args);
    YEXE("special-buffer-prepare-focus", "*shell-output");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*shell-output");
}

static void shell_run_silent(int n_args, char **args) {
    if (cmd_is_running) {
        yed_cerr("a command is already running!");
        return;
    }

    if (n_args == 0) {
        yed_cerr("expected at least 1 argument, but got %d", n_args);
        return;
    }

    do_shell_run(n_args, args);
}

static void shell_view_output(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*shell-output");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*shell-output");
}
