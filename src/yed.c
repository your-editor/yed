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
    int   i, n_oct_lines, oct_width, l;
    char *oct[] = {
"                                                           \n",
"                                                        //(\n",
"                                                      //   \n",
"                   This is Your Editor.              %/    \n",
"                                                     //    \n",
"                                                    //,    \n",
"                               %                    /#     \n",
"                               */#(     //***/*/*  /*/     \n",
"                       &(//////% ,/**//(       (//*,       \n",
"                      //////////*(              /*/*       \n",
"                     (//(///*////*(           ***/ *(      \n",
"                    ((///////*////#        **/*(  (/(      \n",
"                    #////////*///(#   /(****#    /**       \n",
"                    (///**/*****%##***(#/       (*/(       \n",
"                    ./////***(#%/%((,         (/*/         \n",
"                     %///***(((%/#*       /*/**#           \n",
"                     (#//***///(%*(///**/(((*              \n",
"                       //*//*//#(////((%#(%(%              \n",
"                  %&((//**/**************/**/***(          \n",
"              *(///###///***********//(((###///***/#       \n",
"           /////(((///((//**/#/***//*//        /#****(     \n",
"        //(//##(///#    #/*/( /#////#//*/         (****#   \n",
"      //((#%((((//      (///    (//*( %(/*&         ****   \n",
"     (/((&  #(/(.       ((//    (/**/   #/*(         ***/  \n",
"     /((#   #///       (///*   ,*/**(    (/*#        ****  \n",
"     ((%    (/(*      ////(    (/*/*      (/*(      (**/   \n",
"     ((#    (((      (///      ///*        ***     (***    \n",
"     #((    ((/%    ((//      (///         //*/    /**.    \n",
"     .(((    ((/   #(/(      */*(          ///     ***     \n",
"      ((/    #((( (((/(     ///(           */*     **(     \n",
"      #/%     ((( (//*      */*            //*     ***/    \n",
"      (/       /((#(/*      /**            (/*(     .***(  \n",
"     %(%       //(,////     //*/           (*/*         (/(\n",
"    #((         // #(/*      */*(           #**/           \n",
"  (#(&          (/  ////      (//(*           *//&         \n",
"  .             //   //*/       #//*#(&         (**(/      \n",
"               ,/(    /**           #//*/#           //%   \n",
"               //     */*(              //*(/          **  \n",
"              //*      (//                *//#             \n",
"             //         //(                 *//            \n",
"           *(/          ///                  *(            \n",
"          .             #//                  /(            \n",
"                        #//                  ((            \n",
"                        (#                   /(            \n",
"                        (                    (             \n",
"                       ((                                  \n"
    };

    l = 4;

    n_oct_lines = sizeof(oct) / sizeof(char*);
    oct_width   = strlen(oct[0]);

    for (i = 0; i < n_oct_lines; i += 1) {
        if (l + i == ys->term_rows - 1) {
            break;
        }
        yed_set_cursor((ys->term_cols / 2) - (oct_width / 2), l + i);
        append_to_output_buff(oct[i]);
    }
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

static void write_buff_cursor_loc_and_key(int key) {
    int sav_x, sav_y;
    char *path;

    sav_x = ys->cur_x;
    sav_y = ys->cur_y;

    yed_set_cursor(1, ys->term_rows - 1);
    yed_set_attr(yed_active_style_get_status_line());
    append_to_output_buff(TERM_CLEAR_LINE);

    if (ys->active_frame) {
        if (ys->active_frame->buffer) {
            yed_set_cursor(1, ys->term_rows - 1);
            path     = ys->active_frame->buffer->name;
            append_to_output_buff(path);
        }
        yed_set_cursor(ys->term_cols - 20, ys->term_rows - 1);
        append_n_to_output_buff("                    ", 20);
        yed_set_cursor(ys->term_cols - 20, ys->term_rows - 1);
        append_int_to_output_buff(ys->active_frame->cursor_line);
        append_to_output_buff(" :: ");
        append_int_to_output_buff(ys->active_frame->cursor_col);
    }

    yed_set_cursor(ys->term_cols - 5, ys->term_rows - 1);
    append_n_to_output_buff("     ", 5);
    yed_set_cursor(ys->term_cols - 5, ys->term_rows - 1);
    append_int_to_output_buff(key);
    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
    yed_set_cursor(sav_x, sav_y);
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

    write_buff_cursor_loc_and_key(0);

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
    yed_set_attr(yed_active_style_get_command_line());
    append_to_output_buff(ys->small_message);

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
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
        append_to_output_buff(TERM_CURSOR_SHOW);
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
        if (ys->redraw_cls) {
            append_to_output_buff(TERM_CLEAR_SCREEN);
            yed_draw_command_line();
        }
    }

    yed_update_frames();

    ys->redraw = ys->redraw_cls = 0;

    if (ys->interactive_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_RESET);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        write_buff_cursor_loc_and_key(keys[0]);
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
