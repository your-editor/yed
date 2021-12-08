#include <yed/plugin.h>

void style_use_term_bg(int n_args, char **args) {
    yed_style *s;

    s = yed_get_active_style();

    if (!s) { return; }

    if (s->command_line.bg == s->active.bg
    ||  s->command_line.bg == s->inactive.bg) {
        s->command_line.bg = 0;
    }
    if (s->cursor_line.bg == s->active.bg
    ||  s->cursor_line.bg == s->inactive.bg) {
        s->cursor_line.bg = 0;
    }
    if (s->active_gutter.bg == s->active.bg
    ||  s->active_gutter.bg == s->inactive.bg) {
        s->active_gutter.bg = 0;
    }
    if (s->inactive_gutter.bg == s->active.bg
    ||  s->inactive_gutter.bg == s->inactive.bg) {
        s->inactive_gutter.bg = 0;
    }
    s->active.bg          = 0;
    s->active_border.bg   = 0;
    s->inactive.bg        = 0;
    s->inactive_border.bg = 0;

    YEXE("redraw");
}

void style_term_bg(int n_args, char **args) {
    yed_style *s;

    if (n_args != 1) {
        yed_cerr("[!] expected 1 argument but got %d", n_args);
        return;
    }

    YEXE("style", args[0]);

    s = yed_get_active_style();

    if (!s)                             { return; }
    if (strcmp(s->_name, args[0]) != 0) { return; }

    YEXE("style-use-term-bg");
}

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "style-use-term-bg", style_use_term_bg);
    yed_plugin_set_command(self, "style-term-bg", style_term_bg);

    yed_plugin_set_completion(self, "style-term-bg-compl-arg-0", yed_get_completion("style"));
    return 0;
}
