#include "plugin.h"

#define YEXE(cmd_name, n, a) yed_execute_command((cmd_name), (n), (a))

/* COMMANDS */
void vimish_take_key(int n_args, char **args);
void vimish_nav(int key, char* key_str);
void vimish_insert(int key, char* key_str);
/* END COMMANDS */

#define MODE_NAV    (0x0)
#define MODE_INSERT (0x1)

static int mode;

void bind_keys(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {

    yed_plugin_set_command(self, "vimish-take-key", vimish_take_key);
    bind_keys(self);

    mode = MODE_NAV;

    return 0;
}

void bind_keys(yed_plugin *self) {
    int key;

    for (key = 1; key < KEY_MAX; key += 1) {
        if (key == CTRL_F)    { continue; }

        yed_plugin_bind_key(self, key, "vimish-take-key", 1);
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

void vimish_nav(int key, char *key_str) {
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

        case 'L':
        case 'w':
            YEXE("cursor-next-word",    0, NULL);
            break;

        case '0':
            YEXE("cursor-line-begin",    0, NULL);
            break;

        case '$':
            YEXE("cursor-line-end",    0, NULL);
            break;

        case 'g':
            YEXE("cursor-buffer-begin", 0, NULL);
            break;

        case 'G':
            YEXE("cursor-buffer-end",   0, NULL);
            break;

        case CTRL_P:
            fill_cmd_prompt("plugin-load");
            break;


        case 'a':
            YEXE("cursor-right",   0, NULL);
            goto enter_insert;
        case 'A':
            YEXE("cursor-line-end",   0, NULL);
        case 'i':
enter_insert:
            mode = MODE_INSERT;
            yed_append_text_to_cmd_buff("INSERT");
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
            mode = MODE_NAV;
            yed_append_text_to_cmd_buff("NAV");
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
