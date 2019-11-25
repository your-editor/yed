#include "plugin.h"

void find_file(int n_args, char **args);
void find_file_start(void);
void find_file_cleanup(void);
void find_file_take_key(int key);
void find_file_make_frame(void);
void find_file_make_buffer(void);
void find_file_run(void);
void find_file_update_buff(void);
void find_file_select(void);

void find_file_key_pressed_handler(yed_event *event);

static char       *prg;
static yed_frame  *frame;
static yed_buffer *buff;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_KEY_PRESSED;
    h.fn   = find_file_key_pressed_handler;

    yed_plugin_add_event_handler(self, h);
    yed_plugin_set_command(self, "find-file", find_file);

    if (!yed_get_var("find-file-prg")) {
        yed_set_var("find-file-prg", "find . -path ./.git -prune -o -type f -name");
    }

    return 0;
}

void find_file(int n_args, char **args) {
    int key;

    if (!ys->interactive_command) {
        prg = yed_get_var("find-file-prg");
        if (!prg) {
            yed_append_text_to_cmd_buff("[!] 'find-file-prg' not set");
            return;
        }
        find_file_start();
    } else {
        sscanf(args[0], "%d", &key);
        find_file_take_key(key);
    }
}

void find_file_cleanup(void) {
    yed_frame_set_buff(frame, NULL);
    yed_free_buffer(buff);
    yed_delete_frame(frame);

    frame = NULL;
    buff  = NULL;
}

void find_file_start(void) {
    ys->interactive_command = "find-file";
    ys->cmd_prompt          = "(find-file) ";
    yed_set_small_message(NULL);
    find_file_make_frame();
    find_file_make_buffer();
    yed_frame_set_buff(frame, buff);
    yed_clear_cmd_buff();
}

void find_file_take_key(int key) {
    yed_line *line;

    if (key == CTRL_C) {
        ys->interactive_command = NULL;
        ys->current_search      = NULL;
        yed_clear_cmd_buff();
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
    buff = yed_get_buffer("*find_filelist");

    if (!buff) {
        buff = yed_create_buffer("*find-file-list");
        buff->flags |= BUFF_RD_ONLY;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    ASSERT(buff, "did not create '*find-file-list' buffer");
}

void find_file_make_frame(void) {
    frame = yed_add_new_frame(0.15, 0.15, 0.7, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void find_file_update_buff(void) {
    yed_buff_clear_no_undo(buff);
    yed_fill_buff_from_file(buff, "/tmp/find_file_list.yed");
}

void find_file_run(void) {
    char  cmd_buff[256];
    char *pattern;
    int   err;

    ys->cmd_prompt = "(find-file) ";

    cmd_buff[0] = 0;
    pattern = array_data(ys->cmd_buff);

    if (strlen(pattern) == 0)     { goto empty; }

    strcat(cmd_buff, "bash -c '");
    strcat(cmd_buff, prg);
    strcat(cmd_buff, " \"*");
    strcat(cmd_buff, pattern);
    strcat(cmd_buff, "*\" -print 2>/dev/null > /tmp/find_file_list.yed && test ${PIPESTATUS[0]} -eq 0' 2>/dev/null");

    err = system(cmd_buff);

    if (err) {
        /* This is a little bit dirty. */
        ys->cmd_prompt = "(find-file) "TERM_RED;
empty:
        yed_buff_clear_no_undo(buff);
    } else {
        find_file_update_buff();
    }
}

void find_file_select(void) {
    yed_line *line;
    char     *path;

    line = yed_buff_get_line(buff, frame->cursor_line);
    array_zero_term(line->chars);
    path = array_data(line->chars);

    if (strlen(path) >= 2) {
        if (*path == '.' && *(path + 1) == '/') {
            path += 2;
        }
    }

    find_file_cleanup();

    if (!ys->active_frame) {
        YEXE("frame-new");
    }

    YEXE("buffer", path);
}

void find_file_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                                /* not the key we want */
    ||  ys->interactive_command                            /* still typing */
    ||  !eframe                                            /* no frame */
    ||  eframe != frame                                    /* not our frame */
    ||  !eframe->buffer                                    /* no buffer */
    ||  strcmp(eframe->buffer->name, "*find-file-list")) { /* not our buffer */
        return;
    }

    find_file_select();
}
