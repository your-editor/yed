#include "internal.h"

void yed_init_keys(void) {
    ys->key_seq_map       = tree_make(int, yed_key_binding_ptr_t);
    ys->key_sequences     = array_make(yed_key_sequence);
    ys->released_seq_keys = array_make(int);

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

static int _yed_get_key_sequence(int len, int *keys);

int yed_read_key_sequences(char first, int *input) {
    int  seq_key,
         len,
         i,
         found_a_partial_match,
         keep_reading;
    char c;
    yed_key_sequence *seq_it;

    c   = first;
    len = 0;

    do {
        /* We have consumed a keystroke. */
        input[len]    = c;
        len          += 1;

        keep_reading  = 0;

        /*
         * Should we consume another? See if there's
         * a key sequence defined that matches the keys
         * we have so far.
         */
        array_traverse(ys->key_sequences, seq_it) {
            /* Only if there's more in the sequence to read.. */
            if (seq_it->len > len) {
                found_a_partial_match = 1;
                for (i = 0; i < len; i += 1) {
                    if (seq_it->keys[i] != input[i]) {
                        found_a_partial_match = 0;
                        break;
                    }
                }

                if (found_a_partial_match) {
                    keep_reading = 1;
                    break;
                }
            }
        }
    } while (keep_reading && read(0, &c, 1) && len < MAX_SEQ_LEN);

    seq_key = _yed_get_key_sequence(len, input);

    if (seq_key != KEY_NULL) {
        input[0] = seq_key;
        return 1;
    }

    return len;
}

int yed_read_keys(int *input) {
    int  nread;
    char c,
         esc_seq[3];

    while ((nread = read(0, &c, 1)) == 0);
    if (nread == -1)    { return 0; }

    if (c == ESC) {
        /* If this is just an ESC, we'll timeout here. */
        if ((*input = esc_timout(esc_seq)))      { return 1; }
        if ((*input = esc_sequence(esc_seq)))    { return 1; }
    } else {
        return yed_read_key_sequences(c, input);
    }

    return 0;
}

void yed_take_key(int key) {
    yed_key_binding *binding;
    char             key_str_buff[32];
    char            *key_str;


    binding = yed_get_key_binding(key);

    sprintf(key_str_buff, "%d", key);
    key_str = key_str_buff;

    if (ys->interactive_command) {
        yed_execute_command(ys->interactive_command, 1, &key_str);
    } else if (binding) {
        yed_execute_command(binding->cmd, binding->takes_key_as_arg, &key_str);
    } else if (key < REAL_KEY_MAX
      &&      (key == ENTER || key == TAB || !iscntrl(key))) {
        yed_execute_command("insert", 1, &key_str);
    }

}

static yed_key_binding default_key_bindings[] = {
    { CTRL_F,      "command-prompt",    0 },
    { ARROW_UP,    "cursor-up",         0 },
    { ARROW_DOWN,  "cursor-down",       0 },
    { ARROW_RIGHT, "cursor-right",      0 },
    { ARROW_LEFT,  "cursor-left",       0 },
    { BACKSPACE,   "delete-back",       0 },
    { CTRL_C,      "yank-selection",    0 },
    { CTRL_F,      "command-prompt",    0 },
    { CTRL_L,      "frame-next",        0 },
    { CTRL_D,      "delete-line",       0 },
    { CTRL_S,      "select",            0 },
    { CTRL_V,      "paste-yank-buffer", 0 },
    { CTRL_W,      "write-buffer",      0 }
};

void yed_set_default_key_binding(int key) {
    int i;

    for (i = 0; i < sizeof(default_key_bindings) / sizeof(yed_key_binding); i += 1) {

        if (default_key_bindings[i].key == key) {
            yed_bind_key(default_key_bindings[i]);
            return;
        }
    }

    yed_unbind_key(key);
}

void yed_set_default_key_bindings(void) {
    int i;

    for (i = 0; i < sizeof(default_key_bindings) / sizeof(yed_key_binding); i += 1) {

        yed_bind_key(default_key_bindings[i]);
    }
}

void yed_bind_key(yed_key_binding binding) {
    yed_key_binding *b;

    if (binding.key == KEY_NULL)    { return; }

    yed_unbind_key(binding.key);

    binding.cmd = strdup(binding.cmd);

    b  = malloc(sizeof(*b));
    *b = binding;

    if (binding.key < REAL_KEY_MAX) {
        ys->real_key_map[binding.key] = b;
    } else {
        tree_insert(ys->key_seq_map, binding.key, b);
    }
}

void yed_unbind_key(int key) {
    tree_it(int, yed_key_binding_ptr_t)  it;
    yed_key_binding                     *old_binding;

    if (key == KEY_NULL)    { return; }

    if (key < REAL_KEY_MAX) {
        old_binding = ys->real_key_map[key];
        if (old_binding) {
            free(old_binding);
            ys->real_key_map[key] = NULL;
        }
    } else {
        it = tree_lookup(ys->key_seq_map, key);

        if (tree_it_good(it)) {
            old_binding = tree_it_val(it);
            tree_delete(ys->key_seq_map, tree_it_key(it));
            free(old_binding->cmd);
            free(old_binding);
        }
    }
}

yed_key_binding * yed_get_key_binding(int key) {
    tree_it(int, yed_key_binding_ptr_t) it;

    if (key == KEY_NULL)    { return NULL; }

    if (key < REAL_KEY_MAX) {
        return ys->real_key_map[key];
    }

    it = tree_lookup(ys->key_seq_map, key);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

int yed_vadd_key_sequence(int len, va_list args) {
    yed_key_sequence  seq;
    int               key,
                      i,
                     *rel_seq_key;

    if (len < 2 || len > MAX_SEQ_LEN) {
        return KEY_NULL;
    }

    seq.len = len;

    for (i = 0; i < len; i += 1) {
        key         = va_arg(args, int);
        seq.keys[i] = key;
    }

    if (array_len(ys->released_seq_keys)) {
        rel_seq_key = array_last(ys->released_seq_keys);
        seq.seq_key = *rel_seq_key;
        array_pop(ys->released_seq_keys);
    } else {
        seq.seq_key          = SEQ(ys->seq_key_counter);
        ys->seq_key_counter += 1;
    }

    array_push(ys->key_sequences, seq);

    return seq.seq_key;
}

int yed_add_key_sequence(int len, ...) {
    va_list args;
    int     r;

    va_start(args, len);
    r = yed_vadd_key_sequence(len, args);
    va_end(args);

    return r;
}

static int _yed_get_key_sequence(int len, int *keys) {
    yed_key_sequence *seq_it;
    int               i,
                      s,
                      found,
                      good;

    found = 0;
    s     = 0;
    array_traverse(ys->key_sequences, seq_it) {
        if (seq_it->len == len) {
            good = 1;

            for (i = 0; i < len; i += 1) {
                if (seq_it->keys[i] != keys[i]) {
                    good = 0;
                    break;
                }
            }

            if (good) {
                found = 1;
                break;
            }
        }
        s += 1;
    }

    if (!found)    { return KEY_NULL; }

    return seq_it->seq_key;
}

int yed_get_key_sequence(int len, ...) {
    va_list args;
    int     keys[MAX_SEQ_LEN],
            i;

    if (len < 2) {
        return KEY_NULL;
    }

    va_start(args, len);
    for (i = 0; i < len; i += 1) {
        keys[i] = va_arg(args, int);
    }
    va_end(args);

    return _yed_get_key_sequence(len, keys);
}

int yed_delete_key_sequence(int seq_key) {
    int               i,
                      found;
    yed_key_sequence *seq_it;

    i = found = 0;

    array_traverse(ys->key_sequences, seq_it) {
        if (seq_it->seq_key == seq_key) {
            found = 1;
            break;
        }
        i += 1;
    }

    if (!found)    { return 1; }

    array_delete(ys->key_sequences, i);
    array_push(ys->released_seq_keys, seq_key);

    return 0;
}
