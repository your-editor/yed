static int ctrl_h_is_bs;

void yed_init_keys(void) {
    ys->vkey_binding_map     = tree_make(int, yed_key_binding_ptr_t);
    ys->key_sequences        = array_make(yed_key_sequence);
    ys->released_virt_keys   = array_make(int);
    ys->bracketed_paste_buff = array_make(char);

    yed_set_default_key_bindings();
}

static int esc_timeout(int *input) {
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

static int esc_sequence(int *input) {
    char c;
    char buff[64];
    int  i;
    int  k;
    int  b;
    int  x;
    int  y;

    /* the input length is 3 */
    /* input[0] is ESC */

    if (input[1] == '[') { /* ESC [ sequences. */
        if (input[2] >= '0' && input[2] <= '9') {
            /* Extended escape, read additional byte. */
            if (read(0, &c, 1) == 0) {
                return 3;
            } else if (input[2] == '1') {
                input[3] = c;
                if (c == '~') {
                    input[0] = HOME_KEY;
                    return 1;
                } else if (c == ';') {
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '3') {
                        if (read(0, &c, 1) == 0) { return 5; }
                        input[5] = c;
                        switch (c) {
                            case 'A':
                                input[1] = ARROW_UP;
                                return 2;
                            case 'B':
                                input[1] = ARROW_DOWN;
                                return 2;
                            case 'C':
                                input[1] = ARROW_RIGHT;
                                return 2;
                            case 'D':
                                input[1] = ARROW_LEFT;
                                return 2;
                        }
                        return 6;
                    }
                    return 5;
                } else if (c == '5') {
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN5;
                        return 1;
                    }
                    return 5;
                } else if (c == '7') {
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN6;
                        return 1;
                    }
                    return 5;
                } else if (c == '8') {
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN7;
                        return 1;
                    }
                    return 5;
                } else if (c == '9') {
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN8;
                        return 1;
                    }
                    return 5;
                }
                return 4;
            } else if (input[2] == '2') {
                if (c == '0') {
                    input[3] = c;

                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;

                    if (c == '~') {
                        input[0] = FN9;
                        return 1;
                    } else if (c == '0') {
                        if (read(0, &c, 1) == 0) { return 5; }
                        input[5] = c;
                        if (c == '~') { input[0] = _BRACKETED_PASTE_BEGIN; return 1; }
                        return 6;
                    } else if (c == '1') {
                        if (read(0, &c, 1) == 0) { return 5; }
                        input[5] = c;
                        if (c == '~') { input[0] = _BRACKETED_PASTE_END; return 1; }
                        return 6;
                    }
                    return 5;
                } else if (c == '1') {
                    input[3] = c;
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN10;
                        return 1;
                    }
                    return 5;
                } else if (c == '3') {
                    input[3] = c;
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN11;
                        return 1;
                    }
                    return 5;
                } else if (c == '4') {
                    input[3] = c;
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == '~') {
                        input[0] = FN12;
                        return 1;
                    }
                    return 5;
                } else if (c == '7') {
                    input[3] = c;
                    if (read(0, &c, 1) == 0) { return 4; }
                    input[4] = c;
                    if (c == ';') {
                        if (read(0, &c, 1) == 0) { return 5; }
                        input[5] = c;
                        if (c == '5') {
                            if (read(0, &c, 1) == 0) { return 6; }
                            input[6] = c;
                            if (c == ';') {
                                if (read(0, &c, 1) == 0) { return 7; }
                                input[7] = c;
                                if (c == '9') {
                                    if (read(0, &c, 1) == 0) { return 8; }
                                    input[8] = c;
                                    if (c == '~') {
                                        input[0] = CTRL_TAB;
                                        return 1;
                                    }
                                    return 9;
                                } else if (c == '1') {
                                    if (read(0, &c, 1) == 0) { return 9; }
                                    input[9] = c;
                                    if (c == '3') {
                                        if (read(0, &c, 1) == 0) { return 10; }
                                        input[10] = c;
                                        if (c == '~') {
                                            input[0] = CTRL_ENTER;
                                            return 1;
                                        }
                                        return 10;
                                    }
                                    return 9;
                                }
                                return 8;
                            }
                            return 7;
                        } else if (c == '7') {
                            if (read(0, &c, 1) == 0) { return 6; }
                            input[6] = c;
                            if (c == ';') {
                                if (read(0, &c, 1) == 0) { return 7; }
                                input[7] = c;
                                if (c == '9') {
                                    if (read(0, &c, 1) == 0) { return 8; }
                                    input[8] = c;
                                    if (c == '~') {
                                        input[0] = ESC;
                                        input[1] = CTRL_TAB;
                                        return 2;
                                    }
                                    return 9;
                                } else if (c == '1') {
                                    if (read(0, &c, 1) == 0) { return 8; }
                                    input[8] = c;
                                    if (c == '3') {
                                        if (read(0, &c, 1) == 0) { return 9; }
                                        input[9] = c;
                                        if (c == '~') {
                                            input[0] = ESC;
                                            input[1] = CTRL_ENTER;
                                            return 2;
                                        }
                                        return 10;
                                    }
                                    return 9;
                                }
                                return 8;
                            }
                            return 7;
                        }
                        return 6;
                    }
                    return 5;
                }
                return 4;
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
                case '<':    {
                    k = 0;

                    memset(buff, 0, sizeof(buff));
                    for (i = 0; read(0, &c, 1) && c != ';'; i += 1) { buff[i] = c; }
                    buff[i] = 0;
                    b = s_to_i(buff);

                    if (b >= 64) {
                        b = MOUSE_WHEEL_UP + (b - 64);
                    } else if (b >= 32) {
                        k  = MOUSE_DRAG;
                        b -= 32;
                    }

                    memset(buff, 0, sizeof(buff));
                    for (i = 0; read(0, &c, 1) && c != ';'; i += 1) { buff[i] = c; }
                    buff[i] = 0;
                    x = s_to_i(buff);

                    memset(buff, 0, sizeof(buff));
                    for (i = 0; read(0, &c, 1) && toupper(c) != 'M'; i += 1) { buff[i] = c; }
                    buff[i] = 0;
                    y = s_to_i(buff);

                    if (k != MOUSE_DRAG) {
                        k = (c == 'M') ? MOUSE_PRESS : MOUSE_RELEASE;
                    }

                    input[0] = MK_MOUSE(k, b, y, x);

                    break;
                }
            }
            return 1;
        }
    } else if (input[1] == 'O') { /* ESC O sequences. */
        switch (input[2]) {
            case 'A':    { input[0] = ARROW_UP;   break; }
            case 'B':    { input[0] = ARROW_DOWN; break; }
            case 'H':    { input[0] = HOME_KEY;   break; }
            case 'F':    { input[0] = END_KEY;    break; }
            case 'P':    { input[0] = FN1;        break; }
            case 'Q':    { input[0] = FN2;        break; }
            case 'R':    { input[0] = FN3;        break; }
            case 'S':    { input[0] = FN4;        break; }
        }
        return 1;
    }

    if (input[1] == ESC) {
        if (read(0, &c, 1)) {
            input[3] = c;
            if (input[2] == ESC && input[3] == ESC) { return 4; }
            return 1 + esc_sequence(input + 1);
        }
    }

    return 3;
}

int yed_read_key_sequences(int len, int *input) {
    int  seq_key,
         i,
         found_a_partial_match,
         keep_reading;
    char c;
    int  new_key;
    yed_key_sequence *seq_it;

    if (len == 0) { return 0; }

    new_key = (input[len - 1] == CTRL_H && ctrl_h_is_bs
                ? BACKSPACE
                : input[len - 1]);

    len -= 1;

    do {
        /* We have consumed a keystroke. */
        if (new_key == CTRL_H && ctrl_h_is_bs) {new_key = BACKSPACE; }

        input[len]    = new_key;
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
    } while (keep_reading && read(0, &c, 1) && ((new_key = c), len < MAX_SEQ_LEN));

    seq_key = yed_get_key_sequence(len, input);

    if (seq_key != KEY_NULL) {
        input[0] = seq_key;
        return 1;
    }

    return len;
}

int yed_read_mbyte_keys(char first_byte, int n_bytes) {
    int  i;
    char c;

    ASSERT(n_bytes > 1, "should not read more bytes for single byte char");

    ys->mbyte.bytes[0] = first_byte;

    for (i = 1; i < n_bytes; i += 1) {
        /*
         * Check if the read times out.
         * If this is the case, we're gonna indicate to
         * the caller that we could not get all of the bytes
         * that we needed.
         */
        if (read(0, &c, 1) == 0) { return 0; }

        ys->mbyte.bytes[i] = c;
    }

    return 1;
}

int yed_read_keys(int *input) {
    int       len;
    int       nread;
    char      c;
    int       n_bytes;
    yed_glyph g;

    len     = 0;

    ctrl_h_is_bs = yed_var_is_truthy("ctrl-h-is-backspace");

/*
 * BLOCKING(ish):
 **********************************************/
/*    while ((nread = read(0, &c, 1)) == 0); */
/**********************************************/

/*
 * NON-BLOCKING:
 **********************************************/
    nread = read(0, &c, 1);
    if (nread == 0)     { return 0; }
/**********************************************/


    if (nread == -1)    { return 0; }

    n_bytes = nread;

    if (c != 0) {
        if (c == CTRL_H && ctrl_h_is_bs) { c = BACKSPACE; }

        g.c     = c;
        n_bytes = yed_get_glyph_len(g);
    }

    if (c == ESC) {
        input[0] = c;

        len = esc_timeout(input);

        if (len == 3) {
            len = esc_sequence(input);
            if (len > 1) { goto do_seq; }
        }
    } else if (n_bytes > 1) {
        if (yed_read_mbyte_keys(g.c, n_bytes)) {
            input[0] = MBYTE;
            len      = 1;
        }
    } else if (c >= 0) {
        input[0] = c ? c : CTRL_SPACE;
        len      = 1;

do_seq:;
        if (!ys->interactive_command && !ys->doing_bracketed_paste) {
            len = yed_read_key_sequences(len, input);
        }
    }

    return len;
}

static int handle_bracketed_paste(int key) {
    char *str;
    int   key_ch;
    int   i;
    char  key_str_buff[32];
    char *key_str;

    if (key == _BRACKETED_PASTE_BEGIN) {
        if (!yed_var_is_truthy("bracketed-paste-mode")) {
            return 1;
        }

        ys->doing_bracketed_paste = 1;
        return 1;
    } else if (key == _BRACKETED_PASTE_END) {
        if (!yed_var_is_truthy("bracketed-paste-mode")) {
            return 1;
        }

        if (!ys->interactive_command) {
            array_zero_term(ys->bracketed_paste_buff);
            str = array_data(ys->bracketed_paste_buff);
            yed_execute_command("simple-insert-string", 1, &str);
        }

        ys->doing_bracketed_paste = 0;
        array_clear(ys->bracketed_paste_buff);
        return 1;
    } else if (ys->doing_bracketed_paste) {
        if (key < REAL_KEY_MAX
        &&  (key == ENTER || key == NEWLINE || key == TAB || key == MBYTE || !iscntrl(key))) {
            if (ys->interactive_command) {
                sprintf(key_str_buff, "%d", key);
                key_str = key_str_buff;

                yed_execute_command(ys->interactive_command, 1, &key_str);
            } else {
                if (key == MBYTE) {
                    for (i = 0; i < yed_get_glyph_len(ys->mbyte); i += 1) {
                        array_push(ys->bracketed_paste_buff, ys->mbyte.bytes[i]);
                    }
                } else {
                    if (key == NEWLINE) { key = ENTER; }
                    key_ch = (char)key;
                    array_push(ys->bracketed_paste_buff, key_ch);
                }
            }
        }
        return 1;
    }

    return 0;
}

void yed_take_key(int key) {
    yed_key_binding *binding;
    char             key_str_buff[32];
    char            *key_str;
    yed_event        event;

    if (handle_bracketed_paste(key)) {
        return;
    }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_KEY_PRESSED;
    event.key  = key;
    yed_trigger_event(&event);

    if (event.cancel) { return; }

    binding = yed_get_key_binding(key);

    sprintf(key_str_buff, "%d", key);
    key_str = key_str_buff;

    if (ys->interactive_command) {
        yed_execute_command(ys->interactive_command, 1, &key_str);
    } else if (binding) {
        yed_execute_command(binding->cmd, binding->n_args, binding->args);
    } else if (key > 0 && key < REAL_KEY_MAX
      &&      (key == ENTER || key == TAB || key == MBYTE || !iscntrl(key))) {
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
    { CTRL_Y,      "command-prompt",      0, NULL },
    { ARROW_UP,    "cursor-up",           0, NULL },
    { ARROW_DOWN,  "cursor-down",         0, NULL },
    { ARROW_RIGHT, "cursor-right",        0, NULL },
    { ARROW_LEFT,  "cursor-left",         0, NULL },
    { HOME_KEY,    "cursor-line-begin",   0, NULL },
    { END_KEY,     "cursor-line-end",     0, NULL },
    { PAGE_UP,     "cursor-page-up",      0, NULL },
    { PAGE_DOWN,   "cursor-page-down",    0, NULL },
    { BACKSPACE,   "delete-back",         0, NULL },
    { DEL_KEY,     "delete-forward",      0, NULL },
    { CTRL_C,      "yank-selection",      0, NULL },
    { CTRL_L,      "frame-next",          0, NULL },
    { CTRL_K,      "frame-delete",        0, NULL },
    { CTRL_D,      "delete-line",         0, NULL },
    { CTRL_S,      "select",              0, NULL },
    { CTRL_V,      "paste-yank-buffer",   0, NULL },
    { CTRL_W,      "write-buffer",        0, NULL },
    { CTRL_U,      "undo",                0, NULL },
    { CTRL_R,      "redo",                0, NULL },
    { CTRL_F,      "find-in-buffer",      0, NULL },
    { CTRL_N,      "find-next-in-buffer", 0, NULL },
    { CTRL_P,      "find-prev-in-buffer", 0, NULL },
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
    int             i;
    int             keys[2];
    yed_key_binding b;

    for (i = 0; i < sizeof(default_key_bindings) / sizeof(yed_key_binding); i += 1) {

        yed_bind_key(default_key_bindings[i]);
    }

    memset(&b, 0, sizeof(b));

    keys[0] = ESC;

    keys[1] = ARROW_UP;
    b.key = yed_add_key_sequence(2, keys);
    b.cmd = "cursor-prev-paragraph";
    yed_bind_key(b);

    keys[1] = ARROW_DOWN;
    b.key = yed_add_key_sequence(2, keys);
    b.cmd = "cursor-next-paragraph";
    yed_bind_key(b);

    keys[1] = ARROW_RIGHT;
    b.key = yed_add_key_sequence(2, keys);
    b.cmd = "cursor-next-word";
    yed_bind_key(b);

    keys[1] = ARROW_LEFT;
    b.key = yed_add_key_sequence(2, keys);
    b.cmd = "cursor-prev-word";
    yed_bind_key(b);
}

void yed_bind_key(yed_key_binding binding) {
    yed_event         event;
    yed_key_binding  *b;
    char            **args;
    int               i;

    LOG_FN_ENTER();

    if (binding.key == KEY_NULL)    { goto out; }

    memset(&event, 0, sizeof(event));
    event.kind     = EVENT_KEY_PRE_BIND;
    event.key      = binding.key;
    event.cmd_name = binding.cmd;
    event.n_args   = binding.n_args;
    event.args     = (const char * const*)binding.args;
    yed_trigger_event(&event);
    if (event.cancel) { return; }

    if (binding.key == CTRL_H
    &&  yed_var_is_truthy("ctrl-h-is-backspace")) {
        yed_log("[!] warning: Binding ctrl-h to '%s' while 'ctrl-h-is-backspace' is set.\n"
                "    Pressing backspace may invoke this binding depending on what your "
                "terminal sends when you press backspace.", binding.cmd);
    }

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

    event.kind = EVENT_KEY_POST_BIND;
    yed_trigger_event(&event);

out:
    LOG_EXIT();
}

void yed_unbind_key(int key) {
    yed_event                            event;
    tree_it(int, yed_key_binding_ptr_t)  it;
    yed_key_binding                     *old_binding;
    int                                  i;

    if (key == KEY_NULL)    { return; }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_KEY_PRE_UNBIND;
    event.key  = key;
    yed_trigger_event(&event);
    if (event.cancel) { return; }

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

    event.kind = EVENT_KEY_POST_UNBIND;
    yed_trigger_event(&event);
}

yed_key_binding * yed_get_key_binding(int key) {
    tree_it(int, yed_key_binding_ptr_t) it;

    if (key == KEY_NULL || IS_MOUSE(key))    { return NULL; }

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

int yed_get_real_keys(int key, int *len, int *real_keys) {
    yed_key_sequence *seq_it;
    int               i,
                      found;

    if (key < REAL_KEY_MAX) {
        *len       = 1;
        *real_keys = key;
        return 1;
    }

    found = 0;
    array_traverse(ys->key_sequences, seq_it) {
        if (seq_it->seq_key == key) {
            found = 1;
            break;
        }
    }

    if (!found)    { return 0; }

    *len = seq_it->len;

    for (i = 0; i < seq_it->len; i += 1) {
        real_keys[i] = seq_it->keys[i];
    }

    return 1;
}

int _yed_string_to_keys(const char *str, int *keys, int allow_meta) {
    char *str_dup;
    char *key_str;
    int  *key_ptr;
    int   n_keys;
    char *scan;
    int   save_wspc;
    char  key_c;
    int   key_i;
    int   meta;
    int   meta_result;
    int   fn_nr;

    str_dup = strdup(str);
    key_str = str_dup;
    key_ptr = keys;
    n_keys  = 0;

    while (*key_str) {
        if (n_keys == MAX_SEQ_LEN) {
            n_keys = -2;
            goto out;
        }

        if (is_space(*key_str)) {
            key_str += 1;
            continue;
        }

                     scan = strchr(key_str, ' ' );
        if (!scan) { scan = strchr(key_str, '\t'); }
        if (!scan) { scan = strchr(key_str, '\n'); }
        if (!scan) { scan = strchr(key_str, '\r'); }

        if (scan) { save_wspc = *scan; *scan = 0; }

        key_c = key_i = -1;
        meta  = 0;
        fn_nr = -1;

        if (strlen(key_str) == 1) {
            sscanf(key_str, "%c", &key_c);
            key_i = key_c;
        } else if (strcmp(key_str, "tab") == 0) {
            key_i = TAB;
        } else if (strcmp(key_str, "enter") == 0) {
            key_i = ENTER;
        } else if (strcmp(key_str, "esc") == 0) {
            key_i = ESC;
        } else if (strcmp(key_str, "spc") == 0) {
            key_i = ' ';
        } else if (strcmp(key_str, "bsp") == 0) {
            key_i = BACKSPACE;
        } else if (strcmp(key_str, "left") == 0) {
            key_i = ARROW_LEFT;
        } else if (strcmp(key_str, "right") == 0) {
            key_i = ARROW_RIGHT;
        } else if (strcmp(key_str, "up") == 0) {
            key_i = ARROW_UP;
        } else if (strcmp(key_str, "down") == 0) {
            key_i = ARROW_DOWN;
        } else if (strcmp(key_str, "del") == 0) {
            key_i = DEL_KEY;
        } else if (strcmp(key_str, "home") == 0) {
            key_i = HOME_KEY;
        } else if (strcmp(key_str, "end") == 0) {
            key_i = END_KEY;
        } else if (strcmp(key_str, "pageup") == 0) {
            key_i = PAGE_UP;
        } else if (strcmp(key_str, "pagedown") == 0) {
            key_i = PAGE_DOWN;
        } else if (strcmp(key_str, "shift-tab") == 0) {
            key_i = SHIFT_TAB;
        } else if (strcmp(key_str, "ctrl-tab") == 0) {
            key_i = CTRL_TAB;
        } else if (strcmp(key_str, "ctrl-enter") == 0) {
            key_i = CTRL_ENTER;
        } else if (strcmp(key_str, "ctrl-space") == 0) {
            key_i = CTRL_SPACE;
        } else if (sscanf(key_str, "ctrl-%c", &key_c)) {
            if (key_c != -1) {
                if (key_c == '/') {
                    key_i = CTRL_FS;
                } else {
                    key_i = CTRL_KEY(key_c);
                }
            }
        } else if (allow_meta && strncmp(key_str, "meta-", 5) == 0) {
            meta = 1;

            key_i     = ESC;
            *key_ptr  = key_i;
            key_ptr  += 1;
            n_keys   += 1;

            if (n_keys == MAX_SEQ_LEN) {
                n_keys = -2;
                goto out;
            }

            meta_result = _yed_string_to_keys(key_str + 5, key_ptr, 0);

            if (meta_result < 0) {
                n_keys = meta_result;
                goto out;
            }

            key_ptr += meta_result;
            n_keys  += meta_result;
        } else if (sscanf(key_str, "fn-%d", &fn_nr)) {
            if (fn_nr != -1) {
                if (fn_nr > 0 && fn_nr < 13) {
                    key_i = FN1 + (fn_nr - 1);
                }
            }
        }

        if (key_i == -1) {
            n_keys = -1;
            goto out;
        }

        if (!meta) {
            *key_ptr  = key_i;
            key_ptr  += 1;
            n_keys   += 1;
        }

        if (scan) {
            *scan = save_wspc;
            key_str = scan + 1;
        } else {
            key_str += strlen(key_str);
        }
    }

out:;
    free(str_dup);
    return n_keys;
}

int yed_string_to_keys(const char *str, int *keys) {
    return _yed_string_to_keys(str, keys, 1);
}

char *yed_keys_to_string(int n, int *keys) {
    char        buff[1024];
    const char *lazy_space;
    int         i;
    int         key;
    char       *key_str;
    char        key_buff[16];
    int         n_real_keys;
    int         real_keys[MAX_SEQ_LEN];
    char       *binding_string;

    buff[0]    = 0;
    lazy_space = "";

    for (i = 0; i < n; i += 1) {
        key     = keys[i];
        key_str = key_buff;

        switch (key) {
            case CTRL_A:
            case CTRL_B:
            case CTRL_C:
            case CTRL_D:
            case CTRL_E:
            case CTRL_F:
            case CTRL_G:
            case CTRL_H:
            case CTRL_J:
            case CTRL_K:
            case CTRL_L:
            case CTRL_N:
            case CTRL_O:
            case CTRL_P:
            case CTRL_Q:
            case CTRL_R:
            case CTRL_S:
            case CTRL_T:
            case CTRL_U:
            case CTRL_V:
            case CTRL_W:
            case CTRL_X:
            case CTRL_Y:
            case CTRL_Z:
                snprintf(key_buff, sizeof(key_buff), "ctrl-%c", 'a' + (key - CTRL_A));
                break;

            case TAB:
                snprintf(key_buff, sizeof(key_buff), "tab");
                break;

            case ' ':
                snprintf(key_buff, sizeof(key_buff), "spc");
                break;

            case ENTER:
                snprintf(key_buff, sizeof(key_buff), "enter");
                break;

            case ESC:
                snprintf(key_buff, sizeof(key_buff), "esc");
                break;

            case CTRL_FS:
                snprintf(key_buff, sizeof(key_buff), "ctrl-/");
                break;
            case CTRL_TAB:
                snprintf(key_buff, sizeof(key_buff), "ctrl-tab");
                break;
            case CTRL_ENTER:
                snprintf(key_buff, sizeof(key_buff), "ctrl-enter");
                break;
            case CTRL_SPACE:
                snprintf(key_buff, sizeof(key_buff), "ctrl-spc");
                break;

            case BACKSPACE:
                snprintf(key_buff, sizeof(key_buff), "bsp");
                break;

            case ARROW_LEFT:
                snprintf(key_buff, sizeof(key_buff), "left");
                break;
            case ARROW_RIGHT:
                snprintf(key_buff, sizeof(key_buff), "right");
                break;
            case ARROW_UP:
                snprintf(key_buff, sizeof(key_buff), "up");
                break;
            case ARROW_DOWN:
                snprintf(key_buff, sizeof(key_buff), "down");
                break;

            case DEL_KEY:
                snprintf(key_buff, sizeof(key_buff), "del");
                break;

            case HOME_KEY:
                snprintf(key_buff, sizeof(key_buff), "home");
                break;
            case END_KEY:
                snprintf(key_buff, sizeof(key_buff), "end");
                break;
            case PAGE_UP:
                snprintf(key_buff, sizeof(key_buff), "pageup");
                break;
            case PAGE_DOWN:
                snprintf(key_buff, sizeof(key_buff), "pagedown");
                break;

            case SHIFT_TAB:
                snprintf(key_buff, sizeof(key_buff), "shift-tab");
                break;

            case FN1:
            case FN2:
            case FN3:
            case FN4:
            case FN5:
            case FN6:
            case FN7:
            case FN8:
            case FN9:
            case FN10:
            case FN11:
            case FN12:
                snprintf(key_buff, sizeof(key_buff), "fn-%d", 1 + (key - FN1));
                break;

            default:
                if (!yed_is_key(key)) { goto bad; }

                if (key < ASCII_KEY_MAX) {
                    if (!isprint(key)) { goto bad; }
                    snprintf(key_buff, sizeof(key_buff), "%c", (char)key);
                } else if (key >= VIRT_KEY_START) {
                    if (!yed_get_real_keys(key, &n_real_keys, real_keys)) { goto bad; }

                    if (n_real_keys == 2 && real_keys[0] == ESC) {
                        binding_string = yed_keys_to_string(1, &real_keys[1]);
                        snprintf(key_buff, sizeof(key_buff), "meta-%s", binding_string);
                        free(binding_string);
                    } else {
                        binding_string = yed_keys_to_string(n_real_keys, real_keys);
                        if (binding_string == NULL) { goto bad; }
                        key_str = binding_string;
                    }
                } else {
bad:;
                    return NULL;
                }
        }

        strcat(buff, lazy_space); lazy_space = " ";
        strcat(buff, key_str);

        if (key_str != key_buff) { free(key_str); }
    }

    return strdup(buff);
}
