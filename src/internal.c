void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str) {
    volatile int *trap;

    yed_term_exit();

    printf("Assertion failed -- %s\n"
           "at  %s :: line %d\n"
           "    Condition: '%s'\n",
           msg, fname, line, cond_str);

    trap = 0;
    (void)*trap;
}


uint64_t next_power_of_2(uint64_t x) {
    if (x == 0) {
        return 2;
    }

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}



char * pretty_bytes(uint64_t n_bytes) {
    uint64_t    s;
    double      count;
    char       *r;
    const char *suffixes[]
        = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };

    s     = 0;
    count = (double)n_bytes;

    while (count >= 1024 && s < 7) {
        s     += 1;
        count /= 1024;
    }

    r = calloc(64, 1);

    if (count - floor(count) == 0.0) {
        snprintf(r, 64, "%d", (int)count);
    } else {
        snprintf(r, 64, "%.2f", count);
    }

    strcat(r, suffixes[s]);

    return r;
}

int output_buff_len(void) { return array_len(ys->output_buffer); }

static char *itoa(char *p, unsigned x) {
    p += 3*sizeof(int);
    *--p = 0;
    do {
        *--p = '0' + x % 10;
        x /= 10;
    } while (x);
    return p;
}

int yed_check_version_breaking(void) {
    int   breaks;
    char *env;
    char  cmd_buff[1024];
    FILE *p;
    char  ver_buff[32];
    int   new_ver;

    breaks = 1;

    if ((env = getenv("LD_LIBRARY_PATH"))) {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "LD_LIBRARY_PATH='%s' %s --no-init --version",
                 env, ys->argv0);
    } else if ((env = getenv("DYLD_LIBRARY_PATH"))) {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "DYLD_LIBRARY_PATH='%s' %s --no-init --version",
                 env, ys->argv0);
    } else {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "%s --version",
                 ys->argv0);
    }

    p = popen(cmd_buff, "r");
    if (p == NULL) { goto out; }

    if (fgets(ver_buff, sizeof(ver_buff), p) == NULL) { goto out; }

    pclose(p);

    sscanf(ver_buff, "%d", &new_ver);

    breaks = (new_ver / 100) > (yed_version / 100);

out:;
    return breaks;
}

void yed_service_reload(int core) {
    yed_key_map_list                                *list;
    tree_it(yed_command_name_t, yed_command)         cmd_it;
    tree_it(yed_completion_name_t, yed_completion)   compl_it;
    tree_it(yed_style_name_t, yed_style_ptr_t)       style_it;
    char                                            *key;
    yed_style                                       *style;
    char                                           **ft_name_it;

    if (core) {
        tree_reset_fns(yed_style_name_t,      yed_style_ptr_t,       ys->styles);
        tree_reset_fns(yed_var_name_t,        yed_var_val_t,         ys->vars);
        tree_reset_fns(yed_buffer_name_t,     yed_buffer_ptr_t,      ys->buffers);
        for (list = ys->keymap_list; list != NULL; list = list->next) {
            tree_reset_fns(int, yed_key_binding_ptr_t, list->map->binding_map);
        }
        tree_reset_fns(yed_command_name_t,    yed_command,           ys->commands);
        tree_reset_fns(yed_command_name_t,    yed_command,           ys->default_commands);
        tree_reset_fns(yed_completion_name_t, yed_completion,        ys->completions);
        tree_reset_fns(yed_completion_name_t, yed_completion,        ys->default_completions);
        tree_reset_fns(yed_plugin_name_t,     yed_plugin_ptr_t,      ys->plugins);
    }

    ys->cur_log_name = NULL; /* This could be memory from a plugin that got unloaded. */

    array_traverse(ys->ft_array, ft_name_it) {
        free(*ft_name_it);
    }
    array_clear(ys->ft_array);

    /*
     * Clear out all of the old styles.
     */
    while (tree_len(ys->styles)) {
        style_it = tree_begin(ys->styles);
        key   = tree_it_key(style_it);
        style = tree_it_val(style_it);
        tree_delete(ys->styles, key);
        free(key);
        free(style);
    }

    ys->active_style = NULL;
    /*
     * Reset the defaults.
     */
    yed_set_default_styles();

    /*
     * Clear out all of the old commands.
     */
    while (tree_len(ys->commands)) {
        cmd_it = tree_begin(ys->commands);
        key = tree_it_key(cmd_it);
        tree_delete(ys->commands, key);
        free(key);
    }
    while (tree_len(ys->default_commands)) {
        cmd_it = tree_begin(ys->default_commands);
        key = tree_it_key(cmd_it);
        tree_delete(ys->default_commands, key);
        free(key);
    }
    /*
     * Reset the defaults.
     */
    yed_set_default_commands();
    /*
     * Clear out all of the old completions.
     */
    while (tree_len(ys->completions)) {
        compl_it = tree_begin(ys->completions);
        key      = tree_it_key(compl_it);
        tree_delete(ys->completions, key);
        free(key);
    }
    while (tree_len(ys->default_completions)) {
        compl_it = tree_begin(ys->default_completions);
        key      = tree_it_key(compl_it);
        tree_delete(ys->default_completions, key);
        free(key);
    }
    /*
     * Reset the defaults.
     */
    yed_set_default_completions();

    yed_reload_default_event_handlers();
    yed_reload_plugins();

    if (core) {
        yed_register_sigwinch_handler();
        yed_register_sigstop_handler();
        yed_register_sigcont_handler();
    }
}

static void start_update_forcer(void);

void yed_force_update(void) {
    yed_signal(YED_SIG_FORCE_UPDATE);
}

int yed_get_update_hz(void) { return ys->update_hz; }

void yed_set_update_hz(int hz) {
    int need_to_start_updater;

    if      (hz < MIN_UPDATE_HZ) { hz = 0;             }
    else if (hz > MAX_UPDATE_HZ) { hz = MAX_UPDATE_HZ; }

    need_to_start_updater = ys->update_hz < MIN_UPDATE_HZ && hz;

    ys->update_hz = hz;

    if (need_to_start_updater) {
        start_update_forcer();
    }
}

void yed_signal(char sig) {
    int write_ret;

    write_ret = write(ys->signal_pipe_fds[1], &sig, 1);
    (void)write_ret;
}

void yed_handle_signal(char sig) {
    switch (sig) {
        case YED_SIG_FORCE_UPDATE:
            break;
        default:;
            yed_log("unrecognized signal received: 0x%x", sig);
            break;
    }
}

int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}

static const char * _u8_to_s[] = {
"0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",   "10",   "11", "12",  "13",  "14",  "15",
"16",  "17",  "18",  "19",  "20",  "21",  "22",  "23",  "24",  "25",  "26",  "27",  "28",  "29",  "30",  "31",
"32",  "33",  "34",  "35",  "36",  "37",  "38",  "39",  "40",  "41",  "42",  "43",  "44",  "45",  "46",  "47",
"48",  "49",  "50",  "51",  "52",  "53",  "54",  "55",  "56",  "57",  "58",  "59",  "60",  "61",  "62",  "63",
"64",  "65",  "66",  "67",  "68",  "69",  "70",  "71",  "72",  "73",  "74",  "75",  "76",  "77",  "78",  "79",
"80",  "81",  "82",  "83",  "84",  "85",  "86",  "87",  "88",  "89",  "90",  "91",  "92",  "93",  "94",  "95",
"96",  "97",  "98",  "99",  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111",
"112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125", "126", "127",
"128", "129", "130", "131", "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142", "143",
"144", "145", "146", "147", "148", "149", "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
"160", "161", "162", "163", "164", "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
"176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186", "187", "188", "189", "190", "191",
"192", "193", "194", "195", "196", "197", "198", "199", "200", "201", "202", "203", "204", "205", "206", "207",
"208", "209", "210", "211", "212", "213", "214", "215", "216", "217", "218", "219", "220", "221", "222", "223",
"224", "225", "226", "227", "228", "229", "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
"240", "241", "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252", "253", "254", "255"
};

const char *u8_to_s(u8 u) { return _u8_to_s[u]; }

#include "install.c"
#include "array.c"
#include "bucket_array.c"
#include "term.c"
#include "screen.c"
#include "key.c"
#include "wcwidth.c"
#include "utf8.c"
#include "undo.c"
#include "buffer.c"
#include "attrs.c"
#include "ft.c"
#include "frame.c"
#include "log.c"
#include "command.c"
#include "getRSS.c"
#include "measure_time.c"
#include "default_event_handlers.c"
#include "event.c"
#include "plugin.c"
#include "boyer_moore.c"
#include "find.c"
#include "var.c"
#include "util.c"
#include "style.c"
#include "subproc.c"
#include "complete.c"
#include "direct_draw.c"
#include "frame_tree.c"
#include "version.c"
#include "print_backtrace.c"
#include "cmd_line.c"
#include "status_line.c"
