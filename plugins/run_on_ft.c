#include <yed/plugin.h>


int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

LOG_FN_ENTER();
    yed_cerr("run_on_ft: This plugin needs to be revised due to changes in how file types are handled. It has been disabled.");
LOG_EXIT();
    return 0;
}

#if 0

typedef struct {
    char *cmd;
    char **args;
    int    n_args;
} run_on_ft_cmd_t;

array_t ft_cmds[NUM_FT];

void unload(yed_plugin *self);
void run_on_ft_buff_post_set_ft_handler(yed_event *event);
int  run_on_ft(int ft, char *cmd, int n_args, char **args);
void run_on_ft_cmd(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    int               i;
    yed_event_handler buff_post_set_ft_handler;

    yed_plugin_set_unload_fn(self, unload);

    for (i = 0; i < NUM_FT; i += 1) {
        ft_cmds[i] = array_make(run_on_ft_cmd_t);
    }

    yed_plugin_set_command(self, "run-on-ft", run_on_ft_cmd);

    buff_post_set_ft_handler.kind = EVENT_BUFFER_POST_SET_FT;
    buff_post_set_ft_handler.fn   = run_on_ft_buff_post_set_ft_handler;

    yed_plugin_add_event_handler(self, buff_post_set_ft_handler);

    return 0;
}

static void free_cmd_t(run_on_ft_cmd_t *run) {
    int i;

    free(run->cmd);
    for (i = 0; i < run->n_args; i += 1) {
        free(run->args[i]);
    }
    free(run->args);
}

void unload(yed_plugin *self) {
    int              i;
    run_on_ft_cmd_t *it;

    for (i = 0; i < NUM_FT; i += 1) {
        array_traverse(ft_cmds[i], it) {
            free_cmd_t(it);
        }
        array_free(ft_cmds[i]);
    }
}

void run_on_ft_buff_post_set_ft_handler(yed_event *event) {
    yed_buffer      *buffer;
    array_t         *array;
    run_on_ft_cmd_t *it;

    buffer = event->buffer;

    if (!buffer || buffer->file.ft == FT_UNKNOWN) {
        return;
    }

    array = &ft_cmds[buffer->file.ft];

    array_traverse(*array, it) {
        yed_execute_command(it->cmd, it->n_args, it->args);
    }
}

static int cmd_t_equal(run_on_ft_cmd_t *a, run_on_ft_cmd_t *b) {
    int i;

    if (strcmp(a->cmd, b->cmd) != 0) {
        return 0;
    }

    if (a->n_args != b->n_args) {
        return 0;
    }

    for (i = 0; i < a->n_args; i += 1) {
        if (strcmp(a->args[i], b->args[i]) != 0) {
            return 0;
        }
    }

    return 1;
}

int run_on_ft(int ft, char *cmd, int n_args, char **args) {
    array_t         *array;
    run_on_ft_cmd_t  run,
                    *it;
    int              i;

    array = &ft_cmds[ft];

    run.cmd = strdup(cmd);
    run.n_args = n_args;
    run.args = malloc(sizeof(char*) * n_args);
    for (i = 0; i < n_args; i += 1) {
        run.args[i] = strdup(args[i]);
    }

    array_traverse(*array, it) {
        if (cmd_t_equal(&run, it)) {
            free_cmd_t(&run);
            return 0;
        }
    }

    array_push(*array, run);

    return 1;
}

void run_on_ft_cmd(int n_args, char **args) {
    char *ext;
    int   ft;
    int   i;

    if (n_args <= 1) {
        yed_cerr("expected at least 2 arguments, but got %d", n_args);
        return;
    }

    ext = args[0];
    ft  = yed_get_ft(ext);

    if (ft == FT_UNKNOWN) {
        yed_cerr("invalid file type extension string '%s'", ext);
        return;
    }

    if (run_on_ft(ft, args[1], n_args - 2, args + 2)) {
        yed_cprint("command \"%s", args[1]);
        for (i = 0; i < n_args - 2; i += 1) {
            yed_cprint(" '%s'", args[2 + i]);
        }
        yed_cprint("\" will be executed when a %s ft buffer is loaded", ext);
    } else {
        yed_cerr("command is already scheduled to be executed when a %s ft buffer is loaded", ext);
    }

}
#endif
