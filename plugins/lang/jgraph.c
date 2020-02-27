#include <yed/plugin.h>

void unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    yed_plugin_set_unload_fn(self, unload);

    YEXE("plugin-load", "lang/syntax/jgraph");

    return 0;
}

void unload(yed_plugin *self) {
    YEXE("plugin-unload", "lang/syntax/jgraph");
}
