#include "internal.h"

#include "internal.c"
#include "array.c"
#include "bucket_array.c"
#include "term.c"
#include "key.c"
#include "buffer.c"
#include "frame.c"
#include "command.c"
#include "getRSS.c"

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
    int i;

    ys = malloc(sizeof(*ys));
    memset(ys, 0, sizeof(*ys));

    ys->buff_list = array_make(yed_buffer*);

    yed_init_output_stream();
    yed_init_commands();
    yed_init_frames();

    yed_term_enter();
    yed_term_get_dim(&ys->term_rows, &ys->term_cols);

    if (argc > 1) {
        yed_execute_command("frame-new-file", 1, &argv[1]);

        for (i = 2; i < argc; i += 1) {
            yed_execute_command("frame-split-new-file", 1, &argv[i]);
        }

        yed_execute_command("frame", 1, &argv[1]);
        yed_update_frames();
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else {
        append_to_output_buff(TERM_CURSOR_HIDE);
        yed_clear_screen();
        yed_cursor_home();
        write_welcome();
    }

    return ys;
}

void yed_fini(yed_state *state) {
    char *bytes;

    printf(TERM_RESET);
    yed_term_exit();

    free(state);

    bytes = pretty_bytes(getPeakRSS());

    printf(TERM_CURSOR_HIDE
           TERM_CLEAR_SCREEN
           TERM_CURSOR_HOME
           TERM_CURSOR_SHOW
           "Peak RSS: %s\nThanks for using yed!\n", bytes);

    free(bytes);
}

void yed_set_state(yed_state *state)    { ys = state; }
yed_state * yed_get_state(void)         { return ys;  }

int yed_pump(void) {
    int   keys[16], n_keys, i, sav_x, sav_y;

    flush_output_buff();

    if (ys->status == YED_RELOAD) {
        yed_service_reload();
    }

    ys->status = YED_NORMAL;

    append_to_output_buff(TERM_CURSOR_HIDE);

    n_keys = yed_read_keys(keys);

    for (i = 0; i < n_keys; i += 1) {
        yed_take_key(keys[i]);
    }

    yed_update_frames();

    if (ys->accepting_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        sav_x = ys->cur_x;
        sav_y = ys->cur_y;
        yed_set_cursor(ys->term_cols - 5, ys->term_rows);
        append_n_to_output_buff("     ", 5);
        yed_set_cursor(ys->term_cols - 5, ys->term_rows);
        append_int_to_output_buff(keys[0]);
        yed_set_cursor(sav_x, sav_y);

        append_to_output_buff(TERM_CURSOR_SHOW);
    }

    append_to_output_buff(TERM_RESET);

    return ys->status;
}
