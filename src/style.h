#ifndef __STYLE_H__
#define __STYLE_H__


#define __STYLE_COMPONENTS     \
    __SCOMP(active)            \
    __SCOMP(inactive)          \
    __SCOMP(active_border)     \
    __SCOMP(inactive_border)   \
    __SCOMP(active_gutter)     \
    __SCOMP(inactive_gutter)   \
    __SCOMP(cursor_line)       \
    __SCOMP(selection)         \
    __SCOMP(search)            \
    __SCOMP(search_cursor)     \
    __SCOMP(attention)         \
    __SCOMP(associate)         \
    __SCOMP(status_line)       \
    __SCOMP(command_line)      \
    __SCOMP(popup)             \
    __SCOMP(popup_alt)         \
    __SCOMP(good)              \
    __SCOMP(bad)               \
    __SCOMP(code_comment)      \
    __SCOMP(code_keyword)      \
    __SCOMP(code_control_flow) \
    __SCOMP(code_typename)     \
    __SCOMP(code_preprocessor) \
    __SCOMP(code_fn_call)      \
    __SCOMP(code_number)       \
    __SCOMP(code_constant)     \
    __SCOMP(code_field)        \
    __SCOMP(code_variable)     \
    __SCOMP(code_string)       \
    __SCOMP(code_character)    \
    __SCOMP(code_escape)       \
    __SCOMP(white)             \
    __SCOMP(gray)              \
    __SCOMP(black)             \
    __SCOMP(red)               \
    __SCOMP(orange)            \
    __SCOMP(yellow)            \
    __SCOMP(lime)              \
    __SCOMP(green)             \
    __SCOMP(turquoise)         \
    __SCOMP(cyan)              \
    __SCOMP(blue)              \
    __SCOMP(purple)            \
    __SCOMP(magenta)           \
    __SCOMP(pink)              \


enum {
    NO_SCOMP,

    #define __SCOMP(comp) STYLE_##comp,
    __STYLE_COMPONENTS
    #undef __SCOMP

    N_SCOMPS
};

typedef struct yed_style_t {
    char      *_name;
    #define __SCOMP(comp) yed_attrs comp;
    __STYLE_COMPONENTS
    #undef __SCOMP
} yed_style;

void yed_init_styles(void);
void yed_set_style(const char *name, yed_style *style);
void yed_remove_style(const char *name);
yed_style * yed_get_style(const char *name);
int yed_activate_style(const char *name);
yed_style * yed_get_active_style(void);
yed_attrs yed_get_style_scomp(yed_style *style, int scomp);
yed_attrs yed_get_active_style_scomp(int scomp);
int yed_scomp_nr_by_name(const char *name);

#define __SCOMP(comp) yed_attrs yed_active_style_get_##comp(void);
__STYLE_COMPONENTS
#undef __SCOMP

void yed_set_default_styles(void);


#endif
