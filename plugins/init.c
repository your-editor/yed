#include "plugin.h"

void add_commands(yed_plugin *self);
void kammerdiener_fill_cursor_line(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    int   i, n_plugins;
    char *plugins[] = {
        "vimish",
        "syntax_c",
        "indent_c",
        "autotrim",
        "make_check",
        "man",
        "comment",
        "proj",
        "style_first",
        "style_elise",
        "style_nord"
    };

    add_commands(self);

    YEXE("plugins-add-dir", "./.yed");

    n_plugins = sizeof(plugins) / sizeof(char*);

    for (i = 0; i < n_plugins; i += 1) {
        YEXE("plugin-load", plugins[i]);
    }

    YEXE("set", "tab-width", "4");

    YEXE("vimish-bind", "insert",  "j", "j",              "vimish-exit-insert");
    YEXE("vimish-bind", "nav",     "spc", "m", "c",       "make-check");
    YEXE("vimish-bind", "nav",     "spc", "c", "o",       "comment-toggle-line");
    YEXE("vimish-bind", "nav",     "spc", "r", "d",       "redraw");
    YEXE("vimish-bind", "nav",     "spc", "v", "s", "p",  "frame-vsplit");
    YEXE("vimish-bind", "nav",     "spc", "h", "s", "p",  "frame-hsplit");
    YEXE("vimish-bind", "nav",     "spc", "b", "o",       "buffer");
    YEXE("vimish-bind", "nav",     "spc", "b", "d",       "buffer-delete");
    YEXE("vimish-bind", "nav",     "ctrl-n",              "buffer-next");
    YEXE("vimish-bind", "nav",     "M", "M",              "man-word");
    YEXE("vimish-bind", "nav",     "L", "L",              "kammerdiener-fill-cursor-line");
    YEXE("vimish-bind", "nav",     "ctrl-y",              "build-and-reload");
    YEXE("vimish-bind", "nav",     "ctrl-l",              "frame-next");
    YEXE("vimish-bind", "nav",     ">",                   "indent-line");
    YEXE("vimish-bind", "nav",     "<",                   "unindent-line");

    if (yed_term_says_it_supports_truecolor()) {
        YEXE("style", "first-dark");
    } else {
        YEXE("style", "default");
    }

    return 0;
}

static void fill_cmd_prompt(const char *cmd) {
    int   len, i, key;
    char  key_str_buff[32];
    char *key_str;

    len = strlen(cmd);

    YEXE("command-prompt");

    for (i = 0; i < len; i += 1) {
        key = cmd[i];
        sprintf(key_str_buff, "%d", key);
        key_str = key_str_buff;
        YEXE("command-prompt", key_str);
    }

    key_str = "32"; /* space */
    YEXE("command-prompt", key_str);
}

void add_commands(yed_plugin *self) {
    yed_plugin_set_command(self, "kammerdiener-fill-cursor-line", kammerdiener_fill_cursor_line);
}

void kammerdiener_fill_cursor_line(int n_args, char **args) {
    fill_cmd_prompt("cursor-line");
}
