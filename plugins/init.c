#include "plugin.h"

int yed_plugin_boot(yed_plugin *self) {
    int   i;
    char  buff[256];
    char *home,
         *plug_name,
         *plugins[] = { "vimish" };

    home = getenv("HOME");

    for (i = 0; i < sizeof(plugins) / sizeof(char*); i += 1) {
        buff[0] = 0;
        strcat(buff, home);
        strcat(buff, "/.yed/");
        strcat(buff, plugins[i]);
        strcat(buff, ".so");
        plug_name = buff;
        yed_execute_command("plugin-load", 1, &plug_name);
    }

    return 0;
}
