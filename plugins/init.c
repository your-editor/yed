#include "plugin.h"

int yed_plugin_boot(yed_plugin *self) {
    int   i, n_plugins;
    char *plugins[] = {
        "vimish",
        "syntax_c",
        "indent_c",
        "autotrim",
        "make_check",
        "proj"
    };

    YEXE("plugins-add-dir", "./.yed");

    n_plugins = sizeof(plugins) / sizeof(char*);

    for (i = 0; i < n_plugins; i += 1) {
        YEXE("plugin-load", plugins[i]);
    }

    YEXE("set", "tab-width", "4");

    YEXE("vimish-bind", "insert",  "j", "j",              "vimish-exit-insert");
    YEXE("vimish-bind", "nav",     "spc", "m", "c",       "make-check");
    YEXE("vimish-bind", "nav",     "spc", "r", "d",       "redraw");
    YEXE("vimish-bind", "nav",     "spc", "v", "s", "p",  "frame-vsplit");
    YEXE("vimish-bind", "nav",     "spc", "h", "s", "p",  "frame-hsplit");
    YEXE("vimish-bind", "nav",     "spc", "b", "o",       "buffer");
    YEXE("vimish-bind", "nav",     "spc", "b", "d",       "buffer-delete");
    YEXE("vimish-bind", "nav",     "ctrl-n",              "buffer-next");
    YEXE("vimish-bind", "nav",     "M", "M",              "vimish-man-word");

    return 0;
}
