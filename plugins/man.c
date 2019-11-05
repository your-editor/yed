#include "plugin.h"

void man(int n_args, char **args);
void man_word(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_command(self, "man", man);
    yed_plugin_set_command(self, "man-word", man_word);

    return 0;
}

void man_word(int n_args, char **args) {
    char *word;

    word = yed_word_under_cursor();

    if (!word) {
        yed_append_text_to_cmd_buff("[!] cursor is not on a word");
        return;
    }

    YEXE("man", word);

    free(word);
}

void man(int n_args, char **args) {
    char path_buff[128], cmd_buff[256], err_buff[256];
    int  i, err;

    path_buff[0] = 0;
    cmd_buff[0]  = 0;
    err_buff[0]  = 0;

    strcat(cmd_buff, "bash -c 'man");
    strcat(err_buff, "man");
    strcat(path_buff, "/tmp/man");

    for (i = 0; i < n_args; i += 1) {
        strcat(cmd_buff, " ");
        strcat(err_buff, " ");
        strcat(path_buff, "_");
        strcat(cmd_buff, args[i]);
        strcat(path_buff, args[i]);
        strcat(err_buff, args[i]);
    }
    strcat(path_buff, ".yed");
    strcat(cmd_buff, " 2>&1 | col -bx > ");
    strcat(cmd_buff, path_buff);
    strcat(cmd_buff, " && test ${PIPESTATUS[0]} -eq 0'");

    err = system(cmd_buff);

    if (err) {
        ys->redraw = 1; /* 'man' will poop on our screen if the command failed */
        yed_append_text_to_cmd_buff("[!] command '");
        yed_append_text_to_cmd_buff(err_buff);
        yed_append_text_to_cmd_buff("' failed");
        return;
    }

    YEXE("frame-new", "0.15", "0.15", "0.7", "0.7");
    YEXE("buffer", path_buff);
}
