#include "internal.h"
#include "internal.c"

yed_state *ys;

static void * update_forcer(void *arg) {
    (void)arg;

    while (ys->status != YED_RELOAD_CORE && ys->update_hz >= MIN_UPDATE_HZ) {
        usleep(825000 * (1.0 / MIN(ys->update_hz, MAX_UPDATE_HZ)));

        if (!ys->skip_force_update) {
            yed_force_update();
        } else {
            ys->skip_force_update = 0;
        }
    }

    return NULL;
}

static void kill_update_forcer(void) {
    void *junk;

    pthread_join(ys->update_forcer_id, &junk);
}

static void start_update_forcer(void) {
    pthread_create(&ys->update_forcer_id, NULL, update_forcer, NULL);
}


static void print_usage(void) {
    char *usage =
"usage: yed [options] [file...]\n"
"\n"
"options:\n"
"\n"
"--no-init\n"
"    Do not load an init plugin.\n"
"-c, --command=<command>\n"
"    Run the command after any init plugin is loaded. (repeatable)\n"
"--instrument\n"
"    Pause the editor at startup to allow an external tool to attach to it.\n"
"--version\n"
"    Print the version number and exit.\n"
"--major-version\n"
"    Print the major version number and exit.\n"
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
                printf("%d\n", YED_VERSION);
                do_exit = 1;
            } else if (strcmp(argv[i], "--major-version") == 0) {
                printf("%d\n", YED_MAJOR_VERSION);
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-lib-dir") == 0) {
                printf("%s\n", installed_lib_dir());
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-include-dir") == 0) {
                printf("%s\n", installed_include_dir());
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-default-plugin-dir") == 0) {
                printf("%s\n", default_plug_dir());
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-config-dir") == 0) {
                printf("%s\n", get_config_path());
                do_exit = 1;
            } else if (strcmp(argv[i], "--print-cflags") == 0) {
                printf(
#ifdef YED_DEBUG
                "-g -O0 -DYED_DEBUG -DYED_DO_ASSERTIONS "
#endif
                "-std=gnu99 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -shared -fPIC -I%s\n", installed_include_dir());

                do_exit = 1;
            } else if (strcmp(argv[i], "--print-cppflags") == 0) {
                printf(
#ifdef YED_DEBUG
                "-g -O0 -DYED_DEBUG -DYED_DO_ASSERTIONS "
#endif
                "-shared -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -fPIC -I%s\n", installed_include_dir());

                do_exit = 1;
            } else if (strcmp(argv[i], "--print-ldflags") == 0) {
                printf(
#ifdef YED_DEBUG
                "-g -O0 -DYED_DEBUG -DYED_DO_ASSERTIONS "
#endif
                "-rdynamic -shared -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -fPIC -L%s -lyed\n", installed_lib_dir());

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

void yed_draw_everything(void) {
    yed_event event;

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_PRE_DRAW_EVERYTHING;
    yed_trigger_event(&event);

    yed_draw_background();   yed_reset_attr();
    yed_write_status_line(); yed_reset_attr();
    yed_draw_command_line(); yed_reset_attr();
    yed_update_frames();     yed_reset_attr();
    yed_do_direct_draws();   yed_reset_attr();

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_POST_DRAW_EVERYTHING;
    yed_trigger_event(&event);

    yed_diff_and_swap_screens();
    yed_render_screen();
}

yed_state * yed_init(yed_lib_t *yed_lib, int argc, char **argv) {
    char                 cwd[4096];
    unsigned long long   start_time;
    int                  dev_null_fd;
    int                  pipe_ret;
    int                  fd_flags;
    char                *getcwd_ret;
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

    srand(time(NULL));

    /*
    ** Redirect stderr so that we don't get all kinds of unintended output
    ** when running subprocesses.
    */
    dev_null_fd = open("/dev/null", O_WRONLY);
    dup2(dev_null_fd, 2);
    close(dev_null_fd);

    pipe_ret = pipe(ys->signal_pipe_fds);
    (void)pipe_ret;
    fd_flags = fcntl(ys->signal_pipe_fds[0], F_GETFL);
    fcntl(ys->signal_pipe_fds[0], F_SETFL, fd_flags | O_NONBLOCK);

    setlocale(LC_ALL, "en_US.utf8");

    getcwd_ret = getcwd(cwd, sizeof(cwd));
    (void)getcwd_ret;
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
    yed_init_screen();
    yed_init_commands();
    yed_init_keys();
    yed_init_search();
    yed_init_completions();

    LOG_FN_ENTER();
    yed_log("basic systems initialized");
    LOG_EXIT();

    yed_init_plugins();

    array_traverse(cmd_line_commands, it) {
        split = sh_split(*it);
        yed_execute_command_from_split(split);
        free_string_array(split);
    }
    array_free(cmd_line_commands);

    yed_execute_command("open-command-line-buffers",
                        array_len(ys->options.files),
                        array_data(ys->options.files));

    yed_draw_everything();

    ys->start_time_ms = measure_time_now_ms() - start_time;

    LOG_FN_ENTER();
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
    int                  save_hz;
    int                  keys[16], n_keys, i;
    unsigned long long   start_us;
    int                  skip_keys;
    int                  got_non_null_key;

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
        save_hz = ys->update_hz;
        ys->update_hz = 0;
        yed_set_update_hz(save_hz);
    }

    ys->status = YED_NORMAL;

    skip_keys = ys->has_resized;
    if (ys->has_resized) {
        yed_handle_resize();
    } else {
        memset(keys, 0, sizeof(keys));
    }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_PRE_PUMP;
    yed_trigger_event(&event);

    n_keys = skip_keys
                ? 0
                : yed_read_keys(keys);

    got_non_null_key = 0;
    for (i = 0; i < n_keys; i += 1) {
        yed_take_key(keys[i]);
        got_non_null_key |= !!keys[i];
    }

    if (got_non_null_key && ys->update_hz >= MIN_UPDATE_HZ) {
        ys->skip_force_update = 1;
    }

    start_us = measure_time_now_us();

    yed_draw_everything();

    ys->draw_accum_us += measure_time_now_us() - start_us;
    ys->n_pumps       += 1;

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
            kill_update_forcer();
        }
    }
#endif

    if (ys->status == YED_QUIT) {
        event.kind = EVENT_PRE_QUIT;
        yed_trigger_event(&event);
    }

    return ys->status;
}
