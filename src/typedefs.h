#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

struct yed_key_binding_t;
struct yed_buffer_t;
struct yed_plugin_t;
struct yed_style_t;
struct yed_cmd_line_readline_t;
struct yed_completion_results_t;

typedef char *str_t;
typedef struct yed_key_binding_t *yed_key_binding_ptr_t;
typedef char *yed_buffer_name_t;
typedef struct yed_buffer_t *yed_buffer_ptr_t;
typedef char *yed_command_name_t;
typedef void (*yed_command)(int, char**);
typedef char *yed_plugin_name_t;
typedef struct yed_plugin_t *yed_plugin_ptr_t;
typedef char *yed_var_name_t;
typedef char *yed_var_val_t;
typedef char *yed_style_name_t;
typedef struct yed_style_t *yed_style_ptr_t;
typedef struct yed_cmd_line_readline_t *yed_cmd_line_readline_ptr_t;
typedef char *yed_completion_name_t;
typedef int (*yed_completion)(char*, struct yed_completion_results_t*);
typedef char *yed_ft_name_t;

#endif
