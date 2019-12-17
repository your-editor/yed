#include <yed/plugin.h>

int yed_plugin_boot(yed_plugin *self) {
    YEXE("plugin-load", "lang/syntax/c");

    return 0;
}
