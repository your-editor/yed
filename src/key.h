#ifndef __KEY_H__
#define __KEY_H__





enum KEY_ACTION {
    KEY_NULL  = 0,    /* NULL      */
    CTRL_A    = 1,    /* Ctrl-a    */
    CTRL_B    = 2,    /* Ctrl-b    */
    CTRL_C    = 3,    /* Ctrl-c    */
    CTRL_D    = 4,    /* Ctrl-d    */
    CTRL_E    = 5,    /* Ctrl-e    */
    CTRL_F    = 6,    /* Ctrl-f    */
    CTRL_G    = 7,    /* Ctrl-g    */
    CTRL_H    = 8,    /* Ctrl-h    */
    TAB       = 9,    /* Tab       */
    CTRL_J    = 10,   /* Ctrl-j    */
    CTRL_K    = 11,   /* Ctrl-k    */
    CTRL_L    = 12,   /* Ctrl-l    */
    ENTER     = 13,   /* Enter     */
    CTRL_N    = 14,   /* Ctrl-n    */
    CTRL_O    = 15,   /* Ctrl-o    */
    CTRL_P    = 16,   /* Ctrl-p    */
    CTRL_Q    = 17,   /* Ctrl-q    */
    CTRL_R    = 18,   /* Ctrl-r    */
    CTRL_S    = 19,   /* Ctrl-s    */
    CTRL_T    = 20,   /* Ctrl-t    */
    CTRL_U    = 21,   /* Ctrl-u    */
    CTRL_V    = 22,   /* Ctrl-v    */
    CTRL_W    = 23,   /* Ctrl-w    */
    CTRL_X    = 24,   /* Ctrl-x    */
    CTRL_Y    = 25,   /* Ctrl-y    */
    CTRL_Z    = 26,   /* Ctrl-z    */
    ESC       = 27,   /* Escape    */
    CTRL_FS   = 31,   /* Ctrl-/    */
    BACKSPACE = 127,  /* Backspace */

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
    SHIFT_TAB,

    REAL_KEY_MAX,

    VIRT_KEY_START,
};

#define CTRL_KEY(c) ((c) & 0x9F)

#define VIRT_KEY(x)      (VIRT_KEY_START + (x))

#define MAX_SEQ_LEN (8)

#define IS_ARROW(k)   ((k) >= ARROW_LEFT && (k) <= ARROW_DOWN)

void yed_init_keys(void);

int yed_read_keys(int *input);
void yed_take_key(int key);

void yed_feed_keys(int n, int *keys);

typedef struct yed_key_binding_t {
    int    key;
    char  *cmd;
    int    n_args;
    char **args;
} yed_key_binding;

void yed_set_default_key_binding(int key);
void yed_set_default_key_bindings(void);
void yed_bind_key(yed_key_binding binding);
void yed_unbind_key(int key);
yed_key_binding * yed_get_key_binding(int key);

typedef struct {
    int len;
    int keys[MAX_SEQ_LEN];
    int seq_key;
} yed_key_sequence;

int yed_is_key(int key);
int yed_acquire_virt_key(void);
void yed_release_virt_key(int key);

int yed_add_key_sequence(int len, int *keys);
int yed_get_key_sequence(int len, int *keys);
int yed_delete_key_sequence(int seq_key);
int yed_vadd_key_sequence(int len, ...);
int yed_vget_key_sequence(int len, ...);
int yed_vvadd_key_sequence(int len, va_list args);
int yed_vvget_key_sequence(int len, va_list args);

#endif
