#include "plugin.h"

void add_commands(yed_plugin *self);
void kammerdiener_fill_cursor_line(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    int   i, n_plugins;
    char *plugins[] = {
        "vimish",
        "syntax_c",
        "syntax_sh",
        "brace_hl",
        "indent_c",
        "autotrim",
        "make_check",
        "man",
        "comment",
        "latex",
        "focus_frame",
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
    YEXE("set", "latex-comp-prg", "pdflatex -halt-on-error --interaction=nonstopmode");
    YEXE("set", "latex-view-prg", "open -a Skim");

    YEXE("vimish-bind", "insert",     "j", "j",              "vimish-exit-insert");
    YEXE("vimish-bind", "normal",     "spc", "m", "c",       "make-check");
    YEXE("vimish-bind", "normal",     "spc", "c", "o",       "comment-toggle");
    YEXE("vimish-bind", "normal",     "spc", "l", "c",       "latex-compile-current-file");
    YEXE("vimish-bind", "normal",     "spc", "l", "v",       "latex-view-current-file");
    YEXE("vimish-bind", "normal",     "spc", "r", "d",       "redraw");
    YEXE("vimish-bind", "normal",     "spc", "v", "s", "p",  "frame-vsplit");
    YEXE("vimish-bind", "normal",     "spc", "h", "s", "p",  "frame-hsplit");
    YEXE("vimish-bind", "normal",     "spc", "b", "o",       "buffer");
    YEXE("vimish-bind", "normal",     "spc", "b", "d",       "buffer-delete");
    YEXE("vimish-bind", "normal",     "ctrl-n",              "buffer-next");
    YEXE("vimish-bind", "normal",     "M", "M",              "man-word");
    YEXE("vimish-bind", "normal",     "L", "L",              "kammerdiener-fill-cursor-line");
    YEXE("vimish-bind", "normal",     "ctrl-l",              "frame-next");
    YEXE("vimish-bind", "normal",     ">",                   "indent");
    YEXE("vimish-bind", "normal",     "<",                   "unindent");

    YEXE("style", "first-dark");

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
