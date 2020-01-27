#include <yed/plugin.h>

#include <stdio.h>

void yedrc_load(int n_args, char **args) {
    char    *path;
    char     exp_path[512];
    char    *currently_loading;
    FILE    *f;
    char     line[512];
    array_t  split;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    path = args[0];
    expand_path(path, exp_path);
    path = exp_path;

    currently_loading = yed_get_var("yedrc-currently-loading");
    if (currently_loading && strcmp(currently_loading, path) == 0) {
        yed_append_text_to_cmd_buff("[!] recursive yedrc file situation");
        return;
    }

    f = fopen(path, "r");

    if (!f) {
        yed_append_text_to_cmd_buff("[!] unable to open yedrc file '");
        yed_append_text_to_cmd_buff(path);
        yed_append_text_to_cmd_buff("'");
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        yed_set_var("yedrc-currently-loading", path);

        split = sh_split(line);
        if (array_len(split)) {
            yed_execute_command_from_split(split);
        }
        free_string_array(split);
    }

    yed_unset_var("yedrc-currently-loading");

    fclose(f);
}

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_command(self, "yedrc-load", yedrc_load);

    return 0;
}
