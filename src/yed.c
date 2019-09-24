#include "internal.h"

#include "internal.c"
#include "array.c"
#include "term.c"
#include "key.c"
#include "buffer.c"
#include "frame.c"
#include "command.c"

static void write_welcome(void) {
    const char *msg1;
    const char *msg2;

    msg1 = "This is Your Editor.";
    msg2 = "Hit <ctrl-f> to enter a command.";
    yed_set_cursor((ys->term_cols / 2) - (strlen(msg1) / 2), ys->term_rows / 2);
    append_to_output_buff(msg1);
    yed_set_cursor((ys->term_cols / 2) - (strlen(msg2) / 2), (ys->term_rows / 2) + 1);
    append_to_output_buff(msg2);
}

yed_state * yed_init(int argc, char **argv) {
    ys = malloc(sizeof(*ys));
    memset(ys, 0, sizeof(*ys));

    yed_init_output_stream();
    yed_init_commands();
    yed_init_frames();

    yed_term_enter();
    yed_term_get_dim(&ys->term_rows, &ys->term_cols);

    append_to_output_buff(TERM_CURSOR_HIDE);

    yed_clear_screen();
    write_welcome();
    yed_cursor_home();

    return ys;
}

void yed_fini(yed_state *state) {
    printf(TERM_RESET);
    yed_term_exit();

    free(state);

    printf(TERM_CURSOR_HIDE
           TERM_CLEAR_SCREEN
           TERM_CURSOR_HOME
           TERM_CURSOR_SHOW
           "Thanks for using yed!\n");
}

void yed_set_state(yed_state *state)    { ys = state; }
yed_state * yed_get_state(void)         { return ys;  }

int yed_pump(void) {
    int   key;
    char *key_str;
    char  key_str_buff[2];

    flush_output_buff();

    if (ys->status == YED_RELOAD) {
        yed_service_reload();
    }

    ys->status = YED_NORMAL;

    append_to_output_buff(TERM_CURSOR_HIDE);

    key = yed_read_key();

    if (ys->accepting_command) {
        key_str_buff[0] = (char)key;
        key_str_buff[1] = 0;
        key_str         = key_str_buff;
        yed_execute_command("command-prompt", 1, &key_str);
    } else {
        if (key == CTRL('f')) {
            yed_execute_command("command-prompt", 0, NULL);
        } else if (ys->active_frame) {
            yed_frame_take_key(ys->active_frame, key);
        }
    }

    yed_update_frames();

    if (ys->accepting_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        append_to_output_buff(TERM_CURSOR_SHOW);
    }

    append_to_output_buff(TERM_RESET);

    return ys->status;
}
