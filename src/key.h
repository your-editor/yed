#ifndef __KEY_H__
#define __KEY_H__

#include "internal.h"




enum KEY_ACTION {
    KEY_NULL  = 0,    /* NULL      */
    CTRL_C    = 3,    /* Ctrl-c    */
    CTRL_D    = 4,    /* Ctrl-d    */
    CTRL_F    = 6,    /* Ctrl-f    */
    CTRL_H    = 8,    /* Ctrl-h    */
    TAB       = 9,    /* Tab       */
    CTRL_L    = 12,   /* Ctrl+l    */
    ENTER     = 13,   /* Enter     */
    CTRL_O    = 15,   /* Ctrl+o    */
    CTRL_P    = 16,   /* Ctrl+p    */
    CTRL_Q    = 17,   /* Ctrl-q    */
    CTRL_S    = 19,   /* Ctrl-s    */
    CTRL_U    = 21,   /* Ctrl-u    */
    CTRL_W    = 23,   /* Ctrl-w    */
    ESC       = 27,   /* Escape    */
    BACKSPACE =  127, /* Backspace */

    /* The following are just soft codes, not really reported by the
     * terminal directly. */
    ARROW_LEFT = 300,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    FOOZLE,
    KEY_MAX
};

#define IS_ARROW(k)   ((k) >= ARROW_LEFT && (k) <= ARROW_DOWN)

void yed_init_keys(void);

int yed_read_keys(int *input);
void yed_take_key(int key);

typedef struct {
    int   is_bound;
    int   key;
    char *cmd;
    int   takes_key_as_arg;
} yed_key_binding;

void yed_set_default_key_binding(int key);
void yed_set_default_key_bindings(void);
void yed_bind_key(yed_key_binding binding);

#endif
