/* This file needs to interact with the yed core. */
#include <yed/plugin.h>

/*
 * Below is a simple implementation of a command that we're going to use to
 * compile this file when you make changes to it.
 */
void recompile_init(int n_args, char **args);

/* This is the entry point for this file when yed loads it. */
int yed_plugin_boot(yed_plugin *self) {
    /*
     * This macro ensures that our init plugin isn't loaded into an
     * incompatible version of yed.
     * All it does is return an error code back to yed if the versions
     * don't look right.
     */
    YED_PLUG_VERSION_CHECK();

    /* This makes the recompile_init function available as a command. */
    yed_plugin_set_command(self, "recompile-init", recompile_init);

    YEXE("plugin-load", "ypm");
    YEXE("plugin-load", "yedrc");
    YEXE("yedrc-load",  "~/.yed/yedrc");

    return 0;
}

void recompile_init(int n_args, char **args) {
    YEXE("sh", "gcc -o ~/.yed/init.so ~/.yed/init.c $(yed --print-cflags) $(yed --print-ldflags) && echo success");
}
