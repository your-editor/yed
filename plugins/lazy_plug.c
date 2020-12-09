#include <yed/plugin.h>

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

LOG_FN_ENTER();
    yed_cerr("lazy_plug: This plugin needs to be revised due to changes in how file types are handled. It has been disabled.");
LOG_EXIT();
    return 0;
}

#if 0
array_t ft_plugs[NUM_FT];

void unload(yed_plugin *self);
void lazy_plug_buff_post_load_handler(yed_event *event);
void lazy_plug_ft(int ft, char *plug_name);
void lazy_plug_ft_cmd(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    int               i;
    yed_event_handler buff_post_load_handler;

    yed_plugin_set_unload_fn(self, unload);

    for (i = 0; i < NUM_FT; i += 1) {
        ft_plugs[i] = array_make(char*);
    }

    yed_plugin_set_command(self, "lazy-plug-ft", lazy_plug_ft_cmd);

    buff_post_load_handler.kind = EVENT_BUFFER_POST_SET_FT;
    buff_post_load_handler.fn   = lazy_plug_buff_post_load_handler;

    yed_plugin_add_event_handler(self, buff_post_load_handler);

    return 0;
}

void unload(yed_plugin *self) {
    int    i;
    char **it;

    for (i = 0; i < NUM_FT; i += 1) {
        array_traverse(ft_plugs[i], it) {
            free(*it);
        }
        array_free(ft_plugs[i]);
    }
}

static void load_plug_if_not_loaded(char *plug_name) {
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)   it;

    it = tree_lookup(ys->plugins, plug_name);

    if (tree_it_good(it)) { return; }

    YEXE("plugin-load", plug_name);
}

void lazy_plug_buff_post_load_handler(yed_event *event) {
    yed_buffer  *buffer;
    array_t     *array;
    char       **it;

    buffer = event->buffer;

    if (!buffer || buffer->file.ft == FT_UNKNOWN) {
        return;
    }

    array = &ft_plugs[buffer->file.ft];

    array_traverse(*array, it) {
        load_plug_if_not_loaded(*it);
    }
}

void lazy_plug_ft(int ft, char *plug_name) {
    array_t  *array;
    char    **it;
    char     *plug_name_cpy;

    array = &ft_plugs[ft];

    array_traverse(*array, it) {
        if (strcmp(*it, plug_name) == 0) {
            return;
        }
    }

    plug_name_cpy = strdup(plug_name);
    array_push(*array, plug_name_cpy);
}

void lazy_plug_ft_cmd(int n_args, char **args) {
    char *ext;
    char *plug_name;
    int   ft;

    if (n_args != 2) {
        yed_cerr("expected 2 arguments, but got %d", n_args);
        return;
    }

    ext       = args[0];
    plug_name = args[1];

    ft = yed_get_ft(ext);

    if (ft == FT_UNKNOWN) {
        yed_cerr("invalid file type extension string '%s'", ext);
        return;
    }

    lazy_plug_ft(ft, plug_name);

    yed_cprint("plugin '%s' will be loaded when a %s ft buffer is loaded", plug_name, ext);
}
#endif
