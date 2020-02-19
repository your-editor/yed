#include "internal.h"
#include "internal.c"

yed_state *ys;

void yed_write_welcome(void) {
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

    l = 1;

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

static void print_usage(void) {
    char *usage =
"usage: yed [options] [file...]\n"
"\n"
"options:\n"
"\n"
"--no-init\n"
"    Do not load an init plugin.\n"
"-i, --init=<path>\n"
"    Load the init plugin from this path instead of finding one automatically.\n"
"--instrument\n"
"    Pause the editor at startup to allow an external tool to attach to it.\n"
"--help\n"
"    Show this information.\n"
"\n"
;
    fprintf(stderr, "%s", usage);
}

static int parse_options(int argc, char **argv) {
    int i;

    ys->options.files = array_make(char*);

    for (i = 1; i < argc; i += 1) {
        if (strcmp(argv[i], "--instrument") == 0) {
            ys->options.instrument = 1;
        } else if (strcmp(argv[i], "--no-init") == 0) {
            ys->options.no_init = 1;
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i == argc - 1)    { return 0; }
            ys->options.init = argv[i + 1];
            i += 1;
        } else if (strncmp(argv[i], "--init=", 7) == 0) {
            ys->options.init = argv[i] + 7;
        } else if (strcmp(argv[i], "--help") == 0) {
            ys->options.help = 1;
        } else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) {
            return 0;
        } else {
            array_push(ys->options.files, argv[i]);
        }
    }

    return 1;
}

void yed_tool_attach(void) {
    printf("Hit any key to continue once the instrument tool has been attached.\n");
    printf("pid = %d\n", getpid());
    getchar();
}

yed_state * yed_init(yed_lib_t *yed_lib, int argc, char **argv) {
    int                 has_frames;
    char              **file_it;
    unsigned long long  start_time;

    ys = malloc(sizeof(*ys));
    memset(ys, 0, sizeof(*ys));

    ys->yed_lib = yed_lib;

    if (!parse_options(argc, argv)
    ||  ys->options.help) {
        print_usage();
        return NULL;
    }

    if (ys->options.instrument) {
        yed_tool_attach();
    }

    start_time = measure_time_now_ms();

    setlocale(LC_ALL, "en_US.utf8");

    yed_init_vars();
    ys->tabw = yed_get_tab_width(); /* Set again after plugins are loaded. */
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


    /*
     * Check if some configuration chaged the tab width
     * and set it in ys before loading buffers and doing
     * the first draw.
     */
    ys->tabw = yed_get_tab_width();

    has_frames = 0;
    array_traverse(ys->options.files, file_it) {
        YEXE("buffer", *file_it);
    }

    if (array_len(ys->options.files) >= 1) {
        YEXE("frame-new");
        YEXE("buffer", *(char**)array_item(ys->options.files, 0));
        has_frames = 1;
    }
    if (array_len(ys->options.files) > 1) {
        YEXE("frame-vsplit");
        YEXE("buffer", *(char**)array_item(ys->options.files, 1));
        YEXE("frame-prev");
    }

    if (has_frames) {
        yed_update_frames();
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else {
        append_to_output_buff(TERM_CURSOR_HIDE);
        yed_set_attr(yed_active_style_get_active());
        yed_clear_screen();
        yed_cursor_home();
        yed_write_welcome();
        append_to_output_buff(TERM_RESET);
    }

    write_status_bar(0);
    yed_draw_command_line();

    ys->redraw = 1;
    /*
     * setting the style will ask us to clear the screen,
     * but we don't really need to here.
     */
    ys->redraw_cls = 0;

    pthread_mutex_unlock(&ys->write_ready_mtx);

    ys->start_time_ms = measure_time_now_ms() - start_time;

    return ys;
}

void yed_fini(yed_state *state) {
    char *bytes;
    unsigned long long startup_time;

    startup_time = state->start_time_ms;

    printf(TERM_RESET);
    yed_term_exit();

    free(state);

    bytes = pretty_bytes(getPeakRSS());

    printf("Startup time: %llums\nPeak RSS:     %s\nThanks for using yed!\n", startup_time, bytes);
/*     printf("Average draw time: %.1fus\n", ((float)ys->draw_accum_us) / ((float)ys->n_pumps)); */

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
    yed_set_cursor((ys->term_cols / 2) - (strlen(ys->small_message) / 2), ys->term_rows - 1);
    yed_set_attr(yed_active_style_get_status_line());
    append_to_output_buff(ys->small_message);

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
    yed_set_cursor(sav_x, sav_y);
}

int yed_pump(void) {
    int                keys[16], n_keys, i, tabw_var_val;
    unsigned long long start_us;

    if (ys->status == YED_QUIT) {
        return YED_QUIT;
    }

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

    if ((tabw_var_val = yed_get_tab_width()) != ys->tabw) {
        ys->tabw = tabw_var_val;
        yed_update_line_visual_widths();
        ys->redraw = 1;
    }

    start_us = measure_time_now_us();

    if (ys->redraw) {
        if (yed_check_for_resize()) {
            yed_handle_resize();
        }
        if (ys->redraw_cls) {
            append_to_output_buff(TERM_CURSOR_HIDE);
            yed_set_attr(yed_active_style_get_active());
            yed_clear_screen();
            yed_cursor_home();
            yed_write_welcome();
            append_to_output_buff(TERM_RESET);
            yed_draw_command_line();
            write_status_bar(keys[0]);
        }
    }

    yed_update_frames();

    ys->draw_accum_us += measure_time_now_us() - start_us;
    ys->n_pumps       += 1;

    ys->redraw = ys->redraw_cls = 0;

    if (ys->interactive_command) {
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_RESET);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else if (ys->active_frame) {
        write_status_bar(keys[0]);
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
