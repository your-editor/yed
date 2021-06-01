#include <yed/plugin.h>

typedef struct {
    char *buffer_name;
    int   row;
    int   col;
} location;

static array_t stack;

void unload(yed_plugin *self);

void jump_stack_push(int n_args, char **args);
void jump_stack_pop(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    stack = array_make(location);

    yed_plugin_set_command(self, "jump-stack-push", jump_stack_push);
    yed_plugin_set_command(self, "jump-stack-pop",  jump_stack_pop);

    yed_plugin_set_unload_fn(self, unload);

    return 0;
}

void unload(yed_plugin *self) {
    location *it;

    array_traverse(stack, it) {
        free(it->buffer_name);
    }

    array_free(stack);
}

void jump_stack_push(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    location    target;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    target.buffer_name = strdup(buff->name);
    target.row         = frame->cursor_line;
    target.col         = frame->cursor_col;

    array_push(stack, target);

    yed_cprint("pushed a new target to the stack");
}

void jump_stack_pop(int n_args, char **args) {
    location *target;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (array_len(stack) == 0) {
        yed_cerr("the stack is empty");
        return;
    }

    target = array_last(stack);

    YEXE("buffer", target->buffer_name);
    yed_set_cursor_far_within_frame(ys->active_frame, target->row, target->col);

    free(target->buffer_name);
    array_pop(stack);
}
