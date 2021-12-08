#include <yed/plugin.h>

void scroll_buffer(int n_args, char **argv);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "scroll-buffer", scroll_buffer);

    return 0;
}

void scroll_buffer(int n_args, char **argv) {
    int n_lines;

    if(!(ys->active_frame)) {
        return;
    }

    n_lines = 1;
    if(n_args) {
        sscanf(argv[0], "%d", &n_lines);
    }

    yed_frame_scroll_buffer(ys->active_frame, n_lines);
}
