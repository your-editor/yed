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

static int writer_started;

static void * writer(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&ys->write_ready_mtx);

        writer_started = 1;

        pthread_cond_wait(&ys->write_ready_cond, &ys->write_ready_mtx);
        flush_writer_buff();
        pthread_mutex_unlock(&ys->write_ready_mtx);
        if (ys->status == YED_RELOAD_CORE) { break; }
    }

    return NULL;
}

static void kick_off_write(void) {
    pthread_mutex_lock(&ys->write_ready_mtx);
    array_copy(ys->writer_buffer, ys->output_buffer);
    array_clear(ys->output_buffer);
    pthread_cond_signal(&ys->write_ready_cond);
    pthread_mutex_unlock(&ys->write_ready_mtx);
}

static void kill_writer(void) {
    void *junk;

    kick_off_write();
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
"-c, --command=<command>\n"
"    Run the command after any init plugin is loaded. (repeatable)\n"
"--instrument\n"
"    Pause the editor at startup to allow an external tool to attach to it.\n"
"--version\n"
"    Print the version number and exit.\n"
"--print-lib-dir\n"
"    Print the path of the directory containing the yed library and exit.\n"
"--print-include-dir\n"
"    Print the path of the directory containing the yed header files and exit.\n"
"--print-default-plugin-dir\n"
"    Print the path of the directory containing the yed default yed plugins and exit.\n"
"--print-config-dir\n"
"    Print the path of the directory where yed should look for configuration files and exit.\n"
"--print-cflags\n"
"    Print flags necessary to pass to a C compiler when compiling a plugin and exit.\n"
"--print-ldflags\n"
"    Print flags necessary to pass to the linker when linking a plugin and exit.\n"
"--help\n"
"    Show this information and exit.\n"
"\n"
;
    fprintf(stderr, "%s", usage);
}

static array_t cmd_line_commands;
static int parse_options(int argc, char **argv) {
    int   do_exit;
    int   seen_double_dash;
    int   i;
    char *s;

    do_exit          = 0;
    seen_double_dash = 0;

    ys->options.files = array_make(char*);
    cmd_line_commands = array_make(char*);

    for (i = 1; i < argc; i += 1) {
        if (seen_double_dash) {
            array_push(ys->options.files, argv[i]);
        } else {
            if (strcmp(argv[i], "--") == 0) {
                seen_double_dash = 1;
            } else if (strcmp(argv[i], "--version") == 0) {
                printf("%d\n", yed_version);
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-lib-dir") == 0) {
                printf("%s\n", INSTALLED_LIB_DIR);
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-include-dir") == 0) {
                printf("%s\n", INSTALLED_INCLUDE_DIR);
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-default-plugin-dir") == 0) {
                printf("%s\n", DEFAULT_PLUG_DIR);
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-config-dir") == 0) {
                printf("%s\n", get_config_path());
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-cflags") == 0) {
                printf(
#ifdef YED_DEBUG
                "-g -O0 "
#endif
                "-std=gnu99 -shared -fPIC -I%s\n", INSTALLED_INCLUDE_DIR);

                do_exit = 1;
            } else if (strcmp(argv[i], "--print-ldflags") == 0) {
                printf(
#ifdef YED_DEBUG
                "-g -O0 "
#endif
                "-rdynamic -shared -fPIC -L%s -lyed -Wl,-rpath,%s\n", INSTALLED_LIB_DIR, INSTALLED_LIB_DIR);

                do_exit = 1;
            } else if (strcmp(argv[i], "--instrument") == 0) {
                ys->options.instrument = 1;
            } else if (strcmp(argv[i], "--no-init") == 0) {
                ys->options.no_init = 1;
            } else if (strcmp(argv[i], "-c") == 0) {
                if (i == argc - 1)    { return 0; }
                s = argv[i + 1];
                array_push(cmd_line_commands, s);
                i += 1;
            } else if (strncmp(argv[i], "--command=", 10) == 0) {
                s = argv[i] + 7;
                array_push(cmd_line_commands, s);
            } else if (strcmp(argv[i], "--help") == 0) {
                ys->options.help = 1;
            } else if (strncmp(argv[i], "-", 1) == 0) {
                return 0;
            } else {
                array_push(ys->options.files, argv[i]);
            }
        }
    }

    if (do_exit) { exit(0); }

    return 1;
}

static void yed_tool_attach(void) {
    printf("Hit any key to continue once the instrument tool has been attached.\n");
    printf("pid = %d\n", getpid());
    getchar();
}

yed_state * yed_init(yed_lib_t *yed_lib, int argc, char **argv) {
    char                 cwd[4096];
    int                  has_frames;
    char               **file_it;
    unsigned long long   start_time;
    char               **it;
    array_t              split;

    ys = malloc(sizeof(*ys));
    memset(ys, 0, sizeof(*ys));

    ys->yed_lib = yed_lib;
    ys->argv0   = strdup(argv[0]);

    if (!parse_options(argc, argv)
    ||  ys->options.help) {
        print_usage();
        return NULL;
    }

    if (ys->options.instrument) {
        yed_tool_attach();
    }

    start_time = measure_time_now_ms();

    /*
    ** Close stderr so that we don't get all kinds of unintended output
    ** when running subprocesses.
    */
    close(2);

    setlocale(LC_ALL, "en_US.utf8");

    getcwd(cwd, sizeof(cwd));
    ys->working_dir = strdup(cwd);

    yed_init_events();
    yed_init_ft();
    yed_init_buffers();
    yed_init_frames();
    yed_init_vars();
    yed_init_styles();
    yed_init_log();
    yed_init_frame_trees();
    yed_init_direct_draw();

    yed_term_enter();
    yed_term_get_dim(&ys->term_rows, &ys->term_cols);

    ys->written_cells = malloc(ys->term_rows * ys->term_cols);
    memset(ys->written_cells, 0, ys->term_rows * ys->term_cols);

    memset(ys->_4096_spaces, ' ', 4096);
    yed_init_output_stream();

    pthread_mutex_init(&ys->write_ready_mtx, NULL);
    pthread_cond_init(&ys->write_ready_cond, NULL);

    pthread_create(&ys->writer_id, NULL, writer, NULL);
    while (!writer_started) { usleep(100); }
    yed_init_commands();
    yed_init_keys();
    yed_init_search();
    yed_init_completions();

    LOG_FN_ENTER();

    yed_log("basic systems initialized");

    yed_init_plugins();

    array_traverse(cmd_line_commands, it) {
        split = sh_split(*it);
        yed_execute_command_from_split(split);
        free_string_array(split);
    }
    array_free(cmd_line_commands);

    has_frames = !!array_len(ys->frames);

    if (array_len(ys->options.files) >= 1) {
        YEXE("frame-new");
        has_frames = 1;
    }

    array_traverse(ys->options.files, file_it) {
        YEXE("buffer", *file_it);
    }

    if (array_len(ys->options.files) >= 1) {
        YEXE("buffer", *(char**)array_item(ys->options.files, 0));
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

    ys->start_time_ms = measure_time_now_ms() - start_time;

    yed_log("\nStartup time: %llums", ys->start_time_ms);
    LOG_EXIT();

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

    (void)startup_time;
/*     printf("Startup time: %llums\nPeak RSS:     %s\nThanks for using yed!\n", startup_time, bytes); */
/*     printf("Average draw time: %.1fus\n", ((float)ys->draw_accum_us) / ((float)ys->n_pumps)); */
    printf("Thanks for using yed!\n");

    free(bytes);
}

void yed_set_state(yed_state *state)    { ys = state; }
yed_state * yed_get_state(void)         { return ys;  }


int yed_pump(void) {
    yed_event            event;
    int                  keys[16], n_keys, i;
    unsigned long long   start_us;
    int                  skip_keys;

    if (ys->status == YED_QUIT) {
        memset(&event, 0, sizeof(event));
        event.kind = EVENT_PRE_QUIT;
        yed_trigger_event(&event);
        return ys->status;
    }

    if (ys->status == YED_RELOAD) {
        yed_service_reload(0);
    } else if (ys->status == YED_RELOAD_CORE) {
        yed_service_reload(1);
        restart_writer();
    }

    /* Not sure why this is necessary, but... */
    if (!ys->interactive_command && ys->active_frame) {
        yed_set_cursor(ys->active_frame->cur_x, ys->active_frame->cur_y);
        append_to_output_buff(TERM_CURSOR_SHOW);
    }

    ys->status = YED_NORMAL;

    skip_keys = ys->has_resized;
    if (ys->has_resized) {
        yed_handle_resize();
    } else {
        memset(ys->written_cells, 0, ys->term_rows * ys->term_cols);
        memset(keys, 0, sizeof(keys));
    }


    /*
     * Give the writer thread the new screen update.
     */
    kick_off_write();

    append_to_output_buff(TERM_CURSOR_HIDE);



    memset(&event, 0, sizeof(event));
    event.kind = EVENT_PRE_PUMP;
    yed_trigger_event(&event);

    n_keys = skip_keys
                ? 0
                : yed_read_keys(keys);

    for (i = 0; i < n_keys; i += 1) {
        yed_take_key(keys[i]);
    }

    start_us = measure_time_now_us();

    if (ys->redraw) {
        if (ys->redraw_cls) {
            append_to_output_buff(TERM_CURSOR_HIDE);
            yed_set_attr(yed_active_style_get_active());
            yed_clear_screen();
            yed_cursor_home();
            append_to_output_buff(TERM_RESET);
            write_status_bar(keys[0]);
        }
        yed_mark_direct_draws_as_dirty();
    }

    yed_update_frames();
    ys->redraw = ys->redraw_cls = 0;
    yed_do_direct_draws();

    ys->draw_accum_us += measure_time_now_us() - start_us;
    ys->n_pumps       += 1;

    yed_draw_command_line();

    if (ys->interactive_command) {
        write_status_bar(keys[0]);
        yed_set_cursor(ys->cmd_cursor_x, ys->term_rows);
        append_to_output_buff(TERM_RESET);
        append_to_output_buff(TERM_CURSOR_SHOW);
    } else {
        write_status_bar(keys[0]);
        append_to_output_buff(TERM_CURSOR_HIDE);
        append_to_output_buff(TERM_RESET);
    }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_POST_PUMP;
    yed_trigger_event(&event);

    if (ys->status == YED_RELOAD) {
        yed_unload_plugin_libs();
    }

#ifdef CAN_RELOAD_CORE
    else if (ys->status == YED_RELOAD_CORE) {

        if (yed_check_version_breaking()) {
LOG_CMD_ENTER("reload");
            yed_cerr("Attempt to reload yed was rejected because the new version contains breaking changes.\n"
                     "    yed was not reloaded and all shared libraries and internal state remain unchanged.");
LOG_EXIT();
            ys->status = YED_NORMAL;
        } else {
            yed_unload_plugin_libs();
            kill_writer();
        }
    }
#endif

    if (ys->status == YED_QUIT) {
        event.kind = EVENT_PRE_QUIT;
        yed_trigger_event(&event);
    }

    return ys->status;
}
