static int yed_term_has_exited;

#define TERM_DEFAULT_READ_TIMEOUT (3)

int yed_term_enter(void) {
    struct termios raw_term;

    yed_term_has_exited = 0;

/*     printf("[yed] entering raw terminal mode\n"); */

    tcgetattr(0, &ys->sav_term);
    raw_term = ys->sav_term;

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
    yed_register_sigstop_handler();
    yed_register_sigcont_handler();
    yed_register_sigterm_handler();
    yed_register_sigquit_handler();
    yed_register_sigstop_handler();
    yed_register_sigsegv_handler();
    yed_register_sigill_handler();
    yed_register_sigfpe_handler();
    yed_register_sigbus_handler();

    printf(TERM_ALT_SCREEN);
    printf(TERM_ENABLE_BRACKETED_PASTE);

    fflush(stdout);

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

    printf("\e[%d q", TERM_CURSOR_STYLE_DEFAULT);
    printf(TERM_DISABLE_BRACKETED_PASTE);
    printf(TERM_STD_SCREEN);
    printf(TERM_CURSOR_SHOW);

    tcsetattr(0, TCSAFLUSH, &ys->sav_term);

    yed_term_has_exited = 1;

    fflush(stdout);

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

void yed_set_cursor_style(int style) {
    switch (style) {
        case TERM_CURSOR_STYLE_DEFAULT:
        case TERM_CURSOR_STYLE_BLINKING_BLOCK:
        case TERM_CURSOR_STYLE_STEADY_BLOCK:
        case TERM_CURSOR_STYLE_BLINKING_UNDERLINE:
        case TERM_CURSOR_STYLE_STEADY_UNDERLINE:
        case TERM_CURSOR_STYLE_BLINKING_BAR:
        case TERM_CURSOR_STYLE_STEADY_BAR:
            append_to_output_buff("\e[");
            append_int_to_output_buff(style);
            append_to_output_buff(" q");
            break;
        default:;
    }
}

void sigwinch_handler(int sig) {
    yed_check_for_resize();
}

void sigstop_handler(int sig) {
    struct sigaction act;

    if (ys->stopped) { return; }

    /* Save the old handlers and install the default action handler. */
    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGTSTP, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    ys->stopped = 1;
    yed_term_exit();

    /* Do the real suspend */
    kill(0, SIGTSTP);
}

void sigcont_handler(int sig) {
    if (ys->stopped) {
        /* Restore our SIGTSTP handler. */
        yed_register_sigstop_handler();

        ys->stopped = 0;
        yed_term_enter();
        pthread_mutex_unlock(&ys->write_ready_mtx);
        ys->redraw = ys->redraw_cls = 1;
    }
}

void sigterm_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    /* Do the real terminate */
    kill(0, SIGTERM);
}

void sigquit_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGQUIT, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    /* Do the real quit */
    kill(0, SIGQUIT);
}

void print_fatal_signal_message_and_backtrace(char *sig_name) {
    printf("\n" TERM_RED "yed has received a fatal signal (%s).\n", sig_name);
#ifdef HAS_BACKTRACE
    printf("Here is a backtrace of its execution (most recent first):" TERM_RESET "\n\n");
    printf(TERM_BLUE);
    print_backtrace();
#endif
    printf(TERM_RESET);
    printf("\n");
    printf(TERM_GREEN);
    printf("Please create an issue at https://github.com/kammerdienerb/yed\n"
           "describing what happened.\n");
    printf(TERM_RESET);
    printf("\n");
}

void sigsegv_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGSEGV, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    print_fatal_signal_message_and_backtrace("SIGSEGV");

    /* Do the real signal */
    kill(0, SIGSEGV);
}

void sigill_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGILL, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    print_fatal_signal_message_and_backtrace("SIGILL");

    /* Do the real signal */
    kill(0, SIGILL);
}

void sigfpe_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGFPE, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    print_fatal_signal_message_and_backtrace("SIGFPE");

    /* Do the real signal */
    kill(0, SIGFPE);
}

void sigbus_handler(int sig) {
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaction(SIGBUS, &act, NULL);

    /* Stop the writer thread. */
    pthread_mutex_lock(&ys->write_ready_mtx);

    /* Exit the terminal. */
    yed_term_exit();

    print_fatal_signal_message_and_backtrace("SIGBUS");

    /* Do the real signal */
    kill(0, SIGBUS);
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

void yed_register_sigstop_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigstop_handler;
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGTSTP");
    }
}

void yed_register_sigcont_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigcont_handler;
    if (sigaction(SIGCONT, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGCONT");
    }
}

void yed_register_sigterm_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGTERM");
    }
}

void yed_register_sigquit_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigquit_handler;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGQUIT");
    }
}

void yed_register_sigsegv_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigsegv_handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGSEGV");
    }
}

void yed_register_sigill_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigill_handler;
    if (sigaction(SIGILL, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGILL");
    }
}

void yed_register_sigfpe_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigfpe_handler;
    if (sigaction(SIGFPE, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGFPE");
    }
}

void yed_register_sigbus_handler(void) {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = 0;
    sa.sa_handler = sigbus_handler;
    if (sigaction(SIGBUS, &sa, NULL) == -1) {
        ASSERT(0, "sigaction failed for SIGBUS");
    }
}

int yed_check_for_resize(void) {
    int ret;

    ret = 0;

    yed_term_get_dim(&ys->new_term_rows, &ys->new_term_cols);

    if (ys->new_term_rows != ys->term_rows
    ||  ys->new_term_cols != ys->term_cols) {

        ret = 1;
    }

    ys->has_resized = ret;

    return ret;
}

void yed_handle_resize(void) {
    yed_frame **frame_it;
    int save_row, save_col;
    yed_frame *af;
    yed_event  event;

    if (!ys->has_resized) {
        return;
    }

    ys->written_cells = realloc(ys->written_cells, ys->new_term_rows * ys->new_term_cols);
    memset(ys->written_cells, 0, ys->new_term_rows * ys->new_term_cols);

    ys->term_cols = ys->new_term_cols;
    ys->term_rows = ys->new_term_rows;

    af = ys->active_frame;
    if (af) {
        save_row = af->cursor_line;
        save_col = af->cursor_col;
        yed_set_cursor_far_within_frame(af, 1, 1);
    }

    /*
     * Should keep this just in case someone is naughty and the frame
     * doesn't have a tree.
     */
    array_traverse(ys->frames, frame_it) {
        FRAME_RESET_RECT(*frame_it);
    }

    array_traverse(ys->frames, frame_it) {
        if ((*frame_it)->tree) {
            yed_frame_tree_recursive_readjust((*frame_it)->tree);
        }
    }

    ys->redraw = ys->redraw_cls = 1;
    yed_clear_screen();

    if (af) {
        yed_set_cursor_far_within_frame(af, save_row, save_col);
    }

#if 0
    yed_update_frames();
    write_status_bar(0);
    yed_draw_command_line();
#endif

    ys->has_resized = 0;

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_TERMINAL_RESIZED;
    yed_trigger_event(&event);
}
