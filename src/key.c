void yed_init_keys(void) {
    ys->vkey_binding_map   = tree_make(int, yed_key_binding_ptr_t);
    ys->key_sequences      = array_make(yed_key_sequence);
    ys->released_virt_keys = array_make(int);

    yed_set_default_key_bindings();
}

int esc_timeout(int *input) {
    int  seq_key;
    char c;

    /* input[0] is ESC */

    if (read(0, &c, 1) == 0) {
        return 1;
    }
    input[1] = c;

    seq_key = yed_get_key_sequence(2, input);

    if (seq_key != KEY_NULL) {
        input[0] = seq_key;
        return 1;
    }

    if (read(0, &c, 1) == 0) {
        return 2;
    }
    input[2] = c;

    seq_key = yed_get_key_sequence(3, input);

    if (seq_key != KEY_NULL) {
        input[0] = seq_key;
        return 1;
    }

    return 3;
}

int esc_sequence(int *input) {
    char c;

    /* the input length is 3 */
    /* input[0] is ESC */

    if (input[1] == '[') { /* ESC [ sequences. */
        if (input[2] >= '0' && input[2] <= '9') {
            /* Extended escape, read additional byte. */
            if (read(0, &c, 1) == 0) {
                return 3;
            }

            if (c == '~') {
                switch (input[2]) {
                    case '1':    { input[0] = HOME_KEY;  break; }
                    case '3':    { input[0] = DEL_KEY;   break; }
                    case '4':    { input[0] = END_KEY;   break; }
                    case '5':    { input[0] = PAGE_UP;   break; }
                    case '6':    { input[0] = PAGE_DOWN; break; }
                }
                return 1;
            }
        } else {
            switch (input[2]) {
                case 'A':    { input[0] = ARROW_UP;    break; }
                case 'B':    { input[0] = ARROW_DOWN;  break; }
                case 'C':    { input[0] = ARROW_RIGHT; break; }
                case 'D':    { input[0] = ARROW_LEFT;  break; }
                case 'H':    { input[0] = HOME_KEY;    break; }
                case 'F':    { input[0] = END_KEY;     break; }
                case 'P':    { input[0] = DEL_KEY;     break; }
                case 'Z':    { input[0] = SHIFT_TAB;   break; }
            }
            return 1;
        }
    } else if (input[1] == 'O') { /* ESC O sequences. */
        switch (input[2]) {
            case 'H':    { input[0] = HOME_KEY; break; }
            case 'F':    { input[0] = END_KEY;  break; }
        }
        return 1;
    }

    return 3;
}

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

    if (ys->interactive_command) {
        input[len] = c;
        len        = 1;
    } else {
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

        seq_key = yed_get_key_sequence(len, input);

        if (seq_key != KEY_NULL) {
            input[0] = seq_key;
            return 1;
        }
    }

    return len;
}

int yed_read_keys(int *input) {
    int  nread;
    int  len;
    char c;

/*
 * BLOCKING(ish):
 **********************************************/
    while ((nread = read(0, &c, 1)) == 0);
/**********************************************/

/*
 * NON-BLOCKING:
 **********************************************/
/*     nread = read(0, &c, 1); */
/*     if (nread == 0)     { return 0; } */
/**********************************************/


    if (nread == -1)    { return 0; }

    if (c == ESC) {
        input[0] = c;

        len = esc_timeout(input);

        if (len == 3) {
            return esc_sequence(input);
        }

        return len;
    } else if (c >= 0) {
        return yed_read_key_sequences(c, input);
    }

    return 0;
}

void yed_take_key(int key) {
    yed_key_binding *binding;
    char             key_str_buff[32];
    char            *key_str;
    yed_event        event;

    event.kind = EVENT_KEY_PRESSED;
    event.key  = key;
    yed_trigger_event(&event);

    binding = yed_get_key_binding(key);

    sprintf(key_str_buff, "%d", key);
    key_str = key_str_buff;

    if (ys->interactive_command) {
        yed_execute_command(ys->interactive_command, 1, &key_str);
    } else if (binding) {
        yed_execute_command(binding->cmd, binding->n_args, binding->args);
    } else if (key < REAL_KEY_MAX
      &&      (key == ENTER || key == TAB || !iscntrl(key))) {
        yed_execute_command("insert", 1, &key_str);
    }

}

void yed_feed_keys(int n, int *keys) {
    int i;

    for (i = 0; i < n; i += 1) {
        yed_take_key(keys[i]);
    }
}

static yed_key_binding default_key_bindings[] = {
    { CTRL_F,      "command-prompt",    0, NULL },
    { ARROW_UP,    "cursor-up",         0, NULL },
    { ARROW_DOWN,  "cursor-down",       0, NULL },
    { ARROW_RIGHT, "cursor-right",      0, NULL },
    { ARROW_LEFT,  "cursor-left",       0, NULL },
    { HOME_KEY,    "cursor-line-begin", 0, NULL },
    { END_KEY,     "cursor-line-end",   0, NULL },
    { PAGE_UP,     "cursor-page-up",    0, NULL },
    { PAGE_DOWN,   "cursor-page-down",  0, NULL },
    { BACKSPACE,   "delete-back",       0, NULL },
    { DEL_KEY,     "delete-forward",    0, NULL },
    { CTRL_C,      "yank-selection",    0, NULL },
    { CTRL_F,      "command-prompt",    0, NULL },
    { CTRL_L,      "frame-next",        0, NULL },
    { CTRL_D,      "delete-line",       0, NULL },
    { CTRL_S,      "select",            0, NULL },
    { CTRL_V,      "paste-yank-buffer", 0, NULL },
    { CTRL_W,      "write-buffer",      0, NULL },
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
    yed_key_binding  *b;
    char            **args;
    int               i;

    if (binding.key == KEY_NULL)    { return; }

    yed_unbind_key(binding.key);

    binding.cmd  = strdup(binding.cmd);
    if (binding.n_args) {
        args         = binding.args;
        binding.args = malloc(sizeof(char*) * binding.n_args);
        for (i = 0; i < binding.n_args; i += 1) {
            binding.args[i] = strdup(args[i]);
        }
    }

    b  = malloc(sizeof(*b));
    *b = binding;

    if (binding.key < REAL_KEY_MAX) {
        ys->real_key_map[binding.key] = b;
    } else {
        tree_insert(ys->vkey_binding_map, binding.key, b);
    }
}

void yed_unbind_key(int key) {
    tree_it(int, yed_key_binding_ptr_t)  it;
    yed_key_binding                     *old_binding;
    int                                  i;

    if (key == KEY_NULL)    { return; }

    if (key < REAL_KEY_MAX) {
        old_binding = ys->real_key_map[key];
        if (old_binding) {
            free(old_binding->cmd);
            if (old_binding->n_args) {
                for (i = 0; i < old_binding->n_args; i += 1) {
                    free(old_binding->args[i]);
                }
                free(old_binding->args);
            }
            free(old_binding);
            ys->real_key_map[key] = NULL;
        }
    } else {
        it = tree_lookup(ys->vkey_binding_map, key);

        if (tree_it_good(it)) {
            old_binding = tree_it_val(it);
            tree_delete(ys->vkey_binding_map, tree_it_key(it));
            free(old_binding->cmd);
            if (old_binding->n_args) {
                for (i = 0; i < old_binding->n_args; i += 1) {
                    free(old_binding->args[i]);
                }
                free(old_binding->args);
            }
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

    it = tree_lookup(ys->vkey_binding_map, key);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

int yed_is_key(int key) {
    yed_key_sequence *seq;

    if (key < REAL_KEY_MAX) {
        return 1;
    }

    array_traverse(ys->key_sequences, seq) {
        if (seq->seq_key == key) {
            return 1;
        }
    }

    return 0;
}

int yed_acquire_virt_key(void) {
    int key;

    if (array_len(ys->released_virt_keys)) {
        key = *(int*)array_last(ys->released_virt_keys);
        array_pop(ys->released_virt_keys);
    } else {
        key                   = VIRT_KEY(ys->virt_key_counter);
        ys->virt_key_counter += 1;
    }

    return key;
}

void yed_release_virt_key(int key) {
    int *it;

    /*
     * Check if it's already been released.
     * Don't want duplicates.
     */
    array_traverse(ys->released_virt_keys, it) {
        if (*it == key)    { return; }
    }

    yed_unbind_key(key);

    array_push(ys->released_virt_keys, key);
}

int yed_add_key_sequence(int len, int *keys) {
    yed_key_sequence  seq;
    int               i;

    if (len < 2 || len > MAX_SEQ_LEN) {
        return KEY_NULL;
    }

    seq.len = len;

    for (i = 0; i < len; i += 1) {
        seq.keys[i] = keys[i];
    }

    seq.seq_key = yed_acquire_virt_key();

    array_push(ys->key_sequences, seq);

    return seq.seq_key;
}

int yed_get_key_sequence(int len, int *keys) {
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
    yed_release_virt_key(seq_key);

    return 0;
}

int yed_vadd_key_sequence(int len, ...) {
    va_list args;
    int     r;

    va_start(args, len);
    r = yed_vvadd_key_sequence(len, args);
    va_end(args);

    return r;
}

int yed_vget_key_sequence(int len, ...) {
    va_list args;
    int     r;

    va_start(args, len);
    r = yed_vvget_key_sequence(len, args);
    va_end(args);

    return r;
}

int yed_vvadd_key_sequence(int len, va_list args) {
    int i, keys[MAX_SEQ_LEN];

    for (i = 0; i < len; i += 1) {
        keys[i] = va_arg(args, int);
    }

    return yed_add_key_sequence(len, keys);
}

int yed_vvget_key_sequence(int len, va_list args) {
    int i, keys[MAX_SEQ_LEN];

    for (i = 0; i < len; i += 1) {
        keys[i] = va_arg(args, int);
    }

    return yed_get_key_sequence(len, keys);
}
