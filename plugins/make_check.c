#include "plugin.h"

/* COMMANDS */
void make_check(int n_args, char **args);
/* END COMMANDS */

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_command(self, "make-check", make_check);

    return 0;
}

void make_check(int n_args, char **args) {
    YEXE("sh", "make check 2>&1 | less");
}
