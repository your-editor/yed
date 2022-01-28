#define DO_LOG

#define DBG__XSTR(x) #x
#define DBG_XSTR(x) DBG__XSTR(x)
#ifdef DO_LOG
#define DBG(...)                                           \
do {                                                       \
    LOG_FN_ENTER();                                        \
    yed_log(__FILE__ ":" XSTR(__LINE__) ": " __VA_ARGS__); \
    LOG_EXIT();                                            \
} while (0)
#else
#define DBG(...) ;
#endif

#include <yed/plugin.h>
#include <yed/gui.h>

typedef struct plugin_t {
    int     downloaded;
    int     compiled;
    int     installed;
    int     loaded;
    char   *plugin_description;
    char   *version;
    char   *plugin_name;
    array_t keywords;
} plugin;

typedef struct {
    char  *command;
    void (*callback)(void*);
    void  *arg;
    int    exit_code;
} background_task;

static yed_plugin       *SELF;
static char              fetch_script_path[256];
static char              update_script_path[256];
static char              install_script_path[256];
static char              uninstall_script_path[256];
static array_t           tasks;
static array_t           plugin_arr;
static yed_event_handler h_pump;
static yed_event_handler h_plug_load;
static yed_event_handler h_plug_unload;
static yed_event_handler h_line;
static yed_event_handler h_row;
static yed_event_handler h_cur_pre;
static yed_event_handler h_key;
static yed_event_handler h_pre_mod;
static yed_event_handler h_post_mod;
static yed_event_handler h_key;
static yed_event_handler h_mouse;
static int               from_menu;
static int               plug_row;
static int               in_search;
static int               has_search;
static int               cursor_no_recurse;
static int               mod_norecurse;
static int               list_len;
static array_t           update_dds;
static int               update_menu_is_up;
static int               save_update_hz;
static array_t           list_items;
static yed_gui_list_menu list_menu;


/* User facing commands: */
static void ypm_menu(int n_args, char **args);
static void ypm_help(int n_args, char **args);
static void ypm_install(int n_args, char **args);
static void ypm_uninstall(int n_args, char **args);
static void ypm_update(int n_args, char **args);
static void ypm_fetch(int n_args, char **args);

/* Internal functions: */
static int         plug_name_compl(char *str, struct yed_completion_results_t *comp_res);
static void        setup_shell_scripts(void);
static void        free_plugin(plugin *p);
static void        free_bg_task(background_task *task);
static void        add_bg_task(char *command, void (*callback)(void*), void *arg);
static int         pop_bg_task(background_task *task);
static void        fill_plugin_arr(void);
static yed_buffer *get_or_make_buffer(char *buff_name);
static void        load_all_from_list(void);
static void        internal_mod_on(yed_buffer *buff);
static void        internal_mod_off(yed_buffer *buff);
static void        draw_list(void);
static void        draw_menu(void);
static void        update_callback(void *arg);
static void        do_update(void);
static void        fetch_callback(void *arg);
static void        do_fetch(void);
static void        install_callback(void *arg);
static void        do_install(const char *plug_name);
static void        uninstall_callback(void *arg);
static void        do_uninstall(const char *plug_name);
static void        start_search(void);
static void        leave_search(void);
static void        clear_and_leave_search(void);
static void        check_ypm_version(void);
static void        run(void);

/* Event handlers: */
static void pump_handler(yed_event *event);
static void plug_load_handler(yed_event *event);
static void plug_unload_handler(yed_event *event);
static void line_handler(yed_event *event);
static void row_handler(yed_event *event);
static void cursor_pre_move_handler(yed_event *event);
static void key_handler(yed_event *event);
static void pre_mod_handler(yed_event *event);
static void post_mod_handler(yed_event *event);
static void _gui_key_handler(yed_event *event);
static void _gui_mouse_handler(yed_event *event);

static void unload(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    int   manpath_len;
    int   manpath_status;
    char *manpath;
    char *ypm_manpath;
    char  new_manpath[4096];
    int   ret;
    char *item;

    YED_PLUG_VERSION_CHECK();
    SELF = self;
    check_ypm_version();

    tasks            = array_make(background_task);
    plugin_arr       = array_make(plugin);
    list_items = array_make(char*);

    yed_plugin_set_unload_fn(self, unload);

    setup_shell_scripts();

    yed_gui_init_list_menu(&list_menu, list_items);
    list_menu.base.is_up = 0;
    item = "Man Page";  array_push(list_items, item);
    item = "Install";   array_push(list_items, item);
    item = "Uninstall"; array_push(list_items, item);
    item = "Load";      array_push(list_items, item);
    item = "Unload";    array_push(list_items, item);

    load_all_from_list();

    get_or_make_buffer("ypm-menu");
    draw_menu();
    get_or_make_buffer("ypm-output");
    get_or_make_buffer("man-page");

    h_key.kind = EVENT_KEY_PRESSED;
    h_key.fn   = _gui_key_handler;
    yed_plugin_add_event_handler(self, h_key);

    h_mouse.kind = EVENT_KEY_PRESSED;
    h_mouse.fn   = _gui_mouse_handler;
    yed_plugin_add_event_handler(self, h_mouse);

    h_pump.kind = EVENT_PRE_PUMP;
    h_pump.fn   = pump_handler;
    yed_plugin_add_event_handler(self, h_pump);

    h_plug_load.kind = EVENT_PLUGIN_POST_LOAD;
    h_plug_load.fn   = plug_load_handler;
    yed_plugin_add_event_handler(self, h_plug_load);

    h_plug_unload.kind = EVENT_PLUGIN_POST_UNLOAD;
    h_plug_unload.fn   = plug_unload_handler;
    yed_plugin_add_event_handler(self, h_plug_unload);

    h_line.kind = EVENT_LINE_PRE_DRAW;
    h_line.fn   = line_handler;
    yed_plugin_add_event_handler(self, h_line);

    h_row.kind = EVENT_ROW_PRE_CLEAR;
    h_row.fn   = row_handler;
    yed_plugin_add_event_handler(self, h_row);

    h_cur_pre.kind = EVENT_CURSOR_PRE_MOVE;
    h_cur_pre.fn   = cursor_pre_move_handler;
    yed_plugin_add_event_handler(self, h_cur_pre);

    h_key.kind = EVENT_KEY_PRESSED;
    h_key.fn   = key_handler;
    yed_plugin_add_event_handler(self, h_key);

    h_pre_mod.kind = EVENT_BUFFER_PRE_MOD;
    h_pre_mod.fn   = pre_mod_handler;
    yed_plugin_add_event_handler(self, h_pre_mod);

    h_post_mod.kind = EVENT_BUFFER_POST_MOD;
    h_post_mod.fn   = post_mod_handler;
    yed_plugin_add_event_handler(self, h_post_mod);

    yed_plugin_set_command(self, "ypm-fetch",     ypm_fetch);
    yed_plugin_set_command(self, "ypm-update",    ypm_update);
    yed_plugin_set_command(self, "ypm-menu",      ypm_menu);
    yed_plugin_set_command(self, "ypm-install",   ypm_install);
    yed_plugin_set_command(self, "ypm-uninstall", ypm_uninstall);

    yed_plugin_set_completion(self, "ypm-install-compl-arg-0",   plug_name_compl);
    yed_plugin_set_completion(self, "ypm-uninstall-compl-arg-0", plug_name_compl);
    yed_plugin_set_completion(self, "ypm-update-compl-arg-0",    plug_name_compl);


    yed_set_var("ypm-is-updating", "NO");


    manpath = yed_run_subproc("manpath", &manpath_len, &manpath_status);
    if (manpath_status != 0) {
        if (manpath != NULL) { free(manpath); }
        manpath = NULL;
    }

    ypm_manpath = get_config_item_path("ypm/man");

    if (manpath == NULL) {
        setenv("MANPATH", ypm_manpath, 1);
    } else {
        if (strncmp(manpath, ypm_manpath, strlen(ypm_manpath)) != 0) {
            LOG_CMD_ENTER("ypm");
            ret = snprintf(new_manpath, sizeof(new_manpath)-1, "%s:%s", ypm_manpath, manpath);
            if (ret < 0) {
                yed_cerr("item_path was truncated!");
            }
            LOG_EXIT();
            setenv("MANPATH", new_manpath, 1);
        }
    }

    free(ypm_manpath);

    if (manpath != NULL) { free(manpath); }

    return 0;
}

void ypm_fetch(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*ypm-output");

    do_fetch();
}

void ypm_update(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*ypm-output");

    do_update();
}

void ypm_menu(int n_args, char **args) {
    draw_menu();
    YEXE("special-buffer-prepare-focus", "*ypm-menu");
    YEXE("buffer", "*ypm-menu");
}

void ypm_install(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*ypm-output");

    do_install(args[0]);
}

void ypm_uninstall(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*ypm-output");

    do_uninstall(args[0]);
}



void create_update_menu(int version) {
    yed_attrs          attrs;
    int                width;
    char               buff[256];
    const char        *msg1;
    char               msg2[256];
    const char        *msg3;
    yed_direct_draw_t *dd;

    update_dds = array_make(yed_direct_draw_t*);

    attrs = yed_active_style_get_active();
    attrs.flags ^= ATTR_INVERSE;
    width = 48;
    memset(buff, 0, sizeof(buff));
    memset(buff, ' ', width);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    msg1 =
version ? "YPM is out of date." : "YPM has not been initialized.";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg1) / 2), msg1, strlen(msg1));
    dd = yed_direct_draw((ys->term_rows / 2) - 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) - 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

        snprintf(msg2, sizeof(msg2),
"(yed version: %d  vs  YPM version %d)", YED_VERSION, version);

    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg2) / 2), msg2, strlen(msg2));
    dd = yed_direct_draw((ys->term_rows / 2) + 0, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 1, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    msg3 =
"Would you like to update? [y/n]";
    memset(buff, ' ', width);
    memcpy(buff + (width / 2) - (strlen(msg3) / 2), msg3, strlen(msg3));
    dd = yed_direct_draw((ys->term_rows / 2) + 2, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    memset(buff, ' ', width);
    dd = yed_direct_draw((ys->term_rows / 2) + 3, (ys->term_cols / 2) - (width / 2),
                         attrs, buff);
    array_push(update_dds, dd);

    update_menu_is_up = 1;
}


static void check_ypm_version(void) {
    int   version;
    char  cmd[1024];
    char *output;
    int   output_len;
    int   status;

    version = 0;

    snprintf(cmd, sizeof(cmd), "cd %s/ypm && git rev-parse --abbrev-ref HEAD", get_config_path());
    output = yed_run_subproc(cmd, &output_len, &status);
    if (output != NULL && strlen(output) != 0 && status == 0) {
        if (!sscanf(output, "v%d", &version)) {
            version = 0;
        }
    }

    if (version == -1) {
        LOG_FN_ENTER();
        yed_log("[!] failed trying to run '%s'! Do you have git installed?", cmd);
        LOG_EXIT();
    } else {
        if ((YED_VERSION / 100) != (version / 100)) {
            create_update_menu(version);
        }
    }
}

static int plug_name_compl(char *str, struct yed_completion_results_t *comp_res){
    int      ret;
    array_t  a;
    plugin  *it;

    ret = 0;

    a = array_make(char *);
    array_traverse(plugin_arr, it) {
        array_push(a, (*it).plugin_name);
    }

    FN_BODY_FOR_COMPLETE_FROM_ARRAY(str, array_len(a), (char **)array_data(a), comp_res, ret);

    array_free(a);

    return ret;
}

/* Inline BASH Scripts */

static const char *fetch_script =
"#!/usr/bin/env bash\n"
"if ! [ -d %s/ypm ]; then\n"
"    echo 'YPM not found. Please run ypm-update.'\n"
"    exit 1\n"
"else\n"
"    cd %s/ypm\n"
"    git pull || exit $?\n"
"    echo \"Done.\"\n"
"fi\n";

static const char *update_script =
"#!/usr/bin/env bash\n"
"if ! [ -d %s/ypm ]; then\n"
"    git clone https://github.com/your-editor/ypm-plugins %s/ypm\n"
"    if ! [ $? ]; then\n"
"        echo 'Clone failed.'\n"
"        exit 1\n"
"    else\n"
"        cd %s/ypm\n"
"        git checkout v" YED_MAJOR_VERSION_STR " || exit $?\n"
"        mkdir %s/ypm/plugins\n"
"        echo \"Cloned plugins repo.\"\n"
"        echo \"Done.\"\n"
"    fi\n"
"else\n"
"    cd %s/ypm\n"
"    git pull || exit $?\n"
"    git checkout v" YED_MAJOR_VERSION_STR " || exit $?\n"
"    echo \"Done.\"\n"
"fi\n";

static const char *install_script =
"#!/usr/bin/env bash\n"
"if [ -d %s/ypm ] && [ ! -z $PLUGIN ]; then\n"
"    cd %s/ypm/ypm_plugins\n"
"    git submodule init $PLUGIN\n"
"    if ! [ $? ]; then\n"
"        echo 'Init failed.'\n"
"        exit 1\n"
"    else\n"
"        echo $PLUGIN' initialized.'\n"
"    fi\n"
"    git submodule update $PLUGIN\n"
"    if ! [ $? ]; then\n"
"        echo 'Update failed.'\n"
"        exit 1\n"
"    else\n"
"        echo $PLUGIN' pulled.'\n"
"    fi\n"
"    cd $PLUGIN\n"
"    bash build.sh\n"
"    if ! [ $? ]; then\n"
"        echo 'Build failed.'\n"
"        exit 1\n"
"    else\n"
"        echo $PLUGIN' built.'\n"
"    fi\n"
"    mkdir -p %s/ypm/plugins/$(dirname $PLUGIN)\n"
"    mv $(basename $PLUGIN.so) %s/ypm/plugins/${PLUGIN}.so.new\n"
"    mv %s/ypm/plugins/${PLUGIN}.so.new %s/ypm/plugins/${PLUGIN}.so\n"
"else\n"
"    echo 'Run ypm-update.'\n"
"fi\n";

static const char *uninstall_script =
"#!/usr/bin/env bash\n"
"if [ -d %s/ypm ] && [ ! -z $PLUGIN ]; then\n"
"    cd %s/ypm/ypm_plugins\n"
"    git submodule deinit -f $PLUGIN || exit $?\n"
"    if ! [ $? ]; then\n"
"        echo 'Deinit failed.'\n"
"        exit 1\n"
"    else\n"
"        echo $PLUGIN' deinitialized.'\n"
"    fi\n"
"    rm %s/ypm/plugins/${PLUGIN}.so\n"
"    if ! [ $? ]; then\n"
"        echo 'rm failed.'\n"
"        exit 1\n"
"    else\n"
"        echo $PLUGIN' removed.'\n"
"    fi\n"
"fi\n";

static void setup_shell_scripts(void) {
    char *user;
    char  buff[128];
    FILE *f;

    user = getenv("USER");
    if (user == NULL) { user = ""; }

    snprintf(fetch_script_path,     sizeof(buff), "/tmp/ypm-fetch-%s.sh",     user);
    snprintf(update_script_path,    sizeof(buff), "/tmp/ypm-update-%s.sh",    user);
    snprintf(install_script_path,   sizeof(buff), "/tmp/ypm-install-%s.sh",   user);
    snprintf(uninstall_script_path, sizeof(buff), "/tmp/ypm-uninstall-%s.sh", user);

    f = fopen(fetch_script_path, "w");
    if (f == NULL) { goto out; }
    fprintf(f, fetch_script,
            get_config_path(),
            get_config_path());
    fclose(f);

    f = fopen(update_script_path, "w");
    if (f == NULL) { goto out; }
    fprintf(f, update_script,
            get_config_path(),
            get_config_path(),
            get_config_path(),
            get_config_path(),
            get_config_path());
    fclose(f);

    f = fopen(install_script_path, "w");
    if (f == NULL) { goto out; }
    fprintf(f, install_script,
            get_config_path(),
            get_config_path(),
            get_config_path(),
            get_config_path(),
            get_config_path(),
            get_config_path());
    fclose(f);

    f = fopen(uninstall_script_path, "w");
    if (f == NULL) { goto out; }
    fprintf(f, uninstall_script,
            get_config_path(),
            get_config_path(),
            get_config_path());
    fclose(f);

out:;
}

static void free_plugin(plugin *p) {
    free(p->plugin_name);
    if (p->version != NULL)            { free(p->version);            }
    if (p->plugin_description != NULL) { free(p->plugin_description); }
    free_string_array(p->keywords);
}

static void free_bg_task(background_task *task) { free(task->command); }

static void add_bg_task(char *command, void (*callback)(void*), void *arg) {
    background_task t;

    t.command  = strdup(command);
    t.callback = callback;
    t.arg      = arg;


    array_insert(tasks, 0, t);
}

static int pop_bg_task(background_task *task) {
    background_task *t;

    t = array_last(tasks);
    if (t == NULL) { return 0; }

    memcpy(task, t, sizeof(*task));

    array_pop(tasks);

    return 1;
}

static void add_plugin_to_arr(const char *ab_path, const char *rel_path, int installed) {
    plugin                                         plug;
    char                                           item_path[4096];
    struct stat                                    st;
    const char                                    *just_name;
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t)   pit;
    char                                          *man_dir_path;
    char                                           man_path[4096];
    char                                           name_copy[512];
    char                                          *s;
    FILE                                          *f;
    char                                           line[512];
    char                                          *cpy;
    int                                            ret;

    memset(&plug, 0, sizeof(plug));

    plug.plugin_name = path_without_ext((char*)rel_path);

    LOG_CMD_ENTER("ypm");
    /* downloaded */
    ret = snprintf(item_path, sizeof(item_path), "%s/build.sh", ab_path);
    if(ret < 0) {
        yed_cerr("item_path was truncated!");
    }

    if (stat(item_path, &st) == 0) {
        plug.downloaded = 1;
    }

    just_name = get_path_basename(plug.plugin_name);

    //compiled
    ret = snprintf(item_path, sizeof(item_path), "%s/%s.so", ab_path, just_name);
    if(ret < 0) {
        yed_cerr("item_path was truncated!");
    }
    if (stat(item_path, &st) == 0) {
        plug.compiled = 1;
    }

    //installed
    plug.installed = installed;

    //loaded
    snprintf(line, sizeof(line), "ypm/plugins/%s", plug.plugin_name);
    pit = tree_lookup(ys->plugins, line);
    if (tree_it_good(pit)) {
        plug.loaded = 1;
    }

    //Dir path name
    plug.keywords = array_make(char *);
    s = strdup(plug.plugin_name);
    array_push(plug.keywords, s);
    man_dir_path = get_config_item_path("ypm/man/man7");
    if (strncmp(plug.plugin_name, "styles/", strlen("styles/")) == 0) {
        snprintf(name_copy, sizeof(name_copy), "style-%s", plug.plugin_name + strlen("styles/"));
    } else {
        snprintf(name_copy, sizeof(name_copy), "%s", plug.plugin_name);
    }
    s = name_copy;
    while (*s) { if (*s == '/') { *s = '-'; } s += 1; }
    snprintf(man_path, sizeof(man_path), "%s/%s.7", man_dir_path, name_copy);

    free(man_dir_path);

    //version / keywords
    f = fopen(man_path, "r");
    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (strstr(line, "VERSION") != NULL) {
                if (fgets(line, sizeof(line), f) != NULL) {
                    if (line[strlen(line)-1] == '\n') {
                        line[strlen(line)-1] = 0;
                    }
                    plug.version = strdup(line);
                }
            } else if (strstr(line, "KEYWORDS") != NULL) {
                if (fgets(line, sizeof(line), f) != NULL) {
                    s = strtok(line, ", ");
                    do {
                        while (isspace(*s)) { s += 1; }
                        if (strlen(s) > 0) {
                            if (s[strlen(s)-1] == '\n') {
                                s[strlen(s)-1] = 0;
                            }
                            cpy = strdup(s);
                            array_push(plug.keywords, cpy);
                        }
                    } while ((s = strtok(NULL, ", ")));
                }
            } else if (strstr(line, "NAME") != NULL) {
                if (fgets(line, sizeof(line), f) != NULL) {
                    s = strstr(line, "\\- ");
                    if (s != NULL) {
                        s += strlen("\\- ");
                        if (s[strlen(s)-1] == '\n') {
                            s[strlen(s)-1] = 0;
                        }
                        plug.plugin_description = strdup(s);
                    }
                }
            }
        }
        fclose(f);
    }

    array_push(plugin_arr, plug);
    LOG_EXIT();
}

static void clear_plugin_arr(void) {
    plugin *pit;

    array_traverse(plugin_arr, pit) {
        free_plugin(pit);
    }

    array_clear(plugin_arr);
}

static void fill_plugin_arr(void) {
    array_t   listed_plugins;
    char     *path;
    FILE     *f;
    char      line[512];
    char     *cpy;
    char      plug_path[2048];
    char     *s;
    int       installed;
    char    **lit;
    char      plug_full_path[4096];

    listed_plugins = array_make(char*);

    path = get_config_item_path("ypm_list");
    f = fopen(path, "r");

    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (*line && line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = 0;
            }
            if (strlen(line) == 0) { continue; }

            cpy = strdup(line);
            array_push(listed_plugins, cpy);
        }
        fclose(f);
    }

    free(path);


    clear_plugin_arr();

    path = get_config_item_path("ypm/.gitmodules");
    f = fopen(path, "r");

    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (strncmp(line, "[submodule", strlen("[submodule")) != 0) { continue; }
            if (sscanf(line, "[submodule \"ypm_plugins/%s", plug_path)) {
                s = plug_path;
                while (*s && *s != '"') { s += 1; } *s = 0;

                plug_full_path[0] = 0;
                strcat(plug_full_path, get_config_path());
                strcat(plug_full_path, "/ypm/ypm_plugins/");
                strcat(plug_full_path, plug_path);

                installed = 0;
                array_traverse(listed_plugins, lit) {
                    if (strcmp(plug_path, *lit) == 0) {
                        installed = 1;
                        break;
                    }
                }

                add_plugin_to_arr(plug_full_path, plug_path, installed);
            }
        }

        fclose(f);
    }

    free(path);

    free_string_array(listed_plugins);
}

static yed_buffer *get_or_make_buffer(char *buff_name) {
    yed_buffer *buff;
    char buff_name_final[512];

    buff_name_final[0] = 0;
    strcat(buff_name_final, "*");
    strcat(buff_name_final, buff_name);

    buff = yed_get_buffer(buff_name_final);
    if (buff == NULL) {
        buff = yed_create_buffer(buff_name_final);
        buff->flags |= BUFF_SPECIAL | BUFF_RD_ONLY;
    }

    return buff;
}


static void do_plugin_load(char *name) {
    char name_buff[512];

    snprintf(name_buff, sizeof(name_buff), "ypm/plugins/%s", name);

    YEXE("plugin-load", name_buff);
}

static void do_plugin_unload(char *name) {
    char name_buff[512];

    snprintf(name_buff, sizeof(name_buff), "ypm/plugins/%s", name);

    YEXE("plugin-unload", name_buff);
}


static void load_all_from_list(void) {
    array_t   plugs;
    char     *path;
    FILE     *f;
    char      line[512];
    char     *s;
    char    **it;

    plugs = array_make(char*);

    path = get_config_item_path("ypm_list");
    f = fopen(path, "r");

    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (line[strlen(line)-1] == '\n') {
                line[strlen(line)-1] = 0;
            }
            if (strlen(line) == 0) { continue; }
            s = strdup(line);
            array_push(plugs, s);
        }
        fclose(f);
    }

    free(path);

    array_traverse(plugs, it) {
        do_plugin_load(*it);
    }

    free_string_array(plugs);
}

static void internal_mod_on(yed_buffer *buff) {
    buff->flags &= ~BUFF_RD_ONLY;
}

static void internal_mod_off(yed_buffer *buff) {
    buff->flags |= BUFF_RD_ONLY;
}

static int plugin_has_keywords(plugin *plug) {
    yed_line  *line;
    array_t    search_words;
    int        match;
    int        found;
    char     **sit;
    char     **kit;

    if (!has_search) { return 1; }

    line = yed_buff_get_line(get_or_make_buffer("ypm-menu"), 15);
    if (line == NULL) { return 0; }

    array_zero_term(line->chars);

    search_words = sh_split((char*)array_data(line->chars));

    match = 1;
    array_traverse(search_words, sit) {
        found = 0;
        array_traverse(plug->keywords, kit) {
            if (strncmp(*kit, *sit, strlen(*sit)) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) { match = 0; goto out; }
    }

out:;
    free_string_array(search_words);

    return match;
}

static int plug_name_cmp(const void *_a, const void *_b) {
    const plugin *a;
    const plugin *b;

    a = _a;
    b = _b;

    return strcmp(a->plugin_name, b->plugin_name);
}

static void draw_list(void) {
    yed_buffer *buff;
    array_t     plugs;
    plugin     *it;
    yed_attrs   attr;
    int         start_row;
    int         max_width;
    int         max_desc_width;
    char        line_buff[1024];
    int         col_2_width;
    int         col_3_width;
    int         i;
    char        dash1_buff[128];
    char        dash2_buff[128];
    char        dash3_buff[512];
    char        plugin_line[512];

    buff      = get_or_make_buffer("ypm-menu");
    start_row = 17;

    mod_norecurse = 1;
    internal_mod_on(buff);

    while (yed_buff_n_lines(buff) >= start_row) {
        yed_buff_delete_line_no_undo(buff, start_row);
    }

    max_width = 6;
    max_desc_width = 12;

    fill_plugin_arr();

    plugs = array_make(plugin);
    array_traverse(plugin_arr, it) {
        if (plugin_has_keywords(it)) {
            array_push(plugs, *it);
        }
    }


    array_traverse(plugin_arr, it) {
        max_width = MAX(max_width, yed_get_string_width(it->plugin_name));
        if (it->plugin_description != NULL) {
            max_desc_width = MAX(max_desc_width, yed_get_string_width(it->plugin_description));
        }
    }

    col_2_width = 9;
    col_3_width = MIN((sizeof(dash3_buff) / strlen("─")) - 1, max_desc_width + 1);
    snprintf(line_buff, sizeof(line_buff), "%-*s │ %-*s │ %-*s │ %-*s│",
                        max_width,   "Plugin",
                        col_2_width, "Installed",
                        col_2_width, "Loaded",
                        col_3_width, "Description"
            );
    yed_buff_insert_string_no_undo(buff, line_buff, start_row, 1);
    start_row += 1;

    dash1_buff[0] = 0;
    for (i = 0; i < max_width + 1; i += 1)   { strcat(dash1_buff, "─"); }
    dash2_buff[0] = 0;
    for (i = 0; i < col_2_width + 2; i += 1) { strcat(dash2_buff, "─"); }
    dash3_buff[0] = 0;
    for (i = 0; i < col_3_width + 1; i += 1) { strcat(dash3_buff, "─"); }

    LOG_CMD_ENTER("ypm");
    snprintf(line_buff, sizeof(line_buff), "%*s┼%*s┼%*s┼%*s┤",
                        max_width + 1,   dash1_buff,
                        col_2_width + 2, dash2_buff,
                        col_2_width + 2, dash2_buff,
                        col_3_width + 1, dash3_buff
            );
    LOG_EXIT();
    yed_buff_insert_string_no_undo(buff, line_buff, start_row, 1);

    qsort(array_data(plugs), array_len(plugs), sizeof(plugin), plug_name_cmp);

    array_traverse(plugs, it) {
        start_row++;
        plugin_line[0] = 0;
        sprintf(plugin_line, "%-*s │ %-*s │ %-*s │ %-*s│",
                        max_width,   (*it).plugin_name,
                        ((*it).installed == 1) ? col_2_width+2 : col_2_width, ((*it).installed == 1) ? "\xE2\x9C\x93" : "X",
                        ((*it).loaded == 1) ? col_2_width+2 : col_2_width, ((*it).loaded == 1) ? "\xE2\x9C\x93" : "X",
                        max_desc_width + 1, (*it).plugin_description == NULL ? "<no description>" : (*it).plugin_description
                );
        attr.flags = ATTR_16;
        attr.fg = ATTR_16_RED;
        attr.bg = ATTR_16_BLACK;
        yed_set_attr(attr);
        yed_buff_insert_string_no_undo(buff, plugin_line, start_row, 1);
    }

    list_len = array_len(plugs);

    yed_buffer_add_line_no_undo(buff);

    internal_mod_off(buff);
    mod_norecurse = 0;


    array_free(plugs);
}

static void draw_menu(void) {
    yed_buffer *buff;
    const char *logo =
"      ___           ___           ___     \n"
"     |\\__\\         /\\  \\         /\\__\\    \n"
"     |:|  |       /::\\  \\       /::|  |   \n"
"     |:|  |      /:/\\:\\  \\     /:|:|  |   \n"
"     |:|__|__   /::\\~\\:\\  \\   /:/|:|__|__ \n"
"     /::::\\__\\ /:/\\:\\ \\:\\__\\ /:/ |::::\\__\\\n"
"    /:/~~/~    \\/__\\:\\/:/  / \\/__/~~/:/  /\n"
"   /:/  /           \\::/  /        /:/  / \n"
"   \\/__/             \\/__/        /:/  /  \n"
"                                 /:/  /   \n"
"                                 \\/__/    \n";

    buff = get_or_make_buffer("ypm-menu");

    mod_norecurse = 1;
    internal_mod_on(buff);

    yed_buff_clear_no_undo(buff);

    yed_buff_insert_string_no_undo(buff, logo, 1, 1);
    yed_buff_insert_string_no_undo(buff, "Press 'f' to start searching by keyword, ESC to clear, 'm' to view the man page for YPM.", yed_buff_n_lines(buff) + 1, 1);
    yed_buff_insert_string_no_undo(buff, "search:", yed_buff_n_lines(buff) + 2, 1);

    internal_mod_off(buff);
    mod_norecurse = 0;

    draw_list();
}


static long long update_count;

typedef struct {
    long long  count;
    char      *name;
} update_install_arg;

static void update_install_callback(void *_arg) {
    update_install_arg *arg;

    arg = _arg;

LOG_CMD_ENTER("ypm");

    do_plugin_load(arg->name);

    if (arg->count == update_count) {
        yed_cprint("update finished");
        yed_set_update_hz(save_update_hz);
        yed_set_var("ypm-is-updating","NO");
        draw_list();
        YEXE("buffer", "*ypm-menu");
    } else {
        yed_cprint("updated plugin %lld/%lld", arg->count, update_count);
    }

LOG_EXIT();
}

static void fetch_callback(void *_arg) {
    draw_list();

    yed_cprint("fetch finished");
    YEXE("buffer", "*ypm-menu");
}

static void update_callback(void *_arg) {
    int                 doing_install;
    plugin             *pit;
    char                buff[512];
    update_install_arg *arg;

    draw_list();

    doing_install = 0;
    update_count  = 0;
    array_traverse(plugin_arr, pit) {
        if (pit->installed) {
            doing_install  = 1;
            update_count  += 1;
            snprintf(buff, sizeof(buff), "PLUGIN='%s' bash %s 2>&1", pit->plugin_name, install_script_path);
            arg = malloc(sizeof(*arg));
            arg->count = update_count;
            arg->name  = strdup(pit->plugin_name);
            add_bg_task(buff, update_install_callback, (void*)arg);
        }
    }

    if (!doing_install) {
        yed_set_var("ypm-is-updating","NO");
LOG_CMD_ENTER("ypm");
        yed_cprint("update finished");
LOG_EXIT();
        YEXE("buffer", "*ypm-menu");
    }
}

static void do_fetch(void) {
    char buff[512];

    YEXE("buffer", "*ypm-output");
    if (ys->active_frame != NULL) {
        yed_set_cursor_within_frame(ys->active_frame, 1, 1);
    }

    snprintf(buff, sizeof(buff), "bash %s 2>&1", fetch_script_path);

    add_bg_task(buff, fetch_callback, NULL);
}

static void do_update(void) {
    char buff[512];

    YEXE("buffer", "*ypm-output");
    if (ys->active_frame != NULL) {
        yed_set_cursor_within_frame(ys->active_frame, 1, 1);
    }

    snprintf(buff, sizeof(buff), "bash %s 2>&1", update_script_path);

    yed_set_var("ypm-is-updating","YES");
    save_update_hz = yed_get_update_hz();
    yed_set_update_hz(10);
    add_bg_task(buff, update_callback, NULL);
}

static void install_callback(void *arg) {
    char     *plug_name;
    array_t   plugs;
    char     *path;
    FILE     *f;
    char      line[512];
    char     *s;
    char    **it;

    plug_name = arg;

    plugs = array_make(char*);

    path = get_config_item_path("ypm_list");
    f = fopen(path, "r");

    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (line[strlen(line)-1] == '\n') {
                line[strlen(line)-1] = 0;
            }
            if (strcmp(plug_name, line) == 0) { continue; }
            s = strdup(line);
            array_push(plugs, s);
        }
        fclose(f);
    }

    s = strdup(arg);
    array_push(plugs, s);

    f = fopen(path, "w");
    if (f != NULL) {
        array_traverse(plugs, it) {
            fprintf(f, "%s\n", *it);
        }
        fclose(f);
    }

    free(path);

    free_string_array(plugs);

    do_plugin_load(plug_name);
    free(arg);

    draw_list();

    YEXE("buffer", "*ypm-menu");
    yed_set_cursor_within_frame(ys->active_frame, plug_row, 1);
}

static void do_install(const char *plug_name) {
    char buff[512];

    YEXE("buffer", "*ypm-output");
    if (ys->active_frame != NULL) {
        yed_set_cursor_within_frame(ys->active_frame, 1, 1);
    }

    snprintf(buff, sizeof(buff), "PLUGIN='%s' bash %s 2>&1", plug_name, install_script_path);

    add_bg_task(buff, install_callback, strdup(plug_name));
}

static void uninstall_callback(void *arg) {
    char     *plug_name;
    array_t   plugs;
    char     *path;
    FILE     *f;
    char      line[512];
    char     *s;
    char    **it;

    plug_name = arg;

    plugs = array_make(char*);

    path = get_config_item_path("ypm_list");
    f = fopen(path, "r");

    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (line[strlen(line)-1] == '\n') {
                line[strlen(line)-1] = 0;
            }
            if (strcmp(plug_name, line) == 0) { continue; }
            s = strdup(line);
            array_push(plugs, s);
        }
        fclose(f);
    }

    f = fopen(path, "w");
    if (f != NULL) {
        array_traverse(plugs, it) {
            fprintf(f, "%s\n", *it);
        }
        fclose(f);
    }

    free(path);

    free_string_array(plugs);

    do_plugin_unload(plug_name);
    free(arg);

    draw_list();

    YEXE("buffer", "*ypm-menu");
    yed_set_cursor_within_frame(ys->active_frame, plug_row, 1);
}

static void do_uninstall(const char *plug_name) {
    char buff[512];

    YEXE("buffer", "*ypm-output");
    if (ys->active_frame != NULL) {
        yed_set_cursor_within_frame(ys->active_frame, 1, 1);
    }

    snprintf(buff, sizeof(buff), "PLUGIN='%s' bash %s 2>&1", plug_name, uninstall_script_path);

    add_bg_task(buff, uninstall_callback, strdup(plug_name));
}

static void open_man_page(const char *page) {
    char  page_copy[256];
    char *s;
    int   width;
    char  pre_cmd_buff[1024];
    char  cmd_buff[1024];
    char  name_copy[512];
    char *output;
    int   status;

    snprintf(page_copy, sizeof(page_copy), "%s", page);
    for (s = page_copy; *s; s += 1) { if (*s == '/') { *s = '-'; } }

    if (strncmp(page_copy, "styles-", strlen("styles-")) == 0) {
        snprintf(name_copy, sizeof(name_copy), "style-%s", page_copy + strlen("styles-"));
    } else {
        snprintf(name_copy, sizeof(name_copy), "%s", page_copy);
    }

    pre_cmd_buff[0] = 0;
    cmd_buff[0]     = 0;

    snprintf(pre_cmd_buff, sizeof(pre_cmd_buff), "man 7 %s | col -bx; exit ${PIPESTATUS[0]} 2>/dev/null", name_copy);

    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, " >/dev/null");


    snprintf(cmd_buff, sizeof(cmd_buff), "bash -c '");
    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, "'");

    output = yed_run_subproc(cmd_buff, NULL, &status);

    if (output != NULL) { free(output); }

    if (status != 0) {
        yed_cerr("command '%s' failed", pre_cmd_buff);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*man-page");
    if (ys->active_frame != NULL) {
        width = ys->active_frame->width;
    } else {
        width = 80;
    }

    snprintf(cmd_buff, sizeof(cmd_buff),
             "bash -c 'MANWIDTH=%d ", width);
    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, "'");

    get_or_make_buffer("man-page")->flags &= ~BUFF_RD_ONLY;

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buffer("man-page"), &status) != 0) {
        get_or_make_buffer("man-page")->flags |= BUFF_RD_ONLY;
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        return;
    }
    if (status != 0) {
        get_or_make_buffer("man-page")->flags |= BUFF_RD_ONLY;
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        return;
    }

    get_or_make_buffer("man-page")->flags |= BUFF_RD_ONLY;

    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*man-page");
}

static void start_search(void) {
    yed_buffer *buff;
    yed_line   *line;
    int         returning;

    if (in_search) { return; }

    buff = get_or_make_buffer("ypm-menu");
    line = yed_buff_get_line(buff, 15);
    if (line == NULL) { return; }

    internal_mod_on(buff);

    array_zero_term(line->chars);

    returning = 1;
    if (strcmp((char*)array_data(line->chars), "search:") == 0) {
        yed_line_clear_no_undo(buff, 15);
        returning = 0;
    }

    if (ys->active_frame != NULL
    &&  ys->active_frame->buffer == buff) {
        cursor_no_recurse = 1;
        yed_set_cursor_within_frame(ys->active_frame, 1, 1);
        cursor_no_recurse = 0;
    }

    in_search = has_search = 1;

    if (ys->active_frame != NULL
    &&  ys->active_frame->buffer == buff) {
        yed_set_cursor_within_frame(ys->active_frame, 15, returning ? 999 : 1);
    }
}

static void go_first_line(void) {
    cursor_no_recurse = 1;
    yed_set_cursor_within_frame(ys->active_frame, 19, 1);
    cursor_no_recurse = 0;
}

static void leave_search(void) {
    if (!in_search) { return; }
    in_search = 0;
    internal_mod_off(get_or_make_buffer("ypm-menu"));
    if (ys->active_frame != NULL
    &&  ys->active_frame->buffer == get_or_make_buffer("ypm-menu")) {
        go_first_line();
    }
}

static void clear_and_leave_search(void) {
    if (!in_search) { return; }
    in_search = 0;
    yed_line_clear_no_undo(get_or_make_buffer("ypm-menu"), 15);
    yed_buff_insert_string_no_undo(get_or_make_buffer("ypm-menu"), "search:", 15, 1);
    internal_mod_off(get_or_make_buffer("ypm-menu"));
    if (ys->active_frame != NULL
    &&  ys->active_frame->buffer == get_or_make_buffer("ypm-menu")) {
        go_first_line();
    }
    has_search = 0;
}


static void unload(yed_plugin *self) {
    background_task *tit;
    plugin          *pit;

    array_traverse(tasks, tit) { free_bg_task(tit); }
    array_free(tasks);
    array_traverse(plugin_arr, pit) { free_plugin(pit); }
    array_free(plugin_arr);
    array_free(list_items);
/*     array_free(popup_items_sm); */

    yed_free_buffer(get_or_make_buffer("ypm-output"));
    yed_free_buffer(get_or_make_buffer("ypm-menu"));
}

static int              task_running;
background_task         task;
static yed_nb_subproc_t nb_subproc;

static void pump_handler(yed_event *event) {
LOG_CMD_ENTER("ypm");
    internal_mod_on(get_or_make_buffer("ypm-output"));

    if (task_running) {
try:;
        task_running = yed_read_subproc_into_buffer_nb(&nb_subproc);

        if (!task_running) {
            if (nb_subproc.err && nb_subproc.err != ECHILD) {
                yed_cerr("something went wrong -- errno = %d\n", nb_subproc.err);
            } else {
                if (nb_subproc.exit_status != 0 || nb_subproc.err == ECHILD) {
                    yed_cerr("the command '%s' failed with exit code %d",
                             task.command,
                             nb_subproc.exit_status);
                }
            }

            task.callback(task.arg);

            free_bg_task(&task);
            memset(&task, 0, sizeof(task));
        }
    } else if (pop_bg_task(&task)) {
        if (yed_start_read_subproc_into_buffer_nb(task.command, get_or_make_buffer("ypm-output"), &nb_subproc)) {
            yed_cerr("there was an error when calling yed_start_read_subproc_into_buffer_nb()");
            goto out;
        }
        task_running = 1;
        /*
        ** Wait 15 milliseconds so that the subprocess has time to start and
        ** do _something_.
        ** This makes it so that we can start to produce output before the next
        ** pump (which could be ~300 ms if there aren't any keypresses).
        */
        usleep(15000);
        goto try;
    }

out:;
    internal_mod_off(get_or_make_buffer("ypm-output"));
LOG_EXIT();
}

static void plug_load_handler(yed_event *event)   { draw_list(); }
static void plug_unload_handler(yed_event *event) { draw_list(); }

static void line_handler(yed_event *event) {
    yed_buffer *buff;
    yed_attrs   attr;
    yed_attrs  *ait;
    int         i;
    yed_glyph  *git;
    yed_line   *line;
    int         table_col;
    char        chk[] = "\xE2\x9C\x93";

    buff = get_or_make_buffer("ypm-menu");
    if (event->frame->buffer != buff) { return; }

    if (event->row <= 11) {
        attr = yed_active_style_get_code_string();

        array_traverse(event->line_attrs, ait) {
            yed_combine_attrs(ait, &attr);
        }
    } else if (event->row == 13) {
        attr = yed_active_style_get_code_keyword();

        ait = array_item(event->line_attrs, 8 - 1);
        yed_combine_attrs(ait, &attr);

        ait = array_item(event->line_attrs, 42 - 1);
        yed_combine_attrs(ait, &attr);
        ait = array_item(event->line_attrs, 43 - 1);
        yed_combine_attrs(ait, &attr);
        ait = array_item(event->line_attrs, 44 - 1);
        yed_combine_attrs(ait, &attr);

        ait = array_item(event->line_attrs, 57 - 1);
        yed_combine_attrs(ait, &attr);
    } else if (event->row == 17) {
        attr = yed_active_style_get_code_keyword();

        i = 1;
        array_traverse(event->line_attrs, ait) {
            git = yed_buff_get_glyph(buff, event->row, i);
            if (isalpha(git->c)) {
                yed_combine_attrs(ait, &attr);
            }
            i += 1;
        }
    } else if (event->row >= 19) {
        line = yed_buff_get_line(buff, event->row);
        array_zero_term(line->chars);

        table_col = 1;
        i         = 1;

        array_traverse(event->line_attrs, ait) {
            git = yed_buff_get_glyph(buff, event->row, i);

            if (strncmp(&git->c, "│", strlen("│")) == 0) { table_col += 1; goto next; }

            if (table_col == 1) {
                if (!isspace(git->c)) {
                    attr = yed_active_style_get_code_fn_call();
                    yed_combine_attrs(ait, &attr);
                }
            } else if (table_col == 2 || table_col == 3) {
                if (strncmp(&git->c, chk, strlen(chk)) == 0) {
                    attr    = yed_active_style_get_active();
                    attr.bg = 0;
                    if (attr.flags & ATTR_RGB) {
                        attr.fg = RGB_32(0x0, 0x7f, 0x0);
                    } else if (attr.flags & ATTR_256) {
                        attr.fg = 34;
                    } else {
                        attr.fg = ATTR_16_GREEN;
                    }
                    yed_combine_attrs(ait, &attr);
                } else if (git->c == 'X') {
                    attr = yed_active_style_get_attention();
                    yed_combine_attrs(ait, &attr);
                }
            }

next:;
            i += 1;
        }
    }
}

static void row_handler(yed_event *event) {
    yed_attrs assoc;

    if (event->frame->buffer != get_or_make_buffer("ypm-menu")) { return; }
    if (event->row != 15) { return; }

    assoc = yed_active_style_get_associate();

    event->row_base_attr = assoc;
}


static void cursor_pre_move_handler(yed_event *event) {
    if (event->frame->buffer != get_or_make_buffer("ypm-menu")) {
        leave_search();
        return;
    }

    if (in_search) {
        if (event->new_row != 15) { event->cancel = 1; }
    } else {
        if (list_menu.base.is_up) {
            event->cancel = 1;
        }

        if (cursor_no_recurse) { return; }

        if (event->new_row < 19) {
            if (event->frame->buffer_y_offset > 0) {
                cursor_no_recurse = 1;
                yed_set_cursor_within_frame(event->frame, event->frame->buffer_y_offset + event->frame->scroll_off, 1);
                cursor_no_recurse = 0;
            }
            cursor_no_recurse = 1;
            yed_set_cursor_within_frame(event->frame, 19, event->new_col);
            cursor_no_recurse = 0;
            event->cancel = 1;
        } else if (event->new_row >= yed_buff_n_lines(get_or_make_buffer("ypm-menu"))) {
            cursor_no_recurse = 1;
            yed_set_cursor_within_frame(event->frame, yed_buff_n_lines(get_or_make_buffer("ypm-menu")) - (list_len > 0), event->new_col);
            cursor_no_recurse = 0;
            event->cancel = 1;
        }
    }
}


static void key_handler(yed_event *event) {
    yed_direct_draw_t **dit;

    if (ys->interactive_command != NULL) { return; }

    if (update_menu_is_up) {
        event->cancel = 1;
        if (event->key == 'y' || event->key == 'Y') {
            update_menu_is_up = 0;
            array_traverse(update_dds, dit) {
                yed_kill_direct_draw(*dit);
            }
            array_free(update_dds);
            YEXE("ypm-update");
        } else if(event->key == 'n' || event->key == 'N') {
            update_menu_is_up = 0;
            array_traverse(update_dds, dit) {
                yed_kill_direct_draw(*dit);
            }
            array_free(update_dds);
            yed_plugin_uninstall_features(SELF);
            yed_free_buffer(get_or_make_buffer("ypm-menu"));
            yed_free_buffer(get_or_make_buffer("ypm-output"));
        }

        return;
    }

    if (ys->active_frame == NULL) { return; }

LOG_CMD_ENTER("ypm");

    if (ys->active_frame->buffer == get_or_make_buffer("ypm-menu")) {
        if (in_search) {
            if (event->key == ESC) {
                clear_and_leave_search();
                draw_list();
                event->cancel = 1;
            } else if (event->key == ENTER) {
                leave_search();
                event->cancel = 1;
            }
            return;
        }

        if (ys->active_frame->cursor_line < 19) { return; }

        switch (event->key) {
            case 'm':
                if (list_menu.base.is_up) { yed_gui_kill(&list_menu); }
                open_man_page("ypm");
                from_menu     = 1;
                event->cancel = 1;
                break;
            case 'f':
                if (list_menu.base.is_up) { yed_gui_kill(&list_menu); }
                start_search();
                event->cancel = 1;
                break;
            case ENTER:
                yed_delete_event_handler(h_mouse);
                if (!list_menu.base.is_up) {
                    yed_gui_kill(&list_menu);
                    yed_gui_init_list_menu(&list_menu, list_items);

                    if (ys->active_frame->cur_y + array_len(list_items) >= ys->active_frame->top + ys->active_frame->height) {
                        list_menu.base.top = ys->active_frame->cur_y - array_len(list_items) - 1;
                    } else {
                        list_menu.base.top = ys->active_frame->cur_y;
                    }
                    list_menu.base.left = ys->active_frame->cur_x;

                    yed_gui_draw(&list_menu);
                    yed_plugin_add_event_handler(SELF, h_mouse);
                    event->cancel = 1;
                }
                break;
        }
    } else if (ys->active_frame->buffer == get_or_make_buffer("ypm-output")
    ||         ys->active_frame->buffer == get_or_make_buffer("man-page")) {
        switch (event->key) {
            case ESC:
                if (from_menu) {
                    from_menu = 0;
                    YEXE("buffer", "*ypm-menu");
                    yed_set_cursor_within_frame(ys->active_frame, plug_row, 1);
                    event->cancel = 1;
                }
                break;
        }
    }

LOG_EXIT();
}

static void pre_mod_handler(yed_event *event) {
    if (mod_norecurse) { return; }
    if (event->buffer != get_or_make_buffer("ypm-menu")) { return; }
    if (!in_search) { return; }

    if (event->buff_mod_event == BUFF_MOD_ADD_LINE
    ||  event->buff_mod_event == BUFF_MOD_INSERT_LINE
    ||  event->buff_mod_event == BUFF_MOD_DELETE_LINE
    ||  event->buff_mod_event == BUFF_MOD_CLEAR_LINE
    ||  event->buff_mod_event == BUFF_MOD_SET_LINE
    ||  event->buff_mod_event == BUFF_MOD_CLEAR) {
        event->cancel = 1;
    }

    if (event->buff_mod_event == BUFF_MOD_INSERT_INTO_LINE
    ||  event->buff_mod_event == BUFF_MOD_APPEND_TO_LINE
    ||  event->buff_mod_event == BUFF_MOD_DELETE_FROM_LINE
    ||  event->buff_mod_event == BUFF_MOD_POP_FROM_LINE) {
        if (event->row != 15) {
            event->cancel = 1;
        }
    }
}

static void post_mod_handler(yed_event *event) {
    if (mod_norecurse) { return; }
    if (event->buffer != get_or_make_buffer("ypm-menu")) { return; }
/*     if (!in_search) { return; } */

    if (event->row == 15) {
        mod_norecurse = 1;
        draw_list();
        internal_mod_on(get_or_make_buffer("ypm-menu"));
        mod_norecurse = 0;
    }
}

static void _gui_key_handler(yed_event *event) {
    int ret = 0;
    ret = yed_gui_key_pressed(event, &list_menu);
    if (ret) {
        run();
    }

    if (!list_menu.base.is_up) {
        yed_delete_event_handler(h_mouse);
    }
}

static void _gui_mouse_handler(yed_event *event) {
    yed_gui_mouse_pressed(event, &list_menu);

    if (!list_menu.base.is_up) {
        yed_delete_event_handler(h_mouse);
    }
}

static void run() {
    yed_line *line;
    char     *s;
    int       i;
    char      plug_name[256];

    if (list_menu.base.is_up) { yed_gui_kill(&list_menu); }
    line = yed_buff_get_line(ys->active_frame->buffer, ys->active_frame->cursor_line);
    array_zero_term(line->chars);
    s = array_data(line->chars);
    for (i = 0; i < sizeof(plug_name) && s[i]; i += 1) {
        if (s[i] == ' ') { break; }
        plug_name[i] = s[i];
    }
    plug_name[i] = 0;

    switch (list_menu.selection) {
        case 0:
            from_menu = 1;
            plug_row = ys->active_frame->cursor_line;
            open_man_page(plug_name);
            break;
        case 1:
            from_menu = 1;
            plug_row = ys->active_frame->cursor_line;
            do_install(plug_name);
            break;
        case 2:
            from_menu = 1;
            plug_row = ys->active_frame->cursor_line;
            do_uninstall(plug_name);
            break;
        case 3:
            do_plugin_load(plug_name);
            break;
        case 4:
            do_plugin_unload(plug_name);
            break;
    }
}
