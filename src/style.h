#ifndef __STYLE_H__
#define __STYLE_H__

#include "internal.h"

typedef char *yed_style_name_t;

#define __STYLE_COMPONENTS     \
    __SCOMP(active)            \
    __SCOMP(inactive)          \
    __SCOMP(cursor_line)       \
    __SCOMP(selection)         \
    __SCOMP(search)            \
    __SCOMP(search_cursor)     \
    __SCOMP(code_comment)      \
    __SCOMP(code_keyword)      \
    __SCOMP(code_preprocessor) \
    __SCOMP(code_fn_call)      \
    __SCOMP(code_number)       \
    __SCOMP(code_constant)     \
    __SCOMP(code_string)       \
    __SCOMP(code_character)

typedef struct yed_style_t {
    char      *_name;
    #define __SCOMP(comp) yed_attrs comp;
    __STYLE_COMPONENTS
    #undef __SCOMP
} yed_style;

void yed_init_styles(void);
void yed_set_style(char *name, yed_style *style);
void yed_remove_style(char *name);
yed_style * yed_get_style(char *name);
int yed_activate_style(char *name);
yed_style * yed_get_active_style(void);

#define __SCOMP(comp) yed_attrs yed_active_style_get_##comp(void);
__STYLE_COMPONENTS
#undef __SCOMP

void yed_set_default_styles(void);


#endif
