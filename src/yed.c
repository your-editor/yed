#include "internal.h"

#include "internal.c"
#include "array.c"
#include "term.c"
#include "key.c"
#include "buffer.c"
#include "frame.c"
#include "command.c"


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
    yed_cursor_home();

    append_to_output_buff(TERM_CURSOR_SHOW);

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
    int key;

    flush_output_buff();

    if (ys->status == YED_RELOAD) {
        yed_service_reload();
    }

    key = yed_read_key();

    ys->status = YED_NORMAL;

    append_to_output_buff(TERM_CURSOR_HIDE);

    if (ys->accepting_command) {
        yed_command_take_key(key);
    } else {
        if (key == CTRL('f')) {
            ys->save_cur_y = ys->cur_y;
            ys->save_cur_x = ys->cur_x;
            yed_command_prompt();
        } else if (ys->active_frame) {
            yed_frame_take_key(ys->active_frame, key);
            yed_update_frames();
        }
    }

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_SHOW);

    return ys->status;
}
