#include <yed/plugin.h>

void get_env_info(void);
int has(char *prg);

int yed_plugin_boot(yed_plugin *self) {
    int   i, n_plugins;
    char *plugins[] = {
        "vimish",
        "brace_hl", "tag_hl", "cursor_word_hl",
        "lang/c", "lang/sh", "lang/bjou", "lang/latex",
        "indent_c", "comment",
        "autotrim", "completer",
        "grep", "find_file", "man",
        "style_picker",
        "focus_frame",
        "styles/first", "styles/elise", "styles/nord", "styles/monokai", "styles/gruvbox",
        "styles/skyfall", "styles/papercolor",
        "styles/casey",
    };


    /* Set variables before loading plugins. */
    get_env_info();

    if (yed_term_says_it_supports_truecolor()) {
        yed_set_var("truecolor",     "yes");
    }
    yed_set_var("tab-width",         "4");
    yed_set_var("latex-comp-prg",    "pdflatex -halt-on-error --interaction=nonstopmode");
    yed_set_var("latex-view-prg",    "open -a Skim");
    if (has("rg")) {
        yed_set_var("grep-prg",      "rg --vimgrep \"%\"");
    }
    if (has("fzf")) {
        yed_set_var("find-file-prg", "fzf --filter=\"%\"");
    }

    /* I like the cursor line. But it's not default. */
    yed_set_var("cursor-line", "yes");
    yed_set_var("vimish-insert-no-cursor-line", "yes");

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

    /* Load a directory-specific plugin if available. */
    YEXE("plugin-load", "proj");

    return 0;
}

void get_env_info(void) {
    char *term,
         *user;

    if ((term = getenv("TERM"))) { yed_set_var("term", term); }
    if ((user = getenv("USER"))) { yed_set_var("user", user); }
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
