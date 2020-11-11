#include "internal.h" /* include here so that plugins can see everything. */

static void * yed_get_handle_for_plug_if_in_dir(char *plug_name, char *dir_name) {
    char buff[256];

    buff[0] = 0;
    strcat(buff, dir_name);
    strcat(buff, "/");
    strcat(buff, plug_name);
    strcat(buff, ".so");

    return dlopen(buff, RTLD_NOW | RTLD_LOCAL);
}

void load_init(char *path) {
    int   err;
    void *handle;

    LOG_FN_ENTER();

    if (!path)    { goto not_found; }

    yed_log("attempting to load init plugin from '%s'", path);

    if (access(path, F_OK) != -1) {
        handle = yed_get_handle_for_plug_if_in_dir("init", path);

        if (!handle) { goto not_found;}

        yed_add_plugin_dir(path);

        err = yed_load_plugin("init");

        switch (err) {
            case YED_PLUG_NOT_FOUND:
                goto not_found;
            case YED_PLUG_SUCCESS:
                yed_log("\nloaded init");
                break;
            case YED_PLUG_NO_BOOT:
                yed_log("\n[!] init missing 'yed_plugin_boot'");
                break;
            case YED_PLUG_BOOT_FAIL:
                yed_log("\n[!] init 'yed_plugin_boot' failed");
                break;
        }
    } else {
not_found:
        yed_log("\n[!] init plugin not found");
    }

    LOG_EXIT();
}

void load_default_init(void) {
    char     buff[256];
    char    *home,
            *yed_dir;

    home = getenv("HOME");

    if (!home) {
        load_init(NULL);
    }

    buff[0] = 0;
    strcat(buff, home);
    strcat(buff, "/.yed");
    yed_dir = buff;

    load_init(yed_dir);
}

void yed_init_plugins(void) {
    ys->plugin_dirs = array_make(char*);
    ys->plugins     = tree_make_c(yed_plugin_name_t, yed_plugin_ptr_t, strcmp);

    LOG_FN_ENTER();
    yed_log("adding default plugin directory '%s'", default_plug_dir);
    yed_add_plugin_dir(default_plug_dir);
    LOG_EXIT();

    if (!ys->options.no_init) {
        if (ys->options.init) {
            load_init(ys->options.init);
        } else {
            load_default_init();
        }
    }
}

void yed_plugin_force_lib_unload(yed_plugin *plug) {
    void *try_handle;

    while ((try_handle = dlopen(plug->path, RTLD_NOW | RTLD_NOLOAD))) {
        dlclose(try_handle);
        dlclose(plug->handle);
    }
}

int yed_load_plugin(char *plug_name) {
    int                         err;
    char                        buff[256];
    char                      **dir_it;
    yed_plugin                 *plug;
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)   it;

    it = tree_lookup(ys->plugins, plug_name);

    if (tree_it_good(it)) {
        yed_unload_plugin(tree_it_key(it));
    }

    plug = malloc(sizeof(*plug));

    array_traverse(ys->plugin_dirs, dir_it) {
        if ((plug->handle = yed_get_handle_for_plug_if_in_dir(plug_name, *dir_it))) {
            buff[0] = 0;
            sprintf(buff, "%s/%s.so", *dir_it, plug_name);
            break;
        }
    }

    if (!plug->handle) {
        free(plug);
        return YED_PLUG_NOT_FOUND;
    }

    plug->path                 = strdup(buff);
    plug->added_cmds           = array_make(char*);
    plug->acquired_keys        = array_make(int);
    plug->added_bindings       = array_make(int);
    plug->added_key_sequences  = array_make(int);
    plug->added_event_handlers = array_make(yed_event_handler);
    plug->added_styles         = array_make(char*);
    plug->added_fts            = array_make(char*);

    plug->boot = dlsym(plug->handle, "yed_plugin_boot");
    if (!plug->boot) {
        dlclose(plug->handle);
        array_free(plug->added_bindings);
        array_free(plug->acquired_keys);
        array_free(plug->added_cmds);
        array_free(plug->added_key_sequences);
        array_free(plug->added_event_handlers);
        array_free(plug->added_styles);
        array_free(plug->added_fts);
        free(plug);
        return YED_PLUG_NO_BOOT;
    }

    plug->unload = NULL;

    err = plug->boot(plug);
    if (err) {
        dlclose(plug->handle);
        array_free(plug->added_bindings);
        array_free(plug->acquired_keys);
        array_free(plug->added_cmds);
        array_free(plug->added_key_sequences);
        array_free(plug->added_event_handlers);
        array_free(plug->added_styles);
        array_free(plug->added_fts);
        free(plug);
        return YED_PLUG_BOOT_FAIL;
    }

    tree_insert(ys->plugins, strdup(plug_name), plug);

    return YED_PLUG_SUCCESS;
}

void yed_plugin_uninstall_features(yed_plugin *plug) {
    tree_it(yed_command_name_t,
            yed_command)          cmd_it;
    char                        **cmd_name_it, **style_name_it, **ft_name_it;
    int                          *key_it;
    yed_event_handler            *handler_it;

    array_traverse(plug->added_cmds, cmd_name_it) {
        yed_unset_command(*cmd_name_it);

        /* If this is a default command, restore it. */
        cmd_it = tree_lookup(ys->default_commands, *cmd_name_it);
        if (tree_it_good(cmd_it)) {
            yed_set_command(tree_it_key(cmd_it), tree_it_val(cmd_it));
        }
    }
    array_free(plug->added_cmds);

    array_traverse(plug->acquired_keys, key_it) {
        yed_release_virt_key(*key_it);
    }
    array_free(plug->acquired_keys);

    array_traverse(plug->added_bindings, key_it) {
        yed_set_default_key_binding(*key_it);
    }
    array_free(plug->added_bindings);

    array_traverse(plug->added_key_sequences, key_it) {
        yed_delete_key_sequence(*key_it);
    }
    array_free(plug->added_key_sequences);

    array_traverse(plug->added_event_handlers, handler_it) {
        yed_delete_event_handler(*handler_it);
    }
    array_free(plug->added_event_handlers);

    array_traverse(plug->added_styles, style_name_it) {
        yed_remove_style(*style_name_it);
    }
    array_free(plug->added_styles);

    array_traverse(plug->added_fts, ft_name_it) {
        yed_delete_ft(*ft_name_it);
    }
    array_free(plug->added_fts);
}

int yed_unload_plugin(char *plug_name) {
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)     it;
    char                         *old_key;
    yed_plugin                   *old_plug;

    it = tree_lookup(ys->plugins, plug_name);

    if (!tree_it_good(it))    { return 1; }

    old_key  = tree_it_key(it);
    old_plug = tree_it_val(it);

    tree_delete(ys->plugins, old_key);

    if (old_plug) {
        if (old_plug->unload) {
            old_plug->unload(old_plug);
        }

        yed_plugin_uninstall_features(old_plug);
        yed_plugin_force_lib_unload(old_plug);

        free(old_plug->path);
        free(old_plug);
    }

    free(old_key);

    return 0;
}

int yed_unload_plugins(void) {
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)  it;

    while (tree_len(ys->plugins)) {
        it = tree_begin(ys->plugins);
        yed_unload_plugin(tree_it_key(it));
    }

    return 0;
}

int yed_unload_plugin_libs(void) {
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)   it;
    yed_plugin                 *plug;

    /*
     * First uninstall every plugin feature.
     * We don't want to trigger any events or otherwise
     * get into plugin code that won't exist when we start
     * to unload the libraries.
     */
    tree_traverse(ys->plugins, it) {
        plug = tree_it_val(it);
        yed_plugin_uninstall_features(plug);
    }

    tree_traverse(ys->plugins, it) {
        plug = tree_it_val(it);

        if (plug->unload) {
            plug->unload(plug);
            plug->unload = NULL;
        }

        yed_plugin_force_lib_unload(plug);
    }

    return 0;
}

int yed_reload_plugins(void) {
    array_t                      plugs;
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)    it;
    char                        *name_dup,
                               **name_it;

    plugs = array_make(char*);

    tree_traverse(ys->plugins, it) {
        name_dup = strdup(tree_it_key(it));
        array_push(plugs, name_dup);
    }

    yed_unload_plugins();

    array_traverse(plugs, name_it) {
        it = tree_lookup(ys->plugins, *name_it);
        if (!tree_it_good(it)) {
            yed_load_plugin(*name_it);
        }
        free(*name_it);
    }

    array_free(plugs);

    return 0;
}

void yed_plugin_set_command(yed_plugin *plug, char *name, yed_command command) {
    tree_it(yed_command_name_t, yed_command)  it;
    char                                     *old_name, *name_dup;

    it = tree_lookup(ys->commands, name);
    if (tree_it_good(it)) {
        old_name = tree_it_key(it);
        tree_delete(ys->commands, name);
        free(old_name);
    }

    name_dup = strdup(name);
    tree_insert(ys->commands, strdup(name), command);
    array_push(plug->added_cmds, name_dup);
}

int yed_plugin_acquire_virt_key(yed_plugin *plug) {
    int key;

    key = yed_acquire_virt_key();

    array_push(plug->acquired_keys, key);

    return key;
}

void yed_plugin_bind_key(yed_plugin *plug, int key, char *cmd_name, int n_args, char **args) {
    yed_key_binding binding;

    binding.key    = key;
    binding.cmd    = cmd_name;
    binding.n_args = n_args;
    binding.args   = args;

    yed_bind_key(binding);
    array_push(plug->added_bindings, key);
}

int yed_plugin_vadd_key_sequence(yed_plugin *plug, int len, ...) {
    va_list args;
    int     i, keys[MAX_SEQ_LEN];

    va_start(args, len);
    for (i = 0; i < len; i += 1) {
        keys[i] = va_arg(args, int);
    }
    va_end(args);

    return yed_plugin_add_key_sequence(plug, len, keys);
}

int yed_plugin_add_key_sequence(yed_plugin *plug, int len, int *keys) {
    int r;

    r = yed_add_key_sequence(len, keys);

    if (r != KEY_NULL) {
        array_push(plug->added_key_sequences, r);
    }

    return r;
}

void yed_plugin_add_event_handler(yed_plugin *plug, yed_event_handler handler) {
    array_push(plug->added_event_handlers, handler);
    yed_add_event_handler(handler);
}

void yed_plugin_set_style(yed_plugin *plug, char *name, yed_style *style) {
    char *name_dup;

    name_dup = strdup(name);
    yed_set_style(name, style);
    array_push(plug->added_styles, name_dup);
}

int yed_plugin_make_ft(yed_plugin *plug, const char *ft_name) {
    char *name_dup;
    int   result;

    name_dup = strdup(ft_name);
    result = yed_make_ft((char*)ft_name);
    array_push(plug->added_fts, name_dup);

    return result;
}

void yed_add_plugin_dir(char *s) {
    char   buff[1024], *s_dup;
    char **it;
    int    idx;

    expand_path(s, buff);

    /*
     * Delete the directory if it's already in the array.
     * If it's already in there we want to move it to the
     * beginning so that it's priority is updated.
     */
    idx = 0;
    array_traverse(ys->plugin_dirs, it) {
        if (strcmp(buff, *it) == 0) {
            array_delete(ys->plugin_dirs, idx);
            break;
        }
        idx += 1;
    }

    s_dup = strdup(buff);

    array_insert(ys->plugin_dirs, 0, s_dup);
}

void yed_plugin_set_unload_fn(yed_plugin *plug, yed_plugin_unload_fn_t fn) {
    plug->unload = fn;
}
