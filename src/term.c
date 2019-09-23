#include "internal.h"


static int yed_term_enter(void) {
    struct termios raw_term;

    tcgetattr(0, &ys->sav_term);
    raw_term          = ys->sav_term;
    raw_term.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(0, TCSAFLUSH, &raw_term);

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[yed] entering raw terminal mode\n");

    printf(TERM_ALT_SCREEN);

	return 0;
}

static int yed_term_exit(void) {
    printf(TERM_STD_SCREEN);

    tcsetattr(0, TCSAFLUSH, &ys->sav_term);

    printf("[yed] exited raw terminal mode\n");

	return 0;
}

static int yed_term_get_dim(int *r, int *c) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *c = ws.ws_col;
        *r = ws.ws_row;
        return 0;
    }
}

static void yed_clear_screen(void) {
    append_to_output_buff(TERM_CLEAR_SCREEN);
}

static void yed_cursor_home(void) {
    append_to_output_buff(TERM_CURSOR_HOME);

    ys->cur_y = ys->cur_x = 1;
}

static void yed_set_cursor(int col, int row) {

    if (col < 1)    { col = 1; }
    if (row < 1)    { row = 1; }

    ys->cur_x = col;
    ys->cur_y = row;

    append_to_output_buff(TERM_CURSOR_MOVE_BEG);
    append_int_to_output_buff(row);
    append_to_output_buff(TERM_CURSOR_MOVE_SEP);
    append_int_to_output_buff(col);
    append_to_output_buff(TERM_CURSOR_MOVE_END);
}
