#include <yed/plugin.h>

void get_env_info(void);
int has(char *prg);

int yed_plugin_boot(yed_plugin *self) {
    int   i, n_plugins;
    char *plugins[] = {
        "yedrc",
        "vimish",
        "brace_hl", "cursor_word_hl", "log_hl",
        "lang/c", "lang/cpp", "lang/sh", "lang/latex", "lang/python", "lang/yedrc", "lang/jgraph", "lang/glsl",
        "indent_c", "comment", "align",
        "autotrim", "completer",
        "grep", "find_file", "man",
        "style_picker",
        "focus_frame",
        "style_use_term_bg",
        "style_pack",
    };


    YED_PLUG_VERSION_CHECK();

    /* Set variables before loading plugins. */
    get_env_info();

    if (yed_term_says_it_supports_truecolor()) {
        YEXE("set", "truecolor",     "yes");
    }
    YEXE("set", "tab-width",             "4");
    YEXE("set", "latex-comp-prg",        "xelatex -halt-on-error --interaction=nonstopmode '%'");
    YEXE("set", "latex-view-prg",        "echo \"mupdf '%'\" | $SHELL &");
    YEXE("set", "latex-update-view-prg", "pkill -HUP mupdf 2>&1 > /dev/null");
    if (has("rg")) {
        YEXE("set", "grep-prg",      "rg --vimgrep \"%\"");
    }
    if (has("fzf")) {
        YEXE("set", "find-file-prg", "fzf --filter=\"%\"");
    }

    /* I like the cursor line. But it's not default. */
    YEXE("set", "cursor-line", "yes");
    YEXE("set", "vimish-insert-no-cursor-line", "yes");

    /* Load all plugins. */
    YEXE("plugins-add-dir", "./.yed");

    n_plugins = sizeof(plugins) / sizeof(char*);

    for (i = 0; i < n_plugins; i += 1) {
        YEXE("plugin-load", plugins[i]);
    }

    /* Keybindings */
    YEXE("vimish-bind", "insert",    "j", "j",                "CMD",    "vimish-exit-insert");
    YEXE("vimish-bind", "normal",    "spc", "c", "o",         "CMD",    "comment-toggle");
    YEXE("vimish-bind", "normal",    "spc", "l", "c",         "CMD",    "latex-compile-current-file");
    YEXE("vimish-bind", "normal",    "spc", "l", "v",         "CMD",    "latex-view-current-file");
    YEXE("vimish-bind", "normal",    "spc", "r", "d",         "CMD",    "redraw");
    YEXE("vimish-bind", "normal",    "spc", "v", "s", "p",    "CMD",    "frame-vsplit");
    YEXE("vimish-bind", "normal",    "spc", "h", "s", "p",    "CMD",    "frame-hsplit");
    YEXE("vimish-bind", "normal",    "spc", "b", "o",         "CMD",    "fill-command-prompt", "buffer");
    YEXE("vimish-bind", "normal",    "spc", "b", "d",         "CMD",    "buffer-delete");
    YEXE("vimish-bind", "normal",    "ctrl-n",                "CMD",    "buffer-next");
    YEXE("vimish-bind", "normal",    "ctrl-p",                "CMD",    "buffer-prev");
    YEXE("vimish-bind", "normal",    "M", "M",                "CMD",    "man-word");
    YEXE("vimish-bind", "normal",    "L", "L",                "CMD",    "fill-command-prompt", "cursor-line");
    YEXE("vimish-bind", "normal",    "ctrl-l",                "CMD",    "frame-next");
    YEXE("vimish-bind", "normal",    "ctrl-h",                "CMD",    "frame-prev");
    YEXE("vimish-bind", "normal",    ">",                     "CMD",    "indent");
    YEXE("vimish-bind", "normal",    "<",                     "CMD",    "unindent");
    YEXE("vimish-bind", "normal",    "spc", "g",              "CMD",    "grep");
    YEXE("vimish-bind", "normal",    "spc", "f",              "CMD",    "find-file");

    /* Colors */
    YEXE("style", "casey");

    /* Load a user's rc file. */
    YEXE("yedrc-load", "~/.yed/.yedrc");
    YEXE("yedrc-load", "~/.yed/yedrc");

    /* Load a directory-specific rc file. */
    YEXE("yedrc-load", ".yedrc");

    /* Load a directory-specific plugin if available. */
    YEXE("plugin-load", "proj");

    return 0;
}

void get_env_info(void) {
    char *term,
         *user;

    if ((term = getenv("TERM"))) { YEXE("set", "term", term); }
    if ((user = getenv("USER"))) { YEXE("set", "user", user); }
}

/*
 * @bad @performance
 * Amazingly, this is actually the bottleneck for startup time.
 * We're pretty fast about everything else.
 * But it would be nice to have a faster method to get this
 * information.
 */
int has(char *prg) {
    char *path;

    path = exe_path(prg);

    if (path) {
        free(path);
        return 1;
    }

    return 0;
}
