#include <yed/plugin.h>

int yed_plugin_boot(yed_plugin *self) {
    YEXE("plugin-load", "lang/syntax/latex");
    YEXE("plugin-load", "lang/tools/latex");

    return 0;
}
