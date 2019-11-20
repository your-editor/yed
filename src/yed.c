#include "internal.h"

#include "internal.c"
#include "array.c"
#include "bucket_array.c"
#include "term.c"
#include "key.c"
#include "buffer.c"
#include "fs.c"
#include "frame.c"
#include "command.c"
#include "getRSS.c"
#include "event.c"
#include "plugin.c"
#include "find.c"
#include "var.c"
#include "util.c"
#include "style.c"
#include "subproc.c"
#include "undo.c"

yed_state *ys;

static void write_welcome(void) {
    char *msg1;
    char *msg2;

    msg1 = "This is Your Editor.";
    msg2 = "Hit <ctrl-f> to enter a command.";
    yed_set_cursor((ys->term_cols / 2) - (strlen(msg1) / 2), ys->term_rows / 2);
    append_to_output_buff(msg1);
    yed_set_cursor((ys->term_cols / 2) - (strlen(msg2) / 2), (ys->term_rows / 2) + 1);
    append_to_output_buff(msg2);
}

static void * writer(void *arg) {
    int status;

    (void)arg;

    while (1) {
        pthread_mutex_lock(&ys->write_ready_mtx);
        pthread_mutex_lock(&ys->write_mtx);
        flush_writer_buff();
        ys->writer_done = 1;
        status = ys->status;
        pthread_cond_signal(&ys->write_signal);
        pthread_mutex_unlock(&ys->write_mtx);

        if (status == YED_RELOAD) {
            break;
        }
    }

    return NULL;
}

static void kill_writer(void) {
    void *junk;

    /*
     * Wait for the writer thread to signal that it
     * is finished writing the previous update.
     */
    pthread_mutex_lock(&ys->write_mtx);
    while (!ys->writer_done) {
        pthread_cond_wait(&ys->write_signal, &ys->write_mtx);
    }

    /*
     * Let the writer thread continue.
     * This time, ys->status = YED_RELOAD, so the
     * writer thread will break its loop and leave
     * ys->write_mtx unlocked.
     */
    pthread_mutex_unlock(&ys->write_mtx);
    pthread_mutex_unlock(&ys->write_ready_mtx);

    /*
     * We will wait here until the writer thread has
     * exited.
     */
     pthread_join(ys->writer_id, &junk);
}

static void restart_writer(void) {
    pthread_create(&ys->writer_id, NULL, writer, NULL);
}

yed_state * yed_init(yed_lib_t *yed_lib, int argc, char **argv) {
    int i;

    ys = malloc(sizeof(*ys));
    memset(ys, 0, sizeof(*ys));

    ys->yed_lib          = yed_lib;

    yed_init_vars();
    yed_init_styles();
    yed_init_buffers();
    yed_init_frames();

/*     ys->small_message = "* started yed *"; */

    yed_term_enter();
    yed_term_get_dim(&ys->term_rows, &ys->term_cols);

    ys->written_cells = malloc(ys->term_rows * ys->term_cols);

    memset(ys->_4096_spaces, ' ', 4096);
    yed_init_output_stream();
    pthread_mutex_init(&ys->write_mtx, NULL);
    pthread_mutex_init(&ys->write_ready_mtx, NULL);
    pthread_mutex_lock(&ys->write_ready_mtx);
    pthread_cond_init(&ys->write_signal, NULL);

    pthread_create(&ys->writer_id, NULL, writer, NULL);
    yed_init_commands();
    yed_init_keys();
    yed_init_events();
    yed_init_search();
    yed_init_plugins();

    (void)i;
    if (argc > 1) {
        YEXE("frame-new");
        YEXE("buffer", argv[1]);

        for (i = 2; i < argc; i += 1) {
            YEXE("frame-vsplit");
            YEXE("buffer", argv[i]);
        }

        if (argc > 2) {
            YEXE("frame-next");
        }

        yed_update_frames();
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else {
        append_to_output_buff(TERM_CURSOR_HIDE);
        yed_clear_screen();
        yed_cursor_home();
        write_welcome();
    }

    ys->redraw = 1;

    pthread_mutex_unlock(&ys->write_ready_mtx);

    return ys;
}

void yed_fini(yed_state *state) {
    char *bytes;

    printf(TERM_RESET);
    yed_term_exit();

    free(state);

    bytes = pretty_bytes(getPeakRSS());

    printf("Peak RSS: %s\nThanks for using yed!\n", bytes);

    free(bytes);
}

void yed_set_state(yed_state *state)    { ys = state; }
yed_state * yed_get_state(void)         { return ys;  }

static void write_small_message(void) {
    int sav_x, sav_y;

    if (!ys->small_message) {
        return;
    }

    sav_x = ys->cur_x;
    sav_y = ys->cur_y;
    yed_set_cursor((ys->term_cols / 2) - (strlen(ys->small_message) / 2), ys->term_rows);
    append_to_output_buff(ys->small_message);

    yed_set_cursor(sav_x, sav_y);
}

static void write_cursor_loc_and_key(int key) {
    int sav_x, sav_y;

    sav_x = ys->cur_x;
    sav_y = ys->cur_y;

    if (ys->active_frame) {
        yed_set_cursor(ys->term_cols - 20, ys->term_rows);
        append_n_to_output_buff("                    ", 20);
        yed_set_cursor(ys->term_cols - 20, ys->term_rows);
        append_int_to_output_buff(ys->active_frame->cursor_line);
        append_to_output_buff(" :: ");
        append_int_to_output_buff(ys->active_frame->cursor_col);
    }

    yed_set_cursor(ys->term_cols - 5, ys->term_rows);
    append_n_to_output_buff("     ", 5);
    yed_set_cursor(ys->term_cols - 5, ys->term_rows);
    append_int_to_output_buff(key);
    yed_set_cursor(sav_x, sav_y);
}

int yed_pump(void) {
    int   keys[16], n_keys, i;

    if (ys->status == YED_RELOAD) {
        yed_service_reload();
        restart_writer();
    }

    write_small_message();

    /* Not sure why this is necessary, but... */
    if (!ys->interactive_command && ys->active_frame) {
        yed_set_cursor(ys->active_frame->cur_x, ys->active_frame->cur_y);
    }

    ys->status = YED_NORMAL;

    memset(ys->written_cells, 0, ys->term_rows * ys->term_cols);

    /*
     * Wait for the writer thread to signal that it
     * is finished writing the previous update.
     */
    pthread_mutex_lock(&ys->write_mtx);
    while (!ys->writer_done) {
        pthread_cond_wait(&ys->write_signal, &ys->write_mtx);
    }

    /*
     * Give the writer thread the new screen update.
     */
    ys->writer_done = 0;
    array_copy(ys->writer_buffer, ys->output_buffer);
    array_clear(ys->output_buffer);
    pthread_mutex_unlock(&ys->write_mtx);

    /*
     * Signal the writer thread to go ahead and start writing.
     */
    pthread_mutex_unlock(&ys->write_ready_mtx);

    append_to_output_buff(TERM_CURSOR_HIDE);

    n_keys = yed_read_keys(keys);

    for (i = 0; i < n_keys; i += 1) {
        yed_take_key(keys[i]);
    }

    if (ys->redraw) {
        if (yed_check_for_resize()) {
            yed_handle_resize();
        }
    }

    yed_update_frames();

    ys->redraw = 0;

    if (ys->interactive_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_RESET);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        write_cursor_loc_and_key(keys[0]);
        append_to_output_buff(TERM_RESET);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else {
        append_to_output_buff(TERM_CURSOR_HIDE);
        append_to_output_buff(TERM_RESET);
    }

    if (ys->status == YED_RELOAD) {
        yed_unload_plugin_libs();
        kill_writer();
    }

    return ys->status;
}
