#include <yed/plugin.h>

typedef struct {
    u64 min_us;
    u64 max_us;
    u64 tot_us;
    int num_measure;
} cmdtime_t;

typedef struct {
    char *cmd;
    u64   us;
    u64   sub_us;
} cmdstart_t;

use_tree_c(str_t, cmdtime_t, strcmp);

static yed_plugin             *Self;
static tree(str_t, cmdtime_t)  results;
static array_t                 stack;
static int                     profile_on;
static int                     has_profile;
static yed_event_handler       h_cmd_pre;
static yed_event_handler       h_cmd_post;


static void free_profile(void) {
    tree_it(str_t, cmdtime_t)   it;
    char                       *key;
    cmdstart_t                 *start_it;

    while (tree_len(results)) {
        it  = tree_begin(results);
        key = tree_it_key(it);
        tree_delete(results, key);
        free(key);
    }

    tree_free(results);

    array_traverse(stack, start_it) { free(start_it->cmd); }
    array_free(stack);
}

static void profile_start(int n_args, char **args) {
    if (profile_on) {
        yed_cerr("a profile is already running");
        return;
    }

    if (has_profile) { free_profile(); }

    results     = tree_make(str_t, cmdtime_t);
    stack       = array_make(cmdstart_t);
    has_profile = 1;

    profile_on = 1;
    yed_plugin_add_event_handler(Self, h_cmd_pre);
    yed_plugin_add_event_handler(Self, h_cmd_post);
}

static void profile_end(int n_args, char **args) {
    if (!profile_on) {
        yed_cerr("no profile to stop");
        return;
    }

    yed_delete_event_handler(h_cmd_pre);
    yed_delete_event_handler(h_cmd_post);

    profile_on = 0;
}

static int iter_cpm(const void *_a, const void *_b) {
    const tree_it(str_t, cmdtime_t) *a;
    const tree_it(str_t, cmdtime_t) *b;

    a = _a;
    b = _b;

    return   (tree_it_val(*b).tot_us / tree_it_val(*b).num_measure)
           - (tree_it_val(*a).tot_us / tree_it_val(*a).num_measure);
}

static void profile_report(int n_args, char **args) {
    yed_buffer                *buff;
    tree_it(str_t, cmdtime_t)  it;
    int                        max_width;
    char                      *s;
    int                        len;
    int                        w;
    int                        i;
    char                       line[1024];
    array_t                    sorted;
    tree_it(str_t, cmdtime_t) *itit;
    cmdtime_t                 *time;
    const char                *last;

    if (!has_profile) {
        yed_cerr("no profile to report -- run 'profile-start'");
        return;
    }

    if (profile_on) {
        yed_cerr("run profile-end to see the results");
        return;
    }

    buff = yed_get_buffer("*profile");
    if (buff == NULL) {
        buff = yed_create_buffer("*profile");
        buff->flags |= BUFF_SPECIAL;
    }

    buff->flags &= ~BUFF_RD_ONLY;

    yed_buff_clear_no_undo(buff);

    max_width = 7;
    tree_traverse(results, it) {
        s   = tree_it_key(it);
        len = strlen(s);
        w   = 0;
        for (i = 0; i <= len; i += 1) {
            if (s[i] == 0 || s[i] == '\n') {
                if (w > max_width) { max_width = w; }
                w = 0;
            } else {
                w += 1;
            }
        }
    }

    snprintf(line, sizeof(line),
             "%-*s  %7s  %7s  %7s  %7s",
             max_width, "COMMAND", "AVG ms", "MIN ms", "MAX ms", "COUNT");

    len = strlen(line);
    yed_buff_insert_string_no_undo(buff, line, 1, 1);
    memset(line, '-', len);
    line[len] = 0;
    yed_buff_insert_string_no_undo(buff, line, 2, 1);

    sorted = array_make(tree_it(str_t, cmdtime_t));
    tree_traverse(results, it) {
        time = &tree_it_val(it);
        if (time->num_measure == 0) { continue; }
        array_push(sorted, it);
    }

    qsort(array_data(sorted), array_len(sorted), sizeof(tree_it(str_t, cmdtime_t)), iter_cpm);

    array_traverse(sorted, itit) {
        time = &tree_it_val(*itit);
        s    = tree_it_key(*itit);
        len  = strlen(s);
        s    = malloc(len + 1);
        memcpy(s, tree_it_key(*itit), len + 1);
        for (i = 0; i < len; i += 1) { if (s[i] == '\n') { s[i] = 0; } }
        w = 0;
        last = s;
        for (i = 0; i <= len; i += 1) {
            if (s[i] == 0) {
                if (w == 0) {
                    snprintf(line, sizeof(line),
                             "%-*s  %7.2f  %7.2f  %7.2f  %7d",
                             max_width, last,
                             ((float)time->tot_us / time->num_measure) / 1000.0,
                             time->min_us / 1000.0,
                             time->max_us / 1000.0,
                             time->num_measure);
                    yed_buff_insert_string_no_undo(buff, line, yed_buff_n_lines(buff) + 1, 1);
                } else {
                    yed_buff_insert_string_no_undo(buff, last, yed_buff_n_lines(buff) + 1, 1);
                }
                last  = s + i + 1;
                w    += 1;
            }
        }

        free(s);
    }

    buff->flags |= BUFF_RD_ONLY;

    YEXE("special-buffer-prepare-focus", "*profile");
    YEXE("buffer", "*profile");
}

static void cmd_pre_handler(yed_event *event) {
    int         name_len;
    int         len;
    char       *buff;
    int         i;
    int         j;
    cmdstart_t  start;

    if (!profile_on) { return; }

    name_len = strlen(event->cmd_name);
    len      = name_len;
    for (i = 0; i < event->n_args; i += 1) {
        len += strlen(event->args[i]) + 3 + name_len;
    }

    buff    = malloc(len + 1);
    buff[0] = 0;

    strcat(buff, event->cmd_name);
    for (i = 0; i < event->n_args; i += 1) {
        if (i == 0) {
            strcat(buff, " '");
            strcat(buff, event->args[i]);
            strcat(buff, "'");
        } else {
            strcat(buff, "\n");
            for (j = 0; j <= strlen(event->cmd_name); j += 1) {
                strcat(buff, " ");
            }
            strcat(buff, "'");
            strcat(buff, event->args[i]);
            strcat(buff, "'");
        }
    }

    start.cmd    = buff;
    start.us     = measure_time_now_us();
    start.sub_us = 0;

    array_push(stack, start);
}

static void cmd_post_handler(yed_event *event) {
    u64                        sub;
    u64                        elapsed;
    cmdstart_t                *start;
    tree_it(str_t, cmdtime_t)  it;
    cmdtime_t                  newtime;

    if (!profile_on) { return; }

    start = array_last(stack);
    if (start == NULL) { return; }

    sub     = start->sub_us;
    elapsed = measure_time_now_us() - start->us - sub;

    it = tree_lookup(results, start->cmd);
    if (tree_it_good(it)) {
        newtime = tree_it_val(it);
    } else {
        memset(&newtime, 0, sizeof(newtime));
        it = tree_insert(results, strdup(start->cmd), newtime);
    }

    if (newtime.num_measure == 0) {
        newtime.min_us = newtime.max_us = elapsed;
    } else {
        if      (elapsed < newtime.min_us) { newtime.min_us = elapsed; }
        else if (elapsed > newtime.max_us) { newtime.max_us = elapsed; }
    }

    newtime.tot_us      += elapsed;
    newtime.num_measure += 1;

    /*
     * Safe to use start->cmd here since we should always have
     * inserted a strdup()'d version before now.
     */
    tree_insert(results, start->cmd, newtime);

    free(start->cmd);
    array_pop(stack);

    start = array_last(stack);
    if (start != NULL) {
        start->sub_us += elapsed + sub;
    }
}

static void unload(yed_plugin *self) {
    if (has_profile) { free_profile(); }
}

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    Self = self;

    yed_plugin_set_unload_fn(Self, unload);

    h_cmd_pre.kind = EVENT_CMD_PRE_RUN;
    h_cmd_pre.fn   = cmd_pre_handler;

    h_cmd_post.kind = EVENT_CMD_POST_RUN;
    h_cmd_post.fn   = cmd_post_handler;

    yed_plugin_set_command(Self, "profile-start",  profile_start);
    yed_plugin_set_command(Self, "profile-end",    profile_end);
    yed_plugin_set_command(Self, "profile-report", profile_report);

    return 0;
}
