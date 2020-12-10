#include <yed/plugin.h>

void find_file(int n_args, char **args);
void find_file_start(void);
void find_file_cleanup(void);
void find_file_take_key(int key);
void find_file_make_buffer(void);
void find_file_run(void);
void find_file_update_buff(void);
void find_file_select(void);
void find_file_set_prompt(char *p, char *attr);

void find_file_key_pressed_handler(yed_event *event);

static char       *prg;
static yed_buffer *buff;
static char        prompt_buff[256];

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = find_file_key_pressed_handler;

    yed_plugin_add_event_handler(self, h);
    yed_plugin_set_command(self, "find-file", find_file);

    if (!yed_get_var("find-file-prg")) {
        yed_set_var("find-file-prg",
                    "find . -path ./.git -prune -o -type f -name \"*%*\" -print");
    }

    return 0;
}

void find_file(int n_args, char **args) {
    int key;

    if (!ys->interactive_command) {
        prg = yed_get_var("find-file-prg");
        if (!prg) {
            yed_cerr("'find-file-prg' not set");
            return;
        }
        find_file_start();
    } else {
        sscanf(args[0], "%d", &key);
        find_file_take_key(key);
    }
}

void find_file_cleanup(void) {
/*     yed_free_buffer(buff); */
/*  */
/*     buff  = NULL; */
}

void find_file_start(void) {
    ys->interactive_command = "find-file";
    find_file_set_prompt("(find-file) ", NULL);

    if (buff == NULL) {
        find_file_make_buffer();
    } else {
        yed_buff_clear_no_undo(buff);
    }
    YEXE("special-buffer-prepare-focus", "*find-file-list");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    yed_frame_set_buff(ys->active_frame, buff);
    yed_clear_cmd_buff();
}

void find_file_take_key(int key) {
    yed_line *line;

    if (key == CTRL_C) {
        ys->interactive_command = NULL;
        ys->current_search      = NULL;
        yed_clear_cmd_buff();
        YEXE("special-buffer-prepare-unfocus", "*find-file-list");
        find_file_cleanup();
    } else if (key == ENTER) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
        if (yed_buff_n_lines(buff) == 1) {
            line = yed_buff_get_line(buff, 1);
            if (line->visual_width) {
                find_file_select();
            }
        }
    } else if (key == TAB) {
        find_file_run();
    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }

        array_zero_term(ys->cmd_buff);
        find_file_run();
    }
}

void find_file_make_buffer(void) {
    buff = yed_get_buffer("*find-file-list");

    if (!buff) {
        buff = yed_create_buffer("*find-file-list");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    ASSERT(buff, "did not create '*find-file-list' buffer");
}

void find_file_update_buff(void) {
    yed_buff_clear_no_undo(buff);
    yed_fill_buff_from_file(buff, "/tmp/find_file_list.yed");
}

void find_file_run(void) {
    char       cmd_buff[512], *cmd_p;
    yed_attrs  attr_cmd, attr_attn;
    char       attr_buff[128];
    char      *pattern;
    int        len, err;

    find_file_set_prompt("(find-file) ", NULL);

    cmd_buff[0] = 0;
    pattern     = array_data(ys->cmd_buff);

    if (strlen(pattern) == 0)     { goto empty; }

    strcat(cmd_buff, "bash -c '");

    cmd_p = cmd_buff + strlen(cmd_buff);

    len = perc_subst(prg, pattern, cmd_p, sizeof(cmd_buff) - strlen(cmd_buff));

    ASSERT(len > 0, "buff too small for perc_subst");

    strcat(cmd_buff, " 2>/dev/null > /tmp/find_file_list.yed && test ${PIPESTATUS[0]} -eq 0' 2>/dev/null");

    err = system(cmd_buff);

    if (err) {
        attr_cmd    = yed_active_style_get_command_line();
        attr_attn   = yed_active_style_get_attention();
        attr_cmd.fg = attr_attn.fg;
        yed_get_attr_str(attr_cmd, attr_buff);

        find_file_set_prompt("(find-file) ", attr_buff);
empty:
        yed_buff_clear_no_undo(buff);
    } else {
        find_file_update_buff();
    }
}

void find_file_select(void) {
    yed_line *line;
    char     *_path, *path;

    line = yed_buff_get_line(buff, ys->active_frame->cursor_line);
    array_zero_term(line->chars);
    _path = path = strdup(array_data(line->chars));

    if (strlen(path) >= 2) {
        if (*path == '.' && *(path + 1) == '/') {
            path += 2;
        }
    }


    YEXE("special-buffer-prepare-jump-focus", "*grep-list");
    YEXE("buffer", path);

    find_file_cleanup();
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

void find_file_set_prompt(char *p, char *attr) {
    prompt_buff[0] = 0;

    strcat(prompt_buff, p);

    if (attr) {
        strcat(prompt_buff, attr);
    }

    ys->cmd_prompt = prompt_buff;
}
