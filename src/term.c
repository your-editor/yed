static int yed_term_has_exited;

#define TERM_DEFAULT_READ_TIMEOUT (3)

int yed_term_enter(void) {
    struct termios raw_term;

/*     printf("[yed] entering raw terminal mode\n"); */

    tcgetattr(0, &ys->sav_term);
    raw_term          = ys->sav_term;

    raw_term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    /* raw_term.c_oflag &= ~(OPOST); */
    /* control modes - set 8 bit chars */
    raw_term.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     *      * no signal chars (^Z,^C) */
    raw_term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);


    /* control chars - set return condition: min number of bytes and timer. */

    /* Return each byte, or zero for timeout. */
    raw_term.c_cc[VMIN] = 0;
    /* 300 ms timeout (unit is tens of second). */
    raw_term.c_cc[VTIME] = TERM_DEFAULT_READ_TIMEOUT;

    tcsetattr(0, TCSAFLUSH, &raw_term);

    setvbuf(stdout, NULL, _IONBF, 0);

    yed_register_sigwinch_handler();

    printf(TERM_ALT_SCREEN);

    return 0;
}

void yed_term_timeout_on(void) {
    struct termios raw_term;

    tcgetattr(0, &raw_term);
    raw_term.c_cc[VMIN] = 0;
    tcsetattr(0, TCSAFLUSH, &raw_term);
}

void yed_term_timeout_off(void) {
    struct termios raw_term;

    tcgetattr(0, &raw_term);
    raw_term.c_cc[VMIN] = 1;
    tcsetattr(0, TCSAFLUSH, &raw_term);
}

void yed_term_set_timeout(int n_x_100_ms) {
    struct termios raw_term;

    tcgetattr(0, &raw_term);
    raw_term.c_cc[VTIME] = n_x_100_ms;
    tcsetattr(0, TCSAFLUSH, &raw_term);
}

int yed_term_exit(void) {
    if (yed_term_has_exited) {
        return 0;
    }

    printf(TERM_STD_SCREEN);

    tcsetattr(0, TCSAFLUSH, &ys->sav_term);

/*     printf("[yed] exited raw terminal mode\n"); */

    yed_term_has_exited = 1;

    return 0;
}

int yed_term_get_dim(int *r, int *c) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *c = ws.ws_col;
        *r = ws.ws_row;
        return 0;
    }
}

int yed_term_says_it_supports_truecolor(void) {
    char *colorterm;

    colorterm = getenv("COLORTERM");
    if (!colorterm) { return 0; }

    if (strcmp(colorterm, "truecolor")
    &&  strcmp(colorterm, "24bit")) {
        return 0;
    }

    return 1;
}

void yed_term_set_fg_rgb(int r, int g, int b) {
    char buff[128];

    buff[0] = 0;

    sprintf(buff, "\e[38;2;%d;%d;%dm", r, g, b);

    append_to_output_buff(buff);
}

void yed_term_set_bg_rgb(int r, int g, int b) {
    char buff[128];

    buff[0] = 0;

    sprintf(buff, "\e[48;2;%d;%d;%dm", r, g, b);

    append_to_output_buff(buff);
}

void yed_term_set_rgb(int fr, int fg, int fb, int br, int bg, int bb) {
    char buff[128];

    buff[0] = 0;

    sprintf(buff, "\e[38;2;%d;%d;%d;48;2;%d;%d;%dm", fr, fg, fb, br, bg, bb);

    append_to_output_buff(buff);
}

void yed_clear_screen(void) {
    append_to_output_buff(TERM_CLEAR_SCREEN);
}

void yed_cursor_home(void) {
    append_to_output_buff(TERM_CURSOR_HOME);

    ys->cur_y = ys->cur_x = 1;
}

void yed_set_cursor(int col, int row) {

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

void sigwinch_handler(int sig) {
    if (pthread_mutex_trylock(&ys->write_mtx) == 0) {
        if (yed_check_for_resize()) {
            yed_handle_resize();
        }
        pthread_mutex_unlock(&ys->write_mtx);
    }
}

void yed_register_sigwinch_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigwinch_handler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGWINCH");
    }
}

int yed_check_for_resize(void) {
    int       save_rows, save_cols;
    yed_event event;

    save_rows = ys->term_rows;
    save_cols = ys->term_cols;

    yed_term_get_dim(&ys->term_rows, &ys->term_cols);

    if (ys->term_rows != save_rows
    ||  ys->term_cols != save_cols) {

        event.kind = EVENT_TERMINAL_RESIZED;
        yed_trigger_event(&event);

        return 1;
    }

    return 0;
}

void yed_handle_resize(void) {
    yed_frame **frame_it;
    int save_row, save_col;
    yed_frame *af;

    free(ys->written_cells);
    ys->written_cells = malloc(ys->term_rows * ys->term_cols);
    memset(ys->written_cells, 0, ys->term_rows * ys->term_cols);

    af = ys->active_frame;
    if (af) {
        save_row = af->cursor_line;
        save_col = af->cursor_col;
        yed_set_cursor_far_within_frame(af, 1, 1);
    }

    array_traverse(ys->frames, frame_it) {
        FRAME_RESET_RECT(*frame_it);
    }

    ys->redraw = 1;
    yed_clear_screen();

    if (af) {
        yed_set_cursor_far_within_frame(af, save_col, save_row);
    }

    yed_update_frames();
    yed_draw_command_line();
    write_status_bar(0);
}
