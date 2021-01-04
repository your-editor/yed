#include <yed/plugin.h>

void find_file(int n_args, char **args);
void find_file_start(void);
void find_file_take_key(int key);
void find_file_run(void);
void find_file_select(void);

void find_file_key_pressed_handler(yed_event *event);

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*find-file-list");

    if (buff == NULL) {
        buff = yed_create_buffer("*find-file-list");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}

static char *prg;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = find_file_key_pressed_handler;

    yed_plugin_add_event_handler(self, h);
    yed_plugin_set_command(self, "find-file", find_file);
    yed_plugin_set_completion(self, "find-file-compl-arg-0", yed_get_completion("file"));

    if (!yed_get_var("find-file-prg")) {
        yed_set_var("find-file-prg",
                    "find . -path ./.git -prune -o -type f -name '*%*' -print");
    }

    return 0;
}

void find_file(int n_args, char **args) {
    int       i;
    int       key;
    yed_line *line;

    if (!ys->interactive_command) {
        prg = yed_get_var("find-file-prg");
        if (!prg) {
            yed_cerr("'find-file-prg' not set");
            return;
        }
        find_file_start();

        if (n_args) {
            for (i = 0; i < strlen(args[0]); i += 1) {
                yed_cmd_line_readline_take_key(NULL, (int)args[0][i]);
            }
            array_zero_term(ys->cmd_buff);
            find_file_run();
            ys->interactive_command = NULL;
            yed_clear_cmd_buff();
            if (yed_buff_n_lines(get_or_make_buff()) == 1) {
                line = yed_buff_get_line(get_or_make_buff(), 1);
                if (line->visual_width) {
                    find_file_select();
                }
            }
        }
    } else {
        sscanf(args[0], "%d", &key);
        find_file_take_key(key);
    }
}

void find_file_start(void) {
    ys->interactive_command = "find-file";
    ys->cmd_prompt          = "(find-file) ";

    yed_buff_clear_no_undo(get_or_make_buff());
    YEXE("special-buffer-prepare-focus", "*find-file-list");
    yed_frame_set_buff(ys->active_frame, get_or_make_buff());
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    yed_clear_cmd_buff();
}

void find_file_take_key(int key) {
    yed_line *line;

    switch (key) {
        case ESC:
        case CTRL_C:
            ys->interactive_command = NULL;
            ys->current_search      = NULL;
            yed_clear_cmd_buff();
            YEXE("special-buffer-prepare-unfocus", "*find-file-list");
            break;
        case ENTER:
            ys->interactive_command = NULL;
            yed_clear_cmd_buff();
            if (yed_buff_n_lines(get_or_make_buff()) == 1) {
                line = yed_buff_get_line(get_or_make_buff(), 1);
                if (line->visual_width) {
                    find_file_select();
                }
            }
            break;
        default:
            yed_cmd_line_readline_take_key(NULL, key);
            array_zero_term(ys->cmd_buff);
            find_file_run();
            break;
    }
}

void find_file_run(void) {
    char       cmd_buff[1024];
    char      *pattern;
    int        len, status;

    cmd_buff[0] = 0;
    pattern     = array_data(ys->cmd_buff);

    if (strlen(pattern) == 0)     { goto empty; }

    len = perc_subst(prg, pattern, cmd_buff, sizeof(cmd_buff));

    ASSERT(len > 0, "buff too small for perc_subst");

    strcat(cmd_buff, " 2>/dev/null");

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
        goto empty;
    }

    if (status != 0) {
empty:;
        yed_buff_clear_no_undo(get_or_make_buff());
    }
}

void find_file_select(void) {
    yed_line *line;
    char     *_path, *path;

    line = yed_buff_get_line(get_or_make_buff(), ys->active_frame->cursor_line);
    array_zero_term(line->chars);
    _path = path = strdup(array_data(line->chars));

    if (strlen(path) >= 2) {
        if (*path == '.' && *(path + 1) == '/') {
            path += 2;
        }
    }


    YEXE("special-buffer-prepare-jump-focus", "*grep-list");
    YEXE("buffer", path);

    free(_path);
}

void find_file_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                                /* not the key we want */
    ||  ys->interactive_command                            /* still typing */
    ||  !eframe                                            /* no frame */
    ||  !eframe->buffer                                    /* no buffer */
    ||  strcmp(eframe->buffer->name, "*find-file-list")) { /* not our buffer */
        return;
    }

    find_file_select();

    event->cancel = 1;
}
