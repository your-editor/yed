#include "internal.h"

void yed_init_keys(void) {
    yed_set_default_key_bindings();
}

int esc_timout(char *seq) {
    if (read(0, seq, 1)     == 0)    { return ESC; }
    if (read(0, seq + 1, 1) == 0)    { return ESC; }

    return 0;
}

int esc_sequence(char *seq) {
    if (seq[0] == '[') { /* ESC [ sequences. */
        if (seq[1] >= '0' && seq[1] <= '9') {
            /* Extended escape, read additional byte. */
            if (read(0, seq + 2, 1) == 0)    { return ESC; }
            if (seq[2] == '~') {
                switch (seq[1]) {
                    case '3':    { return DEL_KEY;   }
                    case '5':    { return PAGE_UP;   }
                    case '6':    { return PAGE_DOWN; }
                }
            }
        } else {
            switch (seq[1]) {
                case 'A':    { return ARROW_UP;    }
                case 'B':    { return ARROW_DOWN;  }
                case 'C':    { return ARROW_RIGHT; }
                case 'D':    { return ARROW_LEFT;  }
                case 'H':    { return HOME_KEY;    }
                case 'F':    { return END_KEY;     }
            }
        }
    } else if (seq[0] == 'O') { /* ESC O sequences. */
        switch (seq[1]) {
            case 'H':    { return HOME_KEY; }
            case 'F':    { return END_KEY;  }
        }
    }

    return 0;
}

int yed_read_keys(int *input) {
    int  nread;
    char c, seq[3];


    while ((nread = read(0, &c, 1)) == 0);
/*     if (nread == -1)    { ERR("could not read key"); } */
    if (nread == -1)    { return 0; }

    switch (c) {
        case ESC:    /* escape sequence */
            /* If this is just an ESC, we'll timeout here. */
            if ((*input = esc_timout(seq)))      { return 1; }
            if ((*input = esc_sequence(seq)))    { return 1; }
            break;

        /* Here's an example of how we can do some multi-key bindings */
/*         case 'd': */
/*             if (read(0, seq, 1)) { */
/*                 if (seq[0] == 'd') { */
/*                     *input = FOOZLE; */
/*                     return 1; */
/*                 } else { */
/*                     *(input++) = 'd'; */
/*                     *input     = seq[0]; */
/*                     return 2; */
/*                 } */
/*             } */

        default:
            *input = c;
            return 1;
    }

    return 0;
}

void yed_take_key(int key) {
    yed_key_binding *binding;
    char             key_str_buff[32];
    char            *key_str;


    binding = ys->key_map + key;

    sprintf(key_str_buff, "%d", key);
    key_str = key_str_buff;

    if (ys->accepting_command) {
        yed_execute_command("command-prompt", 1, &key_str);
    } else if (binding->is_bound) {
        yed_execute_command(binding->cmd, binding->takes_key_as_arg, &key_str);
    } else if (key == ENTER || key == TAB || !iscntrl(key)) {
        yed_execute_command("insert", 1, &key_str);
    }

}

static yed_key_binding default_key_bindings[] = {
    { 1, CTRL_F,      "command-prompt",   0 },
    { 1, ARROW_UP,    "cursor-up",        0 },
    { 1, ARROW_DOWN,  "cursor-down",      0 },
    { 1, ARROW_RIGHT, "cursor-right",     0 },
    { 1, ARROW_LEFT,  "cursor-left",      0 },
    { 1, BACKSPACE,   "delete-back",      0 },
    { 1, CTRL_F,      "command-prompt",   0 },
    { 1, CTRL_L,      "frame-next",       0 },
    { 1, CTRL_D,      "delete-line",      0 },
    { 1, CTRL_W,      "cursor-next-word", 0 }
};

void yed_set_default_key_binding(int key) {
    int             i;
    yed_key_binding no_binding;

    for (i = 0; i < sizeof(default_key_bindings) / sizeof(yed_key_binding); i += 1) {

        if (default_key_bindings[i].key == key) {
            yed_bind_key(default_key_bindings[i]);
            return;
        }
    }

    memset(&no_binding, 0, sizeof(no_binding));
    no_binding.key = key;
    yed_bind_key(no_binding);
}

void yed_set_default_key_bindings(void) {
    int i;

    for (i = 0; i < sizeof(default_key_bindings) / sizeof(yed_key_binding); i += 1) {

        yed_bind_key(default_key_bindings[i]);
    }
}

void yed_bind_key(yed_key_binding binding) {
    yed_key_binding old_binding;

    if (binding.key >= KEY_MAX) {
        return;
    }

    old_binding = ys->key_map[binding.key];
    if (old_binding.is_bound) {
        free(old_binding.cmd);
    }

    if (binding.is_bound) {
        binding.cmd = strdup(binding.cmd);
    }

    ys->key_map[binding.key] = binding;
}
