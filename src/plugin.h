#ifndef __PLUGIN_H__
#define __PLUGIN_H__

/* This will make everything available to plugins if they include yed/plugin.h */
#include "internal.h"

#define YED_PLUG_SUCCESS     (0x0)
#define YED_PLUG_NOT_FOUND   (0x1)
#define YED_PLUG_NO_BOOT     (0x2)
#define YED_PLUG_BOOT_FAIL   (0x3)
#define YED_PLUG_DLOAD_FAIL  (0x4)
#define YED_PLUG_VER_MIS     (0x5)
#define YED_PLUG_LOAD_CANCEL (0x6)

typedef void *yed_plugin_handle_t;
typedef int (*yed_plugin_boot_t)(struct yed_plugin_t*);
typedef void (*yed_plugin_unload_fn_t)(struct yed_plugin_t*);

struct yed_style_t;

typedef struct yed_plugin_t {
    yed_plugin_handle_t    handle;
    char                  *path;
    yed_plugin_boot_t      boot;
    yed_plugin_unload_fn_t unload;
    array_t                added_cmds;
    array_t                added_maps;
    array_t                added_bindings;
    array_t                acquired_keys;
    array_t                added_key_sequences;
    array_t                added_event_handlers;
    array_t                added_styles;
    array_t                added_fts;
    array_t                added_compls;
    int                    requested_mouse_reporting;
} yed_plugin;

void yed_init_plugins(void);

int yed_load_plugin(char *plug_name);
int yed_unload_plugin(char *plug_name);

int yed_unload_plugin_libs(void);
void yed_plugin_uninstall_features(yed_plugin *plug);

int yed_unload_plugins(void);
int yed_reload_plugins(void);

void yed_plugin_set_command(yed_plugin *plug, const char *name, yed_command cmd);
int yed_plugin_acquire_virt_key(yed_plugin *plug);
void yed_plugin_add_key_map(yed_plugin *plug, const char *mapname);
void yed_plugin_bind_key(yed_plugin *plug, int key, const char *command_name, int n_args, char **args);
void yed_plugin_map_bind_key(yed_plugin *plug, const char *mapname, int key, const char *command_name, int n_args, char **args);
int yed_plugin_add_key_sequence(yed_plugin *plug, int len, int *keys);
int yed_plugin_vadd_key_sequence(yed_plugin *plug, int len, ...);
void yed_plugin_add_event_handler(yed_plugin *plug, yed_event_handler handler);
void yed_plugin_set_style(yed_plugin *plug, const char *name, struct yed_style_t *style);
int yed_plugin_make_ft(yed_plugin *plug, const char *ft_name);
void yed_plugin_set_completion(yed_plugin *plug, const char *name, yed_completion comp);
void yed_plugin_set_unload_fn(yed_plugin *plug, yed_plugin_unload_fn_t fn);
void yed_plugin_request_mouse_reporting(yed_plugin *plug);
void yed_plugin_request_no_mouse_reporting(yed_plugin *plug);

void yed_add_plugin_dir(const char *s);

#endif
