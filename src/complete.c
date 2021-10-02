void yed_init_completions(void) {
    ys->completions         = tree_make(yed_completion_name_t, yed_completion);
    ys->default_completions = tree_make(yed_completion_name_t, yed_completion);
    yed_set_default_completions();
}

void yed_set_completion(char *name, yed_completion compl) {
    tree_it(yed_completion_name_t, yed_completion) it;

    it = tree_lookup(ys->completions, name);

    if (tree_it_good(it)) {
        tree_insert(ys->completions, name, compl);
    } else {
        tree_insert(ys->completions, strdup(name), compl);
    }
}

void yed_unset_completion(char *name) {
    tree_it(yed_completion_name_t, yed_completion)  it;
    char                                           *old_key;

    it = tree_lookup(ys->completions, name);

    if (tree_it_good(it)) {
        old_key = tree_it_key(it);
        tree_delete(ys->completions, name);
        free(old_key);
    }
}

yed_completion yed_get_completion(char *name) {
    tree_it(yed_completion_name_t, yed_completion) it;

    it = tree_lookup(ys->completions, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

void yed_set_default_completion(char *name, yed_completion compl) {
    tree_it(yed_completion_name_t, yed_completion) it;

    it = tree_lookup(ys->default_completions, name);

    if (tree_it_good(it)) {
        tree_insert(ys->default_completions, name, compl);
    } else {
        tree_insert(ys->default_completions, strdup(name), compl);
    }
}

void yed_unset_default_completion(char *name) {
    tree_it(yed_completion_name_t, yed_completion)  it;
    char                                           *old_key;

    it = tree_lookup(ys->default_completions, name);

    if (tree_it_good(it)) {
        old_key = tree_it_key(it);
        tree_delete(ys->default_completions, name);
        free(old_key);
    }
}

yed_completion yed_get_default_completion(char *name) {
    tree_it(yed_completion_name_t, yed_completion) it;

    it = tree_lookup(ys->default_completions, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

static int yed_default_completion_commands(char *string, yed_completion_results *results) {
    tree_it(yed_command_name_t, yed_command) it;
    int                                      status;

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, ys->commands, it, results, status);

    return status;
}

static int yed_default_completion_styles(char *string, yed_completion_results *results) {
    tree_it(yed_style_name_t, yed_style_ptr_t) it;
    int                                        status;

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, ys->styles, it, results, status);

    return status;
}

static int yed_default_completion_buffers(char *string, yed_completion_results *results) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t) it;
    int                                          status;

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, ys->buffers, it, results, status);

    return status;
}

static int yed_default_completion_variables(char *string, yed_completion_results *results) {
    tree_it(yed_var_name_t, yed_var_val_t) it;
    int                                    status;

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, ys->vars, it, results, status);

    return status;
}

static int yed_default_completion_fts(char *string, yed_completion_results *results) {
    int status;

    FN_BODY_FOR_COMPLETE_FROM_ARRAY(string,
                                    array_len(ys->ft_array),
                                    (char**)array_data(ys->ft_array),
                                    results,
                                    status);

    return status;
}

static int yed_default_completion_plugins(char *string, yed_completion_results *results) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;
    int                                          status;

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, ys->plugins, it, results, status);

    return status;
}

static int yed_default_completion_files(char *string, yed_completion_results *results) {
    char                    *orig;
    int                      expanded;
    char                     expanded_path[4096];
    char                    *cpy;
    char                    *dirn;
    tree(str_t, empty_t)     t;
    char                     dir_path_buff[4096];
    DIR                     *dir;
    struct dirent           *dent;
    char                     full_path[4096];
    char                     homeified_path[4096];
    tree_it(str_t, empty_t)  it;
    char                    *key;
    int                      status;

    if (strcmp(string, "~") == 0) {
        array_clear(results->strings);
        cpy = strdup("~/");
        array_push(results->strings, cpy);
        return COMPL_ERR_NO_ERR;
    }

    orig = string;
    expand_path(string, expanded_path);
    expanded = !!strcmp(string, expanded_path);
    if (expanded) {
        string = expanded_path;
    }
    cpy    = strdup(string);
    dirn   = dirname(cpy);
    t      = tree_make(str_t, empty_t);

    if (strlen(string) > 0) {
        if (string[strlen(string) - 1] == '/') {
            strcpy(dir_path_buff, string);
            if (strlen(string) > 1) {
                dir_path_buff[strlen(dir_path_buff) - 1] = 0;
            }
            dirn = dir_path_buff;
        }
    }

    if ((dir = opendir(dirn)) == NULL) { goto out; }

    while ((dent = readdir(dir)) != NULL) {
        if (strcmp(dent->d_name, ".")  == 0
        ||  strcmp(dent->d_name, "..") == 0) {
            continue;
        }

        full_path[0] = 0;
        if (strcmp(dirn, ".") != 0) {
            strcat(full_path, dirn);
            if (strcmp(dirn, "/") != 0) {
                strcat(full_path, "/");
            }
        }
        strcat(full_path, dent->d_name);

        if (expanded) {
            if (homeify_path(full_path, homeified_path) != NULL) {
                tree_insert(t, strdup(homeified_path), (empty_t){});
            }
        } else {
            tree_insert(t, strdup(full_path), (empty_t){});
        }
    }

    string = orig;
    FN_BODY_FOR_COMPLETE_FROM_TREE(string, t, it, results, status);

out:;
    while (tree_len(t)) {
        it  = tree_begin(t);
        key = tree_it_key(it);
        tree_delete(t, key);
        free(key);
    }
    tree_free(t);
    free(cpy);

    return status;
}

static void get_all_line_words(tree(str_t, empty_t) words, yed_line *line) {
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

static void get_all_buff_words(tree(str_t, empty_t) words) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    yed_line                                     *line;

    tree_traverse(ys->buffers, it) {
        bucket_array_traverse(tree_it_val(it)->lines, line) {
            get_all_line_words(words, line);
        }
    }
}

static int yed_default_completion_words(char *string, yed_completion_results *results) {
    tree(str_t, empty_t)     t;
    tree_it(str_t, empty_t)  it;
    int                      status;
    char                    *key;

    t = tree_make(str_t, empty_t);
    get_all_buff_words(t);

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, t, it, results, status);

    while (tree_len(t)) {
        it  = tree_begin(t);
        key = tree_it_key(it);
        tree_delete(t, key);
        free(key);
    }
    tree_free(t);

    return status;
}


#define SET_DEFAULT_COMPL(name1, name2)        \
do {                                           \
    yed_set_completion(name1, &name2);         \
    yed_set_default_completion(name1, &name2); \
} while (0)

void yed_set_default_completions(void) {
    SET_DEFAULT_COMPL("command",                     yed_default_completion_commands);
    SET_DEFAULT_COMPL("buffer",                      yed_default_completion_buffers);
    SET_DEFAULT_COMPL("variable",                    yed_default_completion_variables);
    SET_DEFAULT_COMPL("style",                       yed_default_completion_styles);
    SET_DEFAULT_COMPL("ft",                          yed_default_completion_fts);
    SET_DEFAULT_COMPL("plugin",                      yed_default_completion_plugins);
    SET_DEFAULT_COMPL("file",                        yed_default_completion_files);
    SET_DEFAULT_COMPL("word",                        yed_default_completion_words);

    SET_DEFAULT_COMPL("bind-compl-arg-1",            yed_default_completion_commands);
    SET_DEFAULT_COMPL("buffer-compl-arg-0",          yed_default_completion_files);
    SET_DEFAULT_COMPL("buffer-delete-compl-arg-0",   yed_default_completion_buffers);
    SET_DEFAULT_COMPL("set-compl-arg-0",             yed_default_completion_variables);
    SET_DEFAULT_COMPL("get-compl-arg-0",             yed_default_completion_variables);
    SET_DEFAULT_COMPL("unset-compl-arg-0",           yed_default_completion_variables);
    SET_DEFAULT_COMPL("toggle-var-compl-arg-0",      yed_default_completion_variables);
    SET_DEFAULT_COMPL("style-compl-arg-0",           yed_default_completion_styles);
    SET_DEFAULT_COMPL("buffer-set-ft-compl-arg-0",   yed_default_completion_fts);
    SET_DEFAULT_COMPL("plugin-unload-compl-arg-0",   yed_default_completion_plugins);
    SET_DEFAULT_COMPL("plugin-toggle-compl-arg-0",   yed_default_completion_plugins);
    SET_DEFAULT_COMPL("plugin-path-compl-arg-0",     yed_default_completion_plugins);
    SET_DEFAULT_COMPL("find-in-buffer-compl-arg-0",  yed_default_completion_words);
    SET_DEFAULT_COMPL("plugins-add-dir-compl-arg-0", yed_default_completion_files);
}

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

int get_buff_word_completion(char *in, char ***out) {
    tree(str_t, empty_t)      words;
    tree_it(str_t, empty_t)   it;
    int                       in_len,
                              i,
                              n_items;
    char                    **items,
                             *key;

    words   = tree_make(str_t, empty_t);
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

int yed_complete(char *compl_name, char *string, yed_completion_results *results) {
    tree_it(yed_completion_name_t, yed_completion) it;
    yed_completion                                 compl;
    int                                            status;

    it = tree_lookup(ys->completions, compl_name);

    if (!tree_it_good(it)) {
        return COMPL_ERR_NO_COMPL;
    }

    compl            = tree_it_val(it);
    results->strings = array_make(char*);

    status = compl(string, results);

    if (status == COMPL_ERR_NO_ERR) {
        results->common_prefix_len =
            compute_common_prefix_len(string,
                                      array_len(results->strings),
                                      array_data(results->strings));
    }

    return status;
}

int yed_complete_multiple(int n, char **compl_names, char *string, yed_completion_results *results) {
    int                       status;
    tree(str_t, empty_t)      t;
    int                       i;
    yed_completion_results    tmp_results;
    char                    **str_it;
    tree_it(str_t, empty_t)   it;
    char                     *key;


    status = COMPL_ERR_NO_ERR;
    t      = tree_make(str_t, empty_t);

    for (i = 0; i < n; i += 1) {
        status = yed_complete(compl_names[i], string, &tmp_results);
        if (status != COMPL_ERR_NO_ERR && status != COMPL_ERR_NO_MATCH) { goto out; }
        array_traverse(tmp_results.strings, str_it) {
            it = tree_lookup(t, *str_it);
            if (!tree_it_good(it)) {
                tree_insert(t, strdup(*str_it), (empty_t){});
            }
        }
        free_string_array(tmp_results.strings);
    }

    results->strings = array_make(char*);

    FN_BODY_FOR_COMPLETE_FROM_TREE(string, t, it, results, status);

    results->common_prefix_len =
        compute_common_prefix_len(string,
                                  array_len(results->strings),
                                  array_data(results->strings));


out:;
    while (tree_len(t)) {
        it  = tree_begin(t);
        key = tree_it_key(it);
        tree_delete(t, key);
        free(key);
    }

    return status;
}
