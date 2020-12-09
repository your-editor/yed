#include <yed/plugin.h>

void focus_frame(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "focus-frame", focus_frame);

    return 0;
}

void focus_frame(int n_args, char **args) {
    yed_frame *frame;
    float      top_f, left_f, height_f, width_f;

    top_f    = 0.0;
    left_f   = 0.5 - ((80.0 / (float)ys->term_cols) / 2.0);
    height_f = 1.0;
    width_f  = 80.0 / (float)ys->term_cols;

    LIMIT(top_f,    0.0, 1.0);
    LIMIT(left_f,   0.0, 1.0);
    LIMIT(height_f, 0.1, 1.0);
    LIMIT(width_f,  0.1, 1.0);

    frame = yed_add_new_frame(top_f, left_f, height_f, width_f);

    yed_clear_frame(frame);
    yed_activate_frame(frame);
}
