#include <yed/plugin.h>

#define inline static inline
#include <yed/tree.h>
typedef char *yedrc_path_t;
use_tree(yedrc_path_t, int);
#undef inline


#include <stdio.h>


tree(yedrc_path_t, int) loading;

void yedrc_load(int n_args, char **args) {
    char                       *path;
    char                        exp_path[512];
    char                        abs_path[4096];
    tree_it(yedrc_path_t, int)  it;
    FILE                       *f;
    char                        line[512];
    char                        line_accum[4096];
    char                       *bw_scan;
    int                         bs_cont;
    array_t                     split;

    if (n_args != 1) {
        yed_cerr("expected 1 argument but got %d", n_args);
        return;
    }

    yed_cprint("'%s'\n", args[0]);

    path = args[0];
    expand_path(path, exp_path);
    path = exp_path;

    f = fopen(path, "r");

    if (!f) {
        yed_cerr("unable to open yedrc file '%s'", args[0]);
        return;
    }

    path = realpath(exp_path, abs_path);

    if (!path) {
        yed_cerr("invalid path to yedrc file '%s'", args[0]);
        return;
    }

    it = tree_lookup(loading, path);
    if (tree_it_good(it)) {
        yed_cerr("yedrc file '%s' is recursive! aborting load", args[0]);
        return;
    }

    yed_cprint("executing '%s'", args[0]);

    tree_insert(loading, path, 1);

    line_accum[0] = 0;
    while (fgets(line, sizeof(line), f)) {
        strncat(line_accum, line, sizeof(line_accum) - strlen(line_accum) - 1);

        bw_scan = line_accum + strlen(line_accum) - 1;
        while (bw_scan >= line_accum && isspace(*bw_scan)) { bw_scan -= 1; }
        bs_cont = *bw_scan == '\\';

        if (bs_cont) {
            *bw_scan = ' ';
        } else {
            split = sh_split(line_accum);
            if (array_len(split)) {
                yed_execute_command_from_split(split);
            }
            free_string_array(split);
            line_accum[0] = 0;
        }
    }

    tree_delete(loading, path);

    yed_cprint("done loading '%s'", args[0]);

    fclose(f);
}


void unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    loading = tree_make_c(yedrc_path_t, int, strcmp);

    yed_plugin_set_command(self, "yedrc-load", yedrc_load);

    return 0;
}

void unload(yed_plugin *self) {
    tree_free(loading);
}
