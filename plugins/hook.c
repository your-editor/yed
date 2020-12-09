#include <yed/plugin.h>

static yed_plugin *Self;

#define inline static inline
#include <yed/tree.h>
use_tree(int, array_t);
#undef inline

static tree(int, array_t) hooks_by_event;

typedef struct {
    char    *cmd;
    array_t  cmd_args;
} hook_t;

void unload(yed_plugin *self);
void hook_list_options(int n_args, char **args);
void hook_list_events(int n_args, char **args);
void hook(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    Self = self;

    hooks_by_event = tree_make(int, array_t);

    yed_plugin_set_command(self, "hook",              hook);
    yed_plugin_set_command(self, "hook-list-options", hook_list_options);
    yed_plugin_set_command(self, "hook-list-events",  hook_list_events);

    yed_plugin_set_unload_fn(self, unload);

    return 0;
}

void free_hook(hook_t *h) {
    char **it;

    free(h->cmd);
    array_traverse(h->cmd_args, it) {
        free(*it);
    }
    array_free(h->cmd_args);
}

void unload(yed_plugin *self) {
    tree_it(int, array_t)  event_it;
    hook_t                *hook_it;

    tree_traverse(hooks_by_event, event_it) {
        array_traverse(tree_it_val(event_it), hook_it) {
            free_hook(hook_it);
        }

        array_free(tree_it_val(event_it));
    }

    tree_free(hooks_by_event);
}

void execute_hook(hook_t *h) {
    yed_execute_command(h->cmd,
                        array_len(h->cmd_args),
                        array_data(h->cmd_args));
}

void hook_handler(yed_event *event) {
    tree_it(int, array_t)  lookup;
    hook_t                *hook_it;


    if (!tree_it_good(lookup
        = tree_lookup(hooks_by_event, event->kind))) { return; }

    array_traverse(tree_it_val(lookup), hook_it) {
        execute_hook(hook_it);
    }
}

void add_hook(hook_t *h, int event) {
    tree_it(int, array_t) it;
    yed_event_handler     handler;

    it = tree_lookup(hooks_by_event, event);
    if (!tree_it_good(it)) {
        it = tree_insert(hooks_by_event, event, array_make(hook_t));
        handler.kind = event;
        handler.fn   = hook_handler;
        yed_plugin_add_event_handler(Self, handler);
    }

    array_push(tree_it_val(it), *h);
}

void hook_list_options(int n_args, char **args) {
}
int hook_arg(hook_t *h, char *arg) {
    return 1;
}


void hook_list_events(int n_args, char **args) {
    yed_cprint("\ncursor-moved");
    yed_cprint("\npost-write");
}
int hook_event(char *arg) {
#define CHECK_EV(str, enu) \
    if (strcmp(arg, (str)) == 0) { return (enu); }

    CHECK_EV("cursor-moved", EVENT_CURSOR_MOVED);
    CHECK_EV("post-write",   EVENT_BUFFER_POST_WRITE);

    return -1;
#undef CHECK_EV
}

void hook(int n_args, char **args) {
    hook_t h;
    int    event;
    int    i;
    char * arg_dup;

    memset(&h, 0, sizeof(h));

    for (i = 0; i < n_args && strncmp(args[i], "--", 2) == 0; i += 1) {
        if (!hook_arg(&h, args[i])) { goto arg_error; }
    }

    if (n_args - i < 2) { goto arg_error; }

    if ((event = hook_event(args[i])) == -1) { goto arg_error; }

    i     += 1;
    h.cmd  = strdup(args[i]);

    if (i < n_args - 1) {
        h.cmd_args = array_make(char*);
        for (i = i + 1; i < n_args; i += 1) {
            arg_dup = strdup(args[i]);
            array_push(h.cmd_args, arg_dup);
        }
    }

    add_hook(&h, event);

    goto out;

arg_error:;
    yed_cerr("usage: hook [OPTIONS] EVENT CMD [CMD_ARGS...]\n"
             "    (run 'hook-list-options' or 'hook-list-events' for more info)");
out:;
    return;
}
