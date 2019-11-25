#include "plugin.h"

void grep(int n_args, char **args);
void grep_start(void);
void grep_cleanup(void);
void grep_take_key(int key);
void grep_make_frame(void);
void grep_make_buffer(void);
void grep_run(void);
void grep_update_buff(void);
void grep_select(void);

void grep_key_pressed_handler(yed_event *event);

static char       *prg;
static yed_frame  *frame;
static yed_buffer *buff;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = grep_key_pressed_handler;

    yed_plugin_add_event_handler(self, h);
    yed_plugin_set_command(self, "grep", grep);

    if (!yed_get_var("grep-prg")) {
        yed_set_var("grep-prg", "grep --exclude-dir={.git} -RHnIs");
    }

    return 0;
}

void grep(int n_args, char **args) {
    int key;

    if (!ys->interactive_command) {
        prg = yed_get_var("grep-prg");
        if (!prg) {
            yed_append_text_to_cmd_buff("[!] 'grep-prg' not set");
            return;
        }
        grep_start();
    } else {
        sscanf(args[0], "%d", &key);
        grep_take_key(key);
    }
}

void grep_cleanup(void) {
    yed_frame_set_buff(frame, NULL);
    yed_free_buffer(buff);
    yed_delete_frame(frame);

    frame = NULL;
    buff  = NULL;
}

void grep_start(void) {
    ys->interactive_command = "grep";
    ys->cmd_prompt          = "(grep) ";
    yed_set_small_message(NULL);
    grep_make_frame();
    grep_make_buffer();
    yed_frame_set_buff(frame, buff);
    yed_clear_cmd_buff();
}

void grep_take_key(int key) {
    if (key == CTRL_C) {
        ys->interactive_command = NULL;
        ys->current_search      = NULL;
        yed_clear_cmd_buff();
        grep_cleanup();
    } else if (key == ENTER) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
    } else if (key == TAB) {
        grep_run();
    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }

        array_zero_term(ys->cmd_buff);
        grep_run();
    }
}

void grep_make_buffer(void) {
    buff = yed_get_buffer("*grep-list");

    if (!buff) {
        buff = yed_create_buffer("*grep-list");
        buff->flags |= BUFF_RD_ONLY;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    ASSERT(buff, "did not create '*grep-list' buffer");
}

void grep_make_frame(void) {
    frame = yed_add_new_frame(0.15, 0.15, 0.7, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void grep_update_buff(void) {
    yed_buff_clear_no_undo(buff);
    yed_fill_buff_from_file(buff, "/tmp/grep-list.yed");
}

void grep_run(void) {
    char  cmd_buff[256];
    char *pattern;
    int   err;

    ys->cmd_prompt = "(grep) ";

    cmd_buff[0] = 0;
    pattern = array_data(ys->cmd_buff);

    if (strlen(pattern) == 0)     { goto empty; }

    strcat(cmd_buff, "bash -c '");
    strcat(cmd_buff, prg);
    strcat(cmd_buff, " \"");
    strcat(cmd_buff, pattern);
    strcat(cmd_buff, "\" . 2>/dev/null > /tmp/grep-list.yed && test ${PIPESTATUS[0]} -eq 0' 2>/dev/null");

    err = system(cmd_buff);

    if (err) {
        /* This is a little bit dirty. */
        ys->cmd_prompt = "(grep) "TERM_RED;
empty:
        yed_buff_clear_no_undo(buff);
    } else {
        grep_update_buff();
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
    line = yed_buff_get_line(buff, frame->cursor_line);
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

    grep_cleanup();

    if (!ys->active_frame) {
        YEXE("frame-new");
    }

    YEXE("buffer", path);

    yed_set_cursor_within_frame(ys->active_frame, 1, row);
}

void grep_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                          /* not the key we want */
    ||  ys->interactive_command                      /* still typing */
    ||  !eframe                                      /* no frame */
    ||  eframe != frame                              /* not our frame */
    ||  !eframe->buffer                              /* no buffer */
    ||  strcmp(eframe->buffer->name, "*grep-list")) { /* not our buffer */
        return;
    }

    grep_select();
}
