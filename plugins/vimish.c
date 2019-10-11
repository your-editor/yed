#include "plugin.h"

#define YEXE(cmd_name, n, a) yed_execute_command((cmd_name), (n), (a))

/* COMMANDS */
void vimish_take_key(int n_args, char **args);
void vimish_write(int n_args, char **args);
void vimish_quit(int n_args, char **args);
void vimish_write_quit(int n_args, char **args);
/* END COMMANDS */

#define MODE_NAV    (0x0)
#define MODE_INSERT (0x1)
#define MODE_DELETE (0x2)

static yed_plugin *Self;
static int mode;
static int exit_insert_key;

void vimish_nav(int key, char* key_str);
void vimish_insert(int key, char* key_str);
void vimish_delete(int key, char* key_str);
void bind_keys(void);
void enter_insert(void);
void exit_insert(void);
void enter_delete(int by_line);
void exit_delete(int cancel);

int yed_plugin_boot(yed_plugin *self) {
    Self = self;

    yed_plugin_set_command(Self, "vimish-take-key", vimish_take_key);
    yed_plugin_set_command(Self, "w",               vimish_write);
    yed_plugin_set_command(Self, "W",               vimish_write);
    yed_plugin_set_command(Self, "q",               vimish_quit);
    yed_plugin_set_command(Self, "Q",               vimish_quit);
    yed_plugin_set_command(Self, "wq",              vimish_write_quit);
    yed_plugin_set_command(Self, "Wq",              vimish_write_quit);

    bind_keys();

    mode = MODE_NAV;

    return 0;
}

void bind_keys(void) {
    int key;

    for (key = 1; key < REAL_KEY_MAX; key += 1) {
        if (key == CTRL_S) {
            yed_plugin_bind_key(Self, key, "select", 0);
            continue;
        }
        yed_plugin_bind_key(Self, key, "vimish-take-key", 1);
    }
}

void vimish_take_key(int n_args, char **args) {
    int key;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    sscanf(args[0], "%d", &key);

    switch (mode) {
        case MODE_NAV:    vimish_nav(key, args[0]);    break;
        case MODE_INSERT: vimish_insert(key, args[0]); break;
        case MODE_DELETE: vimish_delete(key, args[0]); break;
        default:
            yed_append_text_to_cmd_buff("[!] invalid mode (?)");
    }
}

static void fill_cmd_prompt(const char *cmd) {
    int   len, i, key;
    char  key_str_buff[32];
    char *key_str;

    len = strlen(cmd);

    YEXE("command-prompt", 0, NULL);

    for (i = 0; i < len; i += 1) {
        key = cmd[i];
        sprintf(key_str_buff, "%d", key);
        key_str = key_str_buff;
        YEXE("command-prompt", 1, &key_str);
    }

    key_str = "32"; /* space */
    YEXE("command-prompt", 1, &key_str);
}

int vimish_nav_common(int key, char *key_str) {
    switch (key) {
        case 'h':
        case ARROW_LEFT:
            YEXE("cursor-left", 0, NULL);
            break;

        case 'j':
        case ARROW_DOWN:
            YEXE("cursor-down", 0, NULL);
            break;

        case 'k':
        case ARROW_UP:
            YEXE("cursor-up", 0, NULL);
            break;

        case 'l':
        case ARROW_RIGHT:
            YEXE("cursor-right", 0, NULL);
            break;

        case 'w':
            YEXE("cursor-next-word",    0, NULL);
            break;

        case 'b':
            YEXE("cursor-prev-word",    0, NULL);
            break;

        case '0':
            YEXE("cursor-line-begin",    0, NULL);
            break;

        case '$':
            YEXE("cursor-line-end",    0, NULL);
            break;

        case '{':
            YEXE("cursor-prev-paragraph", 0, NULL);
            break;

        case '}':
            YEXE("cursor-next-paragraph", 0, NULL);
            break;

        case 'g':
            YEXE("cursor-buffer-begin", 0, NULL);
            break;

        case 'G':
            YEXE("cursor-buffer-end",   0, NULL);
            break;

        default:
            return 0;
    }
    return 1;
}

void vimish_nav(int key, char *key_str) {
    if (vimish_nav_common(key, key_str)) {
        return;
    }

    YEXE("select-off", 0, NULL);

    switch (key) {
        case CTRL_P:
            fill_cmd_prompt("plugin-load");
            break;

        case 'd':
            enter_delete(0);
            break;

        case 'D':
            enter_delete(1);
            break;

        case 'a':
            YEXE("cursor-right",   0, NULL);
            goto enter_insert;
        case 'A':
            YEXE("cursor-line-end",   0, NULL);
        case 'i':
enter_insert:
            enter_insert();
            break;

        case ':':
            YEXE("command-prompt", 0, NULL);
            break;

        default:
            yed_append_text_to_cmd_buff("[!] [NAV] unhandled key ");
            yed_append_int_to_cmd_buff(key);
    }
}

void vimish_insert(int key, char *key_str) {
    if (key == exit_insert_key)    { goto exit_insert; }

    switch (key) {
        case ARROW_LEFT:
            YEXE("cursor-left",  0, NULL);
            break;

        case ARROW_DOWN:
            YEXE("cursor-down",  0, NULL);
            break;

        case ARROW_UP:
            YEXE("cursor-up",    0, NULL);
            break;

        case ARROW_RIGHT:
            YEXE("cursor-right", 0, NULL);
            break;

        case BACKSPACE:
            YEXE("delete-back", 0, NULL);
            break;

        case CTRL_C:
exit_insert:
            exit_insert();
            break;

        default:
            if (key == ENTER || key == TAB || !iscntrl(key)) {
                yed_execute_command("insert", 1, &key_str);
            } else {
                yed_append_text_to_cmd_buff("[!] [INSERT] unhandled key ");
                yed_append_int_to_cmd_buff(key);
            }
    }

}

void vimish_delete(int key, char *key_str) {
    if (vimish_nav_common(key, key_str)) {
        return;
    }

    switch (key) {
        case 'd':
            exit_delete(0);
            break;

        case CTRL_C:
            exit_delete(1);
            break;

        default:
            yed_append_text_to_cmd_buff("[!] [DELETE] unhandled key ");
            yed_append_int_to_cmd_buff(key);
    }

}

void vimish_write(int n_args, char **args) {
    YEXE("write-buffer", 0, NULL);
}

void vimish_quit(int n_args, char **args) {
    YEXE("quit", 0, NULL);
}

void vimish_write_quit(int n_args, char **args) {
    YEXE("write-buffer", 0, NULL);
    YEXE("quit",         0, NULL);
}

void enter_insert(void) {
    mode = MODE_INSERT;

    exit_insert_key = yed_plugin_add_key_sequence(Self, 2, 'j', 'j');
    yed_plugin_bind_key(Self, exit_insert_key, "vimish-take-key", 1);
    yed_append_text_to_cmd_buff("INSERT");
}

void exit_insert(void) {
    mode = MODE_NAV;

    yed_unbind_key(exit_insert_key);
    yed_delete_key_sequence(exit_insert_key);
    yed_append_text_to_cmd_buff("NAV");
}

void enter_delete(int by_line) {
    mode = MODE_DELETE;

    if (by_line) {
        YEXE("select-lines", 0, NULL);
        yed_append_text_to_cmd_buff("DELETE LINES");
    } else {
        YEXE("select", 0, NULL);
        yed_append_text_to_cmd_buff("DELETE");
    }

}

void exit_delete(int cancel) {
    yed_range *sel;

    mode = MODE_NAV;

    if (!cancel) {
        if (ys->active_frame
        &&  ys->active_frame->buffer
        &&  ys->active_frame->buffer->has_selection) {

            sel = &ys->active_frame->buffer->selection;
            if (sel->anchor_row == sel->cursor_row
            &&  sel->anchor_col == sel->cursor_col) {

                YEXE("select-lines", 0, NULL);
            }

            YEXE("delete-back", 0, NULL);
        }
    }

    YEXE("select-off", 0, NULL);

    yed_append_text_to_cmd_buff("NAV");
}
