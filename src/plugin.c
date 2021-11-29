#include "internal.h" /* include here so that plugins can see everything. */

static void * yed_get_handle_for_plug(char *plug_path) {
    return dlopen(plug_path, RTLD_NOW | RTLD_LOCAL);
}

int load_init(const char *path) {
    int   loaded;
    int   err;
    char  full_path[4096];
    char *msg;

    LOG_FN_ENTER();

    loaded = 0;

    if (!path)    { goto not_found; }

    yed_cprint("attempting to load init plugin from '%s'", path);

    snprintf(full_path, sizeof(full_path), "%s/init.so", path);
    if (access(full_path, F_OK) != -1) {
        yed_add_plugin_dir(path);

        err = yed_load_plugin("init");

        switch (err) {
            case YED_PLUG_NOT_FOUND:
                goto not_found;
            case YED_PLUG_DLOAD_FAIL:
                msg = dlerror();
                if (msg == NULL) {
                    msg = "failed to load init plugin for unknown reason";
                }
                break;
            case YED_PLUG_SUCCESS:
                msg = "loaded init";
                break;
            case YED_PLUG_NO_BOOT:
                msg = "init missing 'yed_plugin_boot'";
                break;
            case YED_PLUG_BOOT_FAIL:
                msg = "init 'yed_plugin_boot' failed";
                break;
            case YED_PLUG_VER_MIS:
                msg = "init plugin was rejected because it was compiled against an older version of yed and is not compatible with this version";
                break;
        }

        if (err == YED_PLUG_SUCCESS) {
            yed_cprint("\n%s", msg);
            loaded = 1;
        } else {
            yed_cerr("%s", msg);
        }
    } else {
not_found:;
        yed_cerr("init plugin not found at '%s'", path);
    }

    LOG_EXIT();

    return loaded;
}

static array_t    dds;
yed_event_handler h;
static int        which_menu;

void create_default_init_menu1(void) {
    char               files_buff[256];
    char               f_buff[128];
    char               msg1_buff[256];
    yed_attrs          attrs;
    int                width;
    yed_direct_draw_t *dd;
    char               buff[256];
    const char        *msg1;
    const char        *msg2;
    const char        *msg3;

    dds = array_make(yed_direct_draw_t*);

    files_buff[0] = 0;
    f_buff[0]     = 0;
    homeify_path(get_config_path(), f_buff);
    strcat(files_buff, f_buff);
    strcat(files_buff, "/{init.c init.so build_init.sh yedrc}");

    homeify_path(get_config_path(), f_buff);
    snprintf(msg1_buff, sizeof(msg1_buff),
             "It looks like you don't have a config at %s",
             f_buff);


    attrs = yed_active_style_get_active();
    attrs.flags ^= ATTR_INVERSE;
    width = MAX(46, MAX(strlen(files_buff), strlen(msg1_buff))) + 2;
    memset(buff, 0, sizeof(buff));
    memset(buff, ' ', width);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg1 = msg1_buff;

    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg1) / 2), msg1, strlen(msg1));
    dd = yed_direct_draw((ys->term_rows / 2) - 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg2 =
"Would you like me to create one with a";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg2) / 2), msg2, strlen(msg2));
    dd = yed_direct_draw((ys->term_rows / 2) + 0, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg3 =
"simple example configuration? [y/n]";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg3) / 2), msg3, strlen(msg3));
    dd = yed_direct_draw((ys->term_rows / 2) + 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    which_menu = 1;
}

void create_default_init_menu2(void) {
    char               files_buff[256];
    char               f_buff[128];
    char               msg1_buff[256];
    yed_attrs          attrs;
    int                width;
    char               buff[256];
    yed_direct_draw_t *dd;
    const char        *msg1;
    const char        *msg2;
    const char        *msg3;

    dds = array_make(yed_direct_draw_t*);

    files_buff[0] = 0;
    f_buff[0]     = 0;
    homeify_path(get_config_path(), f_buff);
    strcat(files_buff, f_buff);
    strcat(files_buff, "/{init.c init.so build_init.sh yedrc}");

    homeify_path(get_config_path(), f_buff);
    snprintf(msg1_buff, sizeof(msg1_buff),
             "It looks like you don't have a config at %s",
             f_buff);


    attrs = yed_active_style_get_active();
    attrs.flags ^= ATTR_INVERSE;
    width = MAX(46, MAX(strlen(files_buff), strlen(msg1_buff))) + 2;
    memset(buff, 0, sizeof(buff));
    memset(buff, ' ', width);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg1 =
"Really create these files?";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg1) / 2), msg1, strlen(msg1));
    dd = yed_direct_draw((ys->term_rows / 2) - 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg2 = files_buff;
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg2) / 2), msg2, strlen(msg2));
    dd = yed_direct_draw((ys->term_rows / 2) + 0, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    msg3 =
"[y/n]";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg3) / 2), msg3, strlen(msg3));
    dd = yed_direct_draw((ys->term_rows / 2) + 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(dds, dd);

    which_menu = 2;
}

void do_create_default_init(void) {
    char  buff[4096 + 256];
    char *output;
    int   output_len;
    int   status;

LOG_FN_ENTER();
    snprintf(buff, sizeof(buff),
             "(mkdir -p %s "
             "&& find %s/yed/start/* -maxdepth 1 -type f -exec cp {} %s/. \\; ) 2>&1",
             get_config_path(), INSTALLED_SHARE_DIR, get_config_path());

    output = yed_run_subproc(buff, &output_len, &status);

    if (output != NULL) {
        if (status == 0) {
            ys->status = YED_RELOAD;
        } else {
            yed_cerr(output);
        }
        free(output);
    }

LOG_EXIT();
}

void create_default_init_key_handler(yed_event *event) {
    yed_direct_draw_t **dit;

    if (event->key == 'y' || event->key == 'Y') {
        if (which_menu == 1) {
            array_traverse(dds, dit) {
                yed_kill_direct_draw(*dit);
            }
            create_default_init_menu2();
        } else if (which_menu == 2) {
            array_traverse(dds, dit) {
                yed_kill_direct_draw(*dit);
            }
            array_free(dds);
            yed_delete_event_handler(h);
            do_create_default_init();
            ys->redraw_cls = 1;
        }
    } else if (event->key == 'n' || event->key == 'N') {
        array_traverse(dds, dit) {
            yed_kill_direct_draw(*dit);
        }
        array_free(dds);
        yed_delete_event_handler(h);
        ys->redraw_cls = 1;
    }
    event->cancel = 1;
}

void create_default_init(void) {
    create_default_init_menu1();

    h.kind = EVENT_KEY_PRESSED;
    h.fn = create_default_init_key_handler;
    yed_add_event_handler(h);
}

void load_default_init(void) {
    const char *yed_dir;

    yed_dir = get_config_path();

    if (!load_init(yed_dir)) {
        if (access(yed_dir, F_OK) == -1) {
            create_default_init();
        }
    }
}

void yed_init_plugins(void) {
    ys->plugin_dirs = array_make(char*);
    ys->plugins     = tree_make(yed_plugin_name_t, yed_plugin_ptr_t);

    if (strlen(DEFAULT_PLUG_DIR)) {
        LOG_FN_ENTER();
        yed_log("adding default plugin directory '%s'", DEFAULT_PLUG_DIR);
        yed_add_plugin_dir(DEFAULT_PLUG_DIR);
        LOG_EXIT();
    }

    if (!ys->options.no_init) {
        load_default_init();
    }
}

void yed_plugin_force_lib_unload(yed_plugin *plug) {
/*     void *try_handle; */
/*  */
/*     while ((try_handle = dlopen(plug->path, RTLD_NOW | RTLD_NOLOAD))) { */
/*         dlclose(try_handle); */
/*         dlclose(plug->handle); */
/*     } */
    dlclose(plug->handle);
}

int yed_load_plugin(char *plug_name) {
    yed_event                   evt;
    int                         err;
    char                        path_buff[4096];
    char                      **dir_it;
    yed_plugin                 *plug;
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)   it;

    memset(&evt, 0, sizeof(evt));
    evt.kind        = EVENT_PLUGIN_PRE_LOAD;
    evt.plugin_name = plug_name;

    yed_trigger_event(&evt);
    if (evt.cancel == 1) { return YED_PLUG_LOAD_CANCEL; }

    it = tree_lookup(ys->plugins, plug_name);

    if (tree_it_good(it)) {
        yed_unload_plugin(tree_it_key(it));
    }

    plug = malloc(sizeof(*plug));
    memset(plug, 0, sizeof(*plug));

    array_traverse(ys->plugin_dirs, dir_it) {
        snprintf(path_buff, sizeof(path_buff), "%s/%s.so", *dir_it, plug_name);

        if (access(path_buff, F_OK) == -1)                       { continue; }
        if ((plug->handle = yed_get_handle_for_plug(path_buff))) { break;    }
        else {
            free(plug);
            return YED_PLUG_DLOAD_FAIL;
        }
    }

    if (!plug->handle) {
        free(plug);
        return YED_PLUG_NOT_FOUND;
    }

    plug->path                 = strdup(path_buff);
    plug->added_cmds           = array_make(char*);
    plug->acquired_keys        = array_make(int);
    plug->added_bindings       = array_make(int);
    plug->added_key_sequences  = array_make(int);
    plug->added_event_handlers = array_make(yed_event_handler);
    plug->added_styles         = array_make(char*);
    plug->added_fts            = array_make(char*);
    plug->added_compls         = array_make(char*);

    plug->boot = dlsym(plug->handle, "yed_plugin_boot");
    if (!plug->boot) {
        dlclose(plug->handle);
        yed_plugin_uninstall_features(plug);
        free(plug);
        return YED_PLUG_NO_BOOT;
    }

    plug->unload = NULL;

    err = plug->boot(plug);
    if (err) {
        dlclose(plug->handle);
        yed_plugin_uninstall_features(plug);
        free(plug);

        if (err == YED_PLUG_VER_MIS) {
            return YED_PLUG_VER_MIS;
        }
        return YED_PLUG_BOOT_FAIL;
    }

    tree_insert(ys->plugins, strdup(plug_name), plug);

    evt.kind = EVENT_PLUGIN_POST_UNLOAD;
    yed_trigger_event(&evt);

    return YED_PLUG_SUCCESS;
}

#define FREE_AND_ZERO_PLUGIN_ARRAY(_a) \
do {                                   \
    array_free(_a);                    \
    memset(&(_a), 0, sizeof(_a));      \
} while (0)

#define FREE_AND_ZERO_PLUGIN_STRING_ARRAY(_a) \
do {                                          \
    free_string_array(_a);                    \
    memset(&(_a), 0, sizeof(_a));             \
} while (0)

void yed_plugin_uninstall_features(yed_plugin *plug) {
    tree_it(yed_command_name_t,
            yed_command)             cmd_it;
    char                           **cmd_name_it, **style_name_it, **ft_name_it;
    int                             *key_it;
    yed_event_handler               *handler_it;
    tree_it(yed_completion_name_t,
            yed_completion)          compl_it;
    char                           **compl_name_it;

    array_traverse(plug->added_cmds, cmd_name_it) {
        yed_unset_command(*cmd_name_it);

        /* If this is a default command, restore it. */
        cmd_it = tree_lookup(ys->default_commands, *cmd_name_it);
        if (tree_it_good(cmd_it)) {
            yed_set_command(tree_it_key(cmd_it), tree_it_val(cmd_it));
        }
    }
    FREE_AND_ZERO_PLUGIN_STRING_ARRAY(plug->added_cmds);

    array_traverse(plug->acquired_keys, key_it) {
        yed_release_virt_key(*key_it);
    }
    FREE_AND_ZERO_PLUGIN_ARRAY(plug->acquired_keys);

    array_traverse(plug->added_bindings, key_it) {
        yed_set_default_key_binding(*key_it);
    }
    FREE_AND_ZERO_PLUGIN_ARRAY(plug->added_bindings);

    array_traverse(plug->added_key_sequences, key_it) {
        yed_delete_key_sequence(*key_it);
    }
    FREE_AND_ZERO_PLUGIN_ARRAY(plug->added_key_sequences);

    array_traverse(plug->added_event_handlers, handler_it) {
        yed_delete_event_handler(*handler_it);
    }
    FREE_AND_ZERO_PLUGIN_ARRAY(plug->added_event_handlers);

    array_traverse(plug->added_styles, style_name_it) {
        yed_remove_style(*style_name_it);
    }
    FREE_AND_ZERO_PLUGIN_STRING_ARRAY(plug->added_styles);

    array_traverse(plug->added_fts, ft_name_it) {
        yed_delete_ft(*ft_name_it);
    }
    FREE_AND_ZERO_PLUGIN_STRING_ARRAY(plug->added_fts);

    array_traverse(plug->added_compls, compl_name_it) {
        yed_unset_completion(*compl_name_it);

        /* If this is a default completion, restore it. */
        compl_it = tree_lookup(ys->default_completions, *compl_name_it);
        if (tree_it_good(compl_it)) {
            yed_set_completion(tree_it_key(compl_it), tree_it_val(compl_it));
        }
    }
    FREE_AND_ZERO_PLUGIN_STRING_ARRAY(plug->added_compls);
}

int yed_unload_plugin(char *plug_name) {
    yed_event                     evt;
    tree_it(yed_plugin_name_t,
            yed_plugin_ptr_t)     it;
    char                         *old_key;
    yed_plugin                   *old_plug;

    memset(&evt, 0, sizeof(evt));
    evt.kind        = EVENT_PLUGIN_PRE_UNLOAD;
    evt.plugin_name = plug_name;

    yed_trigger_event(&evt);
    if (evt.cancel == 1) { return 1; }

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

    evt.kind = EVENT_PLUGIN_POST_UNLOAD;
    yed_trigger_event(&evt);

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

    if (!ys->options.no_init) {
        load_default_init();
    }

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
    char *name_dup;

    yed_set_command(name, command);

    name_dup = strdup(name);
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

    yed_set_style(name, style);
    name_dup = strdup(name);
    array_push(plug->added_styles, name_dup);
}

int yed_plugin_make_ft(yed_plugin *plug, const char *ft_name) {
    char *name_dup;
    int   result;

    result = yed_make_ft((char*)ft_name);
    name_dup = strdup(ft_name);
    array_push(plug->added_fts, name_dup);

    return result;
}

void yed_add_plugin_dir(const char *s) {
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

void yed_plugin_set_completion(yed_plugin *plug, char *name, yed_completion comp) {
    char *name_dup;

    yed_set_completion(name, comp);
    name_dup = strdup(name);
    array_push(plug->added_compls, name_dup);
}

void yed_plugin_set_unload_fn(yed_plugin *plug, yed_plugin_unload_fn_t fn) {
    plug->unload = fn;
}
