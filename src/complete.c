int compute_common_prefix_len(char *in, int n_items, char **items) {
    int  i, len, max_len;

    if (n_items == 0)    { return 0; }

    len     = strlen(in);
    max_len = strlen(items[0]);

    while (len < max_len) {
        for (i = 1; i < n_items; i += 1) {
            if (items[i][len] != items[0][len]) {
                goto out;
            }
        }
        len += 1;
    }

out:
    return len;
}

int get_cmd_completion(char *in, char ***out) {
    tree_it(yed_command_name_t, yed_command)   it;
    int                                        in_len,
                                               i,
                                               n_items;
    char                                     **items,
                                              *key;

    if (!in)    { return 0; }

    in_len  = strlen(in);
    n_items = 0;

    /*
     * Count the number of commands that start with our 'in'
     * string. 'in' might be an empty string, in which case,
     * this method still gives us every command as a completion
     * item (which is what we want).
     */
    it  = tree_gtr(ys->commands, in);

    if (!tree_it_good(it))    { goto none; }

    i   = 0;
    key = tree_it_key(it);
    while (strncmp(key, in, in_len) == 0) {
        i += 1;
        tree_it_next(it);
        if (!tree_it_good(it)) {
            break;
        }
        key = tree_it_key(it);
    }

    n_items = i;

    if (n_items) {
        items = malloc(i * sizeof(char*));

        /* Now start collecting the command names. */
        it = tree_gtr(ys->commands, in);
        i  = 0;
        while (i < n_items) {
            items[i] = tree_it_key(it);
            tree_it_next(it);
            i += 1;
        }
    } else {
none:
        items = NULL;
    }

    *out = items;
    return n_items;
}

void get_all_line_words(tree(str_t, empty_t) words, yed_line *line) {
    int  col, start_col;
    char c, *word_start, *word;

    col = 1;

    while (col < line->visual_width) {
        start_col = col;

        c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

        if (isalnum(c) || c == '_') {
            while (col <= line->visual_width) {
                col += 1;
                c    = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

                if (!isalnum(c) && c != '_') {
                    break;
                }
            }

            word_start = array_data(line->chars)
                         + yed_line_col_to_idx(line, start_col);

            word = strndup(word_start, col - start_col);

            tree_insert(words, word, (empty_t){});
        } else if (!isspace(c)) {
            while (col <= line->visual_width) {
                col += 1;
                c    = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

                if (isalnum(c) || c == '_' || isspace(c)) {
                    break;
                }
            }
        }

        if (isspace(c)) {
            while (col <= line->visual_width) {
                col += 1;
                c    = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

                if (!isspace(c)) {
                    break;
                }
            }
        }
    }
}

void get_all_buff_words(tree(str_t, empty_t) words) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    yed_line                                     *line;

    tree_traverse(ys->buffers, it) {
        bucket_array_traverse(tree_it_val(it)->lines, line) {
            get_all_line_words(words, line);
        }
    }
}

int get_buff_word_completion(char *in, char ***out) {
    tree(str_t, empty_t)      words;
    tree_it(str_t, empty_t)   it;
    int                       in_len,
                              i,
                              n_items;
    char                    **items,
                             *key;

    words   = tree_make_c(str_t, empty_t, strcmp);
    in_len  = strlen(in);
    n_items = 0;

    get_all_buff_words(words);

    it  = tree_gtr(words, in);

    if (!tree_it_good(it))    { goto none; }

    i   = 0;
    key = tree_it_key(it);
    while (strncmp(key, in, in_len) == 0) {
        i += 1;
        tree_it_next(it);
        if (!tree_it_good(it)) {
            break;
        }
        key = tree_it_key(it);
    }

    n_items = i;

    if (n_items) {
        items = malloc(i * sizeof(char*));

        it = tree_gtr(words, in);
        i  = 0;
        while (i < n_items) {
            items[i] = tree_it_key(it);
            tree_it_next(it);
            i += 1;
        }
    } else {
none:
        items = NULL;
    }

    tree_free(words);

    *out = items;
    return n_items;
}


int yed_get_completion(int type, char *in, char ***out, int *common_prefix_len) {
    int    num_items;
    char **items;

    items     = NULL;
    num_items = 0;

    switch (type) {
        case COMPL_CMD:
            num_items = get_cmd_completion(in, &items);
            break;
        case COMPL_BUFF:
            num_items = get_buff_word_completion(in, &items);
            break;
    }

    if (items && common_prefix_len) {
        *common_prefix_len = compute_common_prefix_len(in, num_items, items);
    }

    if (out) {
        *out = items;
    }

    return num_items;
}
