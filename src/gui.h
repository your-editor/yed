#ifndef __YED_GUI_H__
#define __YED_GUI_H__

#define LIST_MENU (0)

/* Base GUI Struct */
typedef struct {
    int           kind;          //kind of GUI element
    int           top;           //top row of the menu
    int           left;          //left most column of the menu
    int           is_up;         //if gui is shown
    array_t       dds;           //array of direct draws
    int           mouse_pressed; //we have seen a mouse press
} _yed_gui_base;

/* Specialized GUI Structs */
typedef struct {
    _yed_gui_base     base;      //GUI Base Pointer
    array_t           strings;   //array of strings
    int               selection; //the current selection
    int               max_size;  //max number of strings shown
    int               max_width; //the number of columns of the menu
} yed_gui_list_menu;

/* Global Helper Functions */
static inline yed_frame *yed_gui_find_frame(yed_event* event);
static inline int        yed_gui_find_width(array_t strings);

/* Specialized List Menu Functions */
static inline void       yed_gui_init_list_menu(yed_gui_list_menu *menu, array_t strings);
static inline void      _yed_gui_draw_list_menu(yed_gui_list_menu *menu);
static inline int       _yed_gui_key_pressed_list_menu(yed_event *event, yed_gui_list_menu *menu);
static inline void      _yed_gui_mouse_pressed_list_menu(yed_event *event, yed_gui_list_menu *menu);
static inline void      _yed_gui_kill_list_menu(yed_gui_list_menu *menu);

/* Base Functions */
static inline void       yed_gui_draw(void *base);
static inline int        yed_gui_key_pressed(yed_event *event, void *base);
static inline void       yed_gui_mouse_pressed(yed_event *event, void *base);
static inline void       yed_gui_kill(void *base);


/* Initilization Functions */
static inline void yed_gui_init_list_menu(yed_gui_list_menu *menu, array_t strings) {
    memset(menu, 0, sizeof(*menu));

    yed_gui_kill(menu);

    menu->base.kind          =  LIST_MENU;
    menu->base.is_up         =  1;
    menu->base.dds           =  array_make(yed_direct_draw_t*);
    menu->base.mouse_pressed =  0;
    menu->strings            =  copy_string_array(strings);
    menu->selection          = -1;
    menu->max_size           =  array_len(menu->strings);
    menu->max_width          =  yed_gui_find_width(menu->strings);

    yed_gui_draw(menu);
}

/* Specialiezed Draw Functions */
static inline void _yed_gui_draw_list_menu(yed_gui_list_menu *menu) {
    yed_direct_draw_t **dd_it;
    yed_attrs           popup;
    yed_attrs           popup_alt;
    yed_attrs           active;
    yed_attrs           assoc;
    yed_attrs           normal;
    yed_attrs           selected;
    int                 width;

    char              **it;
    int                 i;
    char                buff[512];
    yed_direct_draw_t  *dd;

    array_traverse(menu->base.dds, dd_it) {
        yed_kill_direct_draw(*dd_it);
    }
    array_free(menu->base.dds);

    menu->base.dds = array_make(yed_direct_draw_t*);

    popup     = yed_active_style_get_popup();
    popup_alt = yed_active_style_get_popup_alt();
    active    = yed_active_style_get_active();
    assoc     = yed_active_style_get_associate();

    if (popup.flags) {
        normal = popup;
    }else {
        normal = active;
        yed_combine_attrs(&normal, &assoc);
    }

    if (popup_alt.flags) {
        selected = popup_alt;
    }else {
        selected = normal;
        selected.flags ^= ATTR_INVERSE;
    }

    width = 0;
    array_traverse(menu->strings, it) {
        width = MAX(width, strlen(*it));
    }
    menu->max_width = width;

/*     fix_bounds */
    if ((menu->base.top + array_len(menu->strings)) > (ys->term_rows - 2)) {
        menu->base.top = ys->term_rows - array_len(menu->strings) - 2;
    }

    if ((menu->base.left + yed_gui_find_width(menu->strings)) > ys->term_cols) {
        menu->base.left = ys->term_cols - yed_gui_find_width(menu->strings);
    }

    i = 1;
    array_traverse(menu->strings, it) {
        snprintf(buff, sizeof(buff), " %*s ", -menu->max_width, *it);
        dd = yed_direct_draw(menu->base.top + i,
                             menu->base.left,
                             i == menu->selection + 1 ? selected : normal,
                             buff);
        array_push(menu->base.dds, dd);
        i += 1;
    }
}

/* Specialized Key Pressed Functions */
static inline int _yed_gui_key_pressed_list_menu(yed_event *event, yed_gui_list_menu *menu) {
    if (ys->interactive_command != NULL) { return 0; }

    if (!menu->base.is_up) { return 0; }

    if (event->key == ESC) {
        event->cancel = 1;
        yed_gui_kill(menu);

    }else if (event->key == ENTER) {
        event->cancel = 1;
        yed_gui_kill(menu);
        if (menu->selection >= 0) { return 1; }

    }else if (event->key == ARROW_UP || event->key == SHIFT_TAB) {
        event->cancel = 1;
        if (menu->selection > 0) {
            menu->selection -= 1;
        }else {
            menu->selection = menu->max_size - 1;
        }
        yed_gui_draw(menu);
    }else if (event->key == ARROW_DOWN || event->key == TAB) {
        event->cancel = 1;
        if (menu->selection < menu->max_size - 1) {
            menu->selection += 1;
        }else {
            menu->selection = 0;
        }
        yed_gui_draw(menu);
    }
    return 0;
}

static inline void _yed_gui_mouse_pressed_list_menu(yed_event *event, yed_gui_list_menu *menu) {
    int key;

    if (IS_MOUSE(event->key)) {
        if (MOUSE_KIND(event->key) == MOUSE_RELEASE) {
            if (MOUSE_BUTTON(event->key) == MOUSE_BUTTON_LEFT && menu->base.mouse_pressed) {
                menu->base.mouse_pressed = 0;
                if ((MOUSE_ROW(event->key) <= (menu->base.top + menu->max_size)) &&
                    (MOUSE_ROW(event->key) >=  menu->base.top + 1)               &&
                    (MOUSE_COL(event->key) >=  menu->base.left)                  &&
                    (MOUSE_COL(event->key) <= (menu->base.left + menu->max_width))) {
                        menu->selection = (MOUSE_ROW(event->key) - menu->base.top - 1);
                        yed_gui_draw(menu);
                        key = ENTER;
                        yed_feed_keys(1, &key);
                    } else {
                        yed_gui_kill(menu);
                    }
                event->cancel = 1;
            }
        }else if (MOUSE_KIND(event->key) == MOUSE_PRESS) {
            if (MOUSE_BUTTON(event->key) == MOUSE_WHEEL_UP) {
                if (menu->selection > 0) {
                    menu->selection -= 1;
                }else {
                    menu->selection = menu->max_size - 1;
                }
                yed_gui_draw(menu);
                event->cancel = 1;
            }else if (MOUSE_BUTTON(event->key) == MOUSE_WHEEL_DOWN) {
                if (menu->selection < menu->max_size - 1) {
                    menu->selection += 1;
                }else {
                    menu->selection = 0;
                }
                yed_gui_draw(menu);
                event->cancel = 1;
            }else if (MOUSE_KIND(event->key) == MOUSE_BUTTON_LEFT) {
                menu->base.mouse_pressed = 1;
                event->cancel = 1;
            }
        }
    }
}

/* Specialized Kill Functions */
static inline void _yed_gui_kill_list_menu(yed_gui_list_menu *menu) {
    yed_direct_draw_t **dd;

    if (!menu->base.is_up) { return; }


    while(array_len(menu->strings) > 0) {
        free(*(char **)array_last(menu->strings));
        array_pop(menu->strings);
    }

    array_traverse(menu->base.dds, dd) {
        yed_kill_direct_draw(*dd);
    }

    array_free(menu->base.dds);

    menu->base.is_up = 0;
}

/* Abstracted Base Functions */
static inline int yed_gui_key_pressed(yed_event *event, void* base) {
    switch(((_yed_gui_base*)base)->kind) {
        case LIST_MENU:;
            return _yed_gui_key_pressed_list_menu(event, base);
            break;
        default:;
    }
    return 0;
}

static inline void yed_gui_mouse_pressed(yed_event *event, void* base) {
    switch(((_yed_gui_base*)base)->kind) {
        case LIST_MENU:;
            _yed_gui_mouse_pressed_list_menu(event, base);
            break;
        default:;
    }
}

static inline void yed_gui_draw(void *base) {
    switch(((_yed_gui_base*)base)->kind) {
        case LIST_MENU:
            return _yed_gui_draw_list_menu(base);
            break;
        default:;
    }
}

static inline void yed_gui_kill(void *base) {
    switch(((_yed_gui_base*)base)->kind) {
        case LIST_MENU:
            _yed_gui_kill_list_menu(base);
            break;
        default:;
    }
}

/* Global Helper Functions */
static inline yed_frame *yed_gui_find_frame(yed_event* event) {
    yed_frame **frame_it;

    if(ys->active_frame == NULL) {return NULL;}

    if (yed_cell_is_in_frame(MOUSE_ROW(event->key), MOUSE_COL(event->key), ys->active_frame)) {
        return ys->active_frame;
    }

    array_rtraverse(ys->frames, frame_it) {
        if (yed_cell_is_in_frame(MOUSE_ROW(event->key), MOUSE_COL(event->key), (*frame_it))) {
            return (*frame_it);
        }
    }

    return NULL;
}

static inline int yed_gui_find_width(array_t strings) {
    char **it;
    int width = 0;

    array_traverse(strings, it) {
        width = MAX(width, strlen(*it));
    }

    return width;
}

#endif
