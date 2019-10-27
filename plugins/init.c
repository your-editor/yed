#include "plugin.h"

static void add_home_plug_dir(void) {
    char  buff[256];
    char *home,
         *yed_dir;

    home    = getenv("HOME");
    buff[0] = 0;
    strcat(buff, home);
    strcat(buff, "/.yed");
    yed_dir = buff;

    yed_execute_command("plugins-add-dir", 1, &yed_dir);
}

int yed_plugin_boot(yed_plugin *self) {
    int   i;
    char *plugins[] = {
        "vimish",
        "syntax_c",
        "indent_c",
        "proj"
    };
    char *cwd_yed = "./.yed";

    /* Not necessary. */
/*     add_home_plug_dir(); */

    yed_execute_command("plugins-add-dir", 1, &cwd_yed);

    for (i = 0; i < sizeof(plugins) / sizeof(char*); i += 1) {
        yed_execute_command("plugin-load", 1, plugins + i);
    }

    return 0;
}
