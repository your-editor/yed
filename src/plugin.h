#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "internal.h"

#define YED_PLUG_SUCCESS   (0x0)
#define YED_PLUG_NOT_FOUND (0x1)
#define YED_PLUG_NO_BOOT   (0x2)
#define YED_PLUG_BOOT_FAIL (0x3)

typedef char *yed_plugin_name_t;
typedef void *yed_plugin_handle_t;
struct yed_plugin_t;
typedef int (*yed_plugin_boot_t)(struct yed_plugin_t*);

typedef struct yed_plugin_t {
    yed_plugin_handle_t handle;
    yed_plugin_boot_t   boot;
    array_t             added_cmds;
    array_t             added_bindings;
} yed_plugin;

void yed_init_plugins(void);

int yed_load_plugin(char *plug_name);
int yed_unload_plugin(char *plug_name);

int yed_unload_plugin_libs(void);

int yed_unload_plugins(void);
int yed_reload_plugins(void);

void yed_plugin_add_command(yed_plugin *plug, char *name, yed_command cmd);
void yed_plugin_bind_key(yed_plugin *plug, int key, char *command_name, int takes_key_as_arg);

#endif
