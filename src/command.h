#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "internal.h"

#define YED_CMD_PROMPT ": "

typedef const char *yed_command_name_t;
typedef void (*yed_command_t)(int, char**);

static void yed_init_commands(void);

static void yed_add_command(const char *name, yed_command_t command);
static yed_command_t yed_get_command(const char *name);

static void yed_add_default_commands(void);
static void yed_reload_default_commands(void);

static void yed_draw_command_line(void);
static void yed_do_command(void);
static void yed_command_take_key(int key);

#define DEF_DEFAULT_COMMAND(name) \
    static void yed_default_command_##name(int n_args, char **args)

DEF_DEFAULT_COMMAND(command_prompt);
DEF_DEFAULT_COMMAND(quit);
DEF_DEFAULT_COMMAND(reload);
DEF_DEFAULT_COMMAND(echo);
DEF_DEFAULT_COMMAND(cursor_down);
DEF_DEFAULT_COMMAND(cursor_up);
DEF_DEFAULT_COMMAND(cursor_left);
DEF_DEFAULT_COMMAND(cursor_right);
DEF_DEFAULT_COMMAND(buffer_new);
DEF_DEFAULT_COMMAND(frame);
DEF_DEFAULT_COMMAND(frames_list);
DEF_DEFAULT_COMMAND(frame_set_buffer);
DEF_DEFAULT_COMMAND(frame_new_file);
DEF_DEFAULT_COMMAND(frame_split_new_file);
DEF_DEFAULT_COMMAND(insert);
DEF_DEFAULT_COMMAND(delete_back);
DEF_DEFAULT_COMMAND(write_buffer);



static int yed_execute_command(const char *name, int n_args, char **args);

#endif
