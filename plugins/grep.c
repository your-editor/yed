#include <yed/plugin.h>

void grep(int n_args, char **args);
void grep_start(void);
void grep_cleanup(void);
void grep_take_key(int key);
void grep_run(void);
void grep_select(void);

void grep_key_pressed_handler(yed_event *event);

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*grep-list");

    if (buff == NULL) {
        buff = yed_create_buffer("*grep-list");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}

static char *prg;
static char *save_current_search;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = grep_key_pressed_handler;

    yed_plugin_add_event_handler(self, h);
    yed_plugin_set_command(self, "grep", grep);

    if (!yed_get_var("grep-prg")) {
        yed_set_var("grep-prg", "grep --exclude-dir={.git} -RHnIs '%' .");
    }

    return 0;
}

void grep(int n_args, char **args) {
    int i;
    int key;

    if (!ys->interactive_command) {
        prg = yed_get_var("grep-prg");
        if (!prg) {
            yed_cerr("'grep-prg' not set");
            return;
        }
        grep_start();
        if (n_args) {
            for (i = 0; i < strlen(args[0]); i += 1) {
                yed_cmd_line_readline_take_key(NULL, (int)args[0][i]);
            }
            array_zero_term(ys->cmd_buff);
            grep_run();
            ys->interactive_command = NULL;
            yed_clear_cmd_buff();
        }
    } else {
        sscanf(args[0], "%d", &key);
        grep_take_key(key);
    }
}

void grep_cleanup(void) {
    ys->save_search = save_current_search;
}

void grep_start(void) {
    ys->interactive_command = "grep";
    ys->cmd_prompt          = "(grep) ";
    save_current_search = ys->current_search;

    yed_buff_clear_no_undo(get_or_make_buff());
    YEXE("special-buffer-prepare-focus", "*grep-list");
    yed_frame_set_buff(ys->active_frame, get_or_make_buff());
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    yed_clear_cmd_buff();
}

void grep_take_key(int key) {
    switch (key) {
        case ESC:
        case CTRL_C:
            ys->interactive_command = NULL;
            ys->current_search      = NULL;
            yed_clear_cmd_buff();
            YEXE("special-buffer-prepare-unfocus", "*grep-list");
            grep_cleanup();
            break;
        case ENTER:
            ys->interactive_command = NULL;
            ys->current_search      = NULL;
            ys->active_frame->dirty = 1;
            yed_clear_cmd_buff();
            break;
        default:
            yed_cmd_line_readline_take_key(NULL, key);
            grep_run();
            break;
    }
}

void grep_run(void) {
    char       cmd_buff[1024];
    char      *pattern;
    int        len, status;

    array_zero_term(ys->cmd_buff);

    cmd_buff[0]        = 0;
    pattern            = array_data(ys->cmd_buff);
    ys->current_search = pattern;

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

void grep_select(void) {
    yed_line *line;
    char      _path[256];
    char     *path,
             *c;
    int       row,
              row_idx;

    path = _path;
    line = yed_buff_get_line(get_or_make_buff(), ys->active_frame->cursor_line);
    array_zero_term(line->chars);

    row_idx = 0;
    array_traverse(line->chars, c) {
        row_idx += 1;
        if (*c == ':') {
            break;
        }
        *path++ = *c;
    }
    *path = 0;
    path  = _path;

    if (row_idx >= 2) {
        if (*path == '.' && *(path + 1) == '/') {
            path += 2;
        }
    }

    sscanf(array_data(line->chars) + row_idx, "%d", &row);

    YEXE("special-buffer-prepare-jump-focus", "*grep-list");
    YEXE("buffer", path);
    yed_set_cursor_within_frame(ys->active_frame, row, 1);
    grep_cleanup();
}

void grep_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                           /* not the key we want */
    ||  ys->interactive_command                       /* still typing        */
    ||  !eframe                                       /* no frame            */
    ||  !eframe->buffer                               /* no buffer           */
    ||  strcmp(eframe->buffer->name, "*grep-list")) { /* not our buffer      */
        return;
    }

    grep_select();

    event->cancel = 1;
}
