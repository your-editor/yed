void yed_init_commands(void) {
    ys->commands         = tree_make(yed_command_name_t, yed_command);
    ys->default_commands = tree_make(yed_command_name_t, yed_command);
    yed_set_default_commands();

    ys->cmd_buff       = array_make(char);
    ys->log_name_stack = array_make(char*);

    ys->cmd_prompt_hist     = array_make(char*);
    ys->cmd_prompt_readline = malloc(sizeof(*ys->cmd_prompt_readline));
    yed_cmd_line_readline_make(ys->cmd_prompt_readline, &ys->cmd_prompt_hist);

    ys->search_hist     = array_make(char*);
    ys->search_readline = malloc(sizeof(*ys->search_readline));
    yed_cmd_line_readline_make(ys->search_readline, &ys->search_hist);
}

yed_command yed_get_command(char *name) {
    tree_it(yed_command_name_t, yed_command) it;

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

void yed_set_command(char *name, yed_command command) {
    tree_it(yed_command_name_t, yed_command)  it;

    it = tree_lookup(ys->commands, name);

    if (tree_it_good(it)) {
        tree_insert(ys->commands, name, command);
    } else {
        tree_insert(ys->commands, strdup(name), command);
    }
}

void yed_unset_command(char *name) {
    tree_it(yed_command_name_t, yed_command)  it;
    char                                     *old_key;

    it = tree_lookup(ys->commands, name);

    if (tree_it_good(it)) {
        old_key = tree_it_key(it);
        tree_delete(ys->commands, name);
        free(old_key);
    }
}

void yed_set_default_command(char *name, yed_command command) {
    tree_it(yed_command_name_t, yed_command)  it;

    it = tree_lookup(ys->default_commands, name);

    if (tree_it_good(it)) {
        tree_insert(ys->default_commands, name, command);
    } else {
        tree_insert(ys->default_commands, strdup(name), command);
    }
}

yed_command yed_get_default_command(char *name) {
    tree_it(yed_command_name_t, yed_command) it;

    it = tree_lookup(ys->default_commands, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

void yed_set_default_commands(void) {
#define SET_DEFAULT_COMMAND(name1, name2)                         \
do {                                                              \
    yed_set_command(name1, &yed_default_command_##name2);         \
    yed_set_default_command(name1, &yed_default_command_##name2); \
} while (0)

    SET_DEFAULT_COMMAND("command-prompt",                     command_prompt);
    SET_DEFAULT_COMMAND("quit",                               quit);
    SET_DEFAULT_COMMAND("reload",                             reload);
    SET_DEFAULT_COMMAND("reload-core",                        reload_core);
    SET_DEFAULT_COMMAND("redraw",                             redraw);
    SET_DEFAULT_COMMAND("set",                                set);
    SET_DEFAULT_COMMAND("get",                                get);
    SET_DEFAULT_COMMAND("unset",                              unset);
    SET_DEFAULT_COMMAND("toggle-var",                         toggle_var);
    SET_DEFAULT_COMMAND("sh",                                 sh);
    SET_DEFAULT_COMMAND("sh-silent",                          sh_silent);
    SET_DEFAULT_COMMAND("less",                               less);
    SET_DEFAULT_COMMAND("echo",                               echo);
    SET_DEFAULT_COMMAND("cursor-move",                        cursor_move);
    SET_DEFAULT_COMMAND("cursor-down",                        cursor_down);
    SET_DEFAULT_COMMAND("cursor-up",                          cursor_up);
    SET_DEFAULT_COMMAND("cursor-left",                        cursor_left);
    SET_DEFAULT_COMMAND("cursor-right",                       cursor_right);
    SET_DEFAULT_COMMAND("cursor-line-begin",                  cursor_line_begin);
    SET_DEFAULT_COMMAND("cursor-line-end",                    cursor_line_end);
    SET_DEFAULT_COMMAND("cursor-prev-word",                   cursor_prev_word);
    SET_DEFAULT_COMMAND("cursor-next-word",                   cursor_next_word);
    SET_DEFAULT_COMMAND("cursor-prev-paragraph",              cursor_prev_paragraph);
    SET_DEFAULT_COMMAND("cursor-next-paragraph",              cursor_next_paragraph);
    SET_DEFAULT_COMMAND("cursor-page-up",                     cursor_page_up);
    SET_DEFAULT_COMMAND("cursor-page-down",                   cursor_page_down);
    SET_DEFAULT_COMMAND("cursor-buffer-begin",                cursor_buffer_begin);
    SET_DEFAULT_COMMAND("cursor-buffer-end",                  cursor_buffer_end);
    SET_DEFAULT_COMMAND("cursor-line",                        cursor_line);
    SET_DEFAULT_COMMAND("word-under-cursor",                  word_under_cursor);
    SET_DEFAULT_COMMAND("buffer",                             buffer);
    SET_DEFAULT_COMMAND("buffer-delete",                      buffer_delete);
    SET_DEFAULT_COMMAND("buffer-next",                        buffer_next);
    SET_DEFAULT_COMMAND("buffer-prev",                        buffer_prev);
    SET_DEFAULT_COMMAND("buffer-name",                        buffer_name);
    SET_DEFAULT_COMMAND("buffer-path",                        buffer_path);
    SET_DEFAULT_COMMAND("buffer-set-ft",                      buffer_set_ft);
    SET_DEFAULT_COMMAND("buffer-reload",                      buffer_reload);
    SET_DEFAULT_COMMAND("frame-new",                          frame_new);
    SET_DEFAULT_COMMAND("frame-delete",                       frame_delete);
    SET_DEFAULT_COMMAND("frame-vsplit",                       frame_vsplit);
    SET_DEFAULT_COMMAND("frame-hsplit",                       frame_hsplit);
    SET_DEFAULT_COMMAND("frame-next",                         frame_next);
    SET_DEFAULT_COMMAND("frame-prev",                         frame_prev);
    SET_DEFAULT_COMMAND("frame-move",                         frame_move);
    SET_DEFAULT_COMMAND("frame-resize",                       frame_resize);
    SET_DEFAULT_COMMAND("insert",                             insert);
    SET_DEFAULT_COMMAND("simple-insert-string",               simple_insert_string);
    SET_DEFAULT_COMMAND("delete-back",                        delete_back);
    SET_DEFAULT_COMMAND("delete-forward",                     delete_forward);
    SET_DEFAULT_COMMAND("delete-line",                        delete_line);
    SET_DEFAULT_COMMAND("write-buffer",                       write_buffer);
    SET_DEFAULT_COMMAND("plugin-load",                        plugin_load);
    SET_DEFAULT_COMMAND("plugin-unload",                      plugin_unload);
    SET_DEFAULT_COMMAND("plugin-toggle",                      plugin_toggle);
    SET_DEFAULT_COMMAND("plugin-path",                        plugin_path);
    SET_DEFAULT_COMMAND("plugins-list",                       plugins_list);
    SET_DEFAULT_COMMAND("plugins-list-dirs",                  plugins_list_dirs);
    SET_DEFAULT_COMMAND("plugins-add-dir",                    plugins_add_dir);
    SET_DEFAULT_COMMAND("select",                             select);
    SET_DEFAULT_COMMAND("select-lines",                       select_lines);
    SET_DEFAULT_COMMAND("select-off",                         select_off);
    SET_DEFAULT_COMMAND("yank-selection",                     yank_selection);
    SET_DEFAULT_COMMAND("paste-yank-buffer",                  paste_yank_buffer);
    SET_DEFAULT_COMMAND("find-in-buffer",                     find_in_buffer);
    SET_DEFAULT_COMMAND("find-next-in-buffer",                find_next_in_buffer);
    SET_DEFAULT_COMMAND("find-prev-in-buffer",                find_prev_in_buffer);
    SET_DEFAULT_COMMAND("replace-current-search",             replace_current_search);
    SET_DEFAULT_COMMAND("style",                              style);
    SET_DEFAULT_COMMAND("style-off",                          style_off);
    SET_DEFAULT_COMMAND("styles-list",                        styles_list);
    SET_DEFAULT_COMMAND("undo",                               undo);
    SET_DEFAULT_COMMAND("redo",                               redo);
    SET_DEFAULT_COMMAND("bind",                               bind);
    SET_DEFAULT_COMMAND("unbind",                             unbind);
    SET_DEFAULT_COMMAND("multi",                              multi);
    SET_DEFAULT_COMMAND("suspend",                            suspend);
    SET_DEFAULT_COMMAND("scomps-list",                        scomps_list);
    SET_DEFAULT_COMMAND("version",                            version);
    SET_DEFAULT_COMMAND("show-bindings",                      show_bindings);
    SET_DEFAULT_COMMAND("show-vars",                          show_vars);
    SET_DEFAULT_COMMAND("special-buffer-prepare-focus",       special_buffer_prepare_focus);
    SET_DEFAULT_COMMAND("special-buffer-prepare-jump-focus",  special_buffer_prepare_jump_focus);
    SET_DEFAULT_COMMAND("special-buffer-prepare-unfocus",     special_buffer_prepare_unfocus);
    SET_DEFAULT_COMMAND("log",                                log);
    SET_DEFAULT_COMMAND("nop",                                nop);
    SET_DEFAULT_COMMAND("cursor-style",                       cursor_style);
    SET_DEFAULT_COMMAND("feed-keys",                          feed_keys);
    SET_DEFAULT_COMMAND("alias",                              alias);
    SET_DEFAULT_COMMAND("unalias",                            unalias);
    SET_DEFAULT_COMMAND("repeat",                             repeat);
}

void yed_clear_cmd_buff(void) {
    array_clear(ys->cmd_buff);
    array_zero_term(ys->cmd_buff);
    ys->cmd_cursor_x = 1;

    if (ys->cmd_prompt) {
        ys->cmd_cursor_x += yed_get_string_width(ys->cmd_prompt);
    }
}

void yed_append_to_cmd_buff(const char *s) {
    int i, len;

    len = strlen(s);
    for (i = 0; i < len; i += 1) {
        array_push(ys->cmd_buff, ((char*)s)[i]);
    }
}

void yed_append_text_to_cmd_buff(const char *s) {
    if (s) {
        yed_append_to_cmd_buff(s);
        ys->cmd_cursor_x += yed_get_string_width(s);
    }
}

void yed_append_non_text_to_cmd_buff(const char *s) {
    yed_append_to_cmd_buff(s);
}

void yed_append_int_to_cmd_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    yed_append_text_to_cmd_buff(s);
}

static int cprinted_len   = 0;

void yed_vcprint(char *fmt, va_list args) {
    char        buff[1024];
    int         len, i, j;
    char        spc, dot;
    va_list     args_copy;
    int         should_clear;
    const char *current_command;

    va_copy(args_copy, args);

    should_clear = yed_vlog(fmt, args) || ys->clear_cmd_output;
    if (should_clear) { ys->clear_cmd_output = 0; }

    if (ys->interactive_command) { return; }

    current_command = yed_top_log_name();

    if (should_clear) {
        yed_clear_cmd_buff();
        ys->cmd_prompt = YED_CMD_PROMPT;
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff(current_command);
        yed_append_text_to_cmd_buff(") ");
        cprinted_len = 3 + strlen(current_command);
    }

    len = vsnprintf(buff, sizeof(buff), fmt, args_copy);
    va_end(args_copy);

    if (len > sizeof(buff) - 1) {
        len = sizeof(buff) - 1;
    }

    spc = ' ';
    dot = '.';
    for (i = 0; i < len && cprinted_len < ys->term_cols - 3; i += 1) {
        if (buff[i] == '\n') {
            for (j = 0; j < 4 && cprinted_len < ys->term_cols - 3; j += 1) {
                array_push(ys->cmd_buff, spc);
                cprinted_len += 1;
            }
        } else if (buff[i] == '\t') {
            for (j = 0; j < ys->tabw && cprinted_len < ys->term_cols - 3; j += 1) {
                array_push(ys->cmd_buff, spc);
                cprinted_len += 1;
            }
        } else {
            array_push(ys->cmd_buff, buff[i]);
            cprinted_len += 1;
        }

        if (cprinted_len == ys->term_cols - 3) {
            array_push(ys->cmd_buff, dot);
            array_push(ys->cmd_buff, dot);
            array_push(ys->cmd_buff, dot);
            cprinted_len += 3;
        }
    }
}

void yed_vcerr(char *fmt, va_list args) {
    char       buff[1024];
    int        len, i, j;
    char       spc, dot;
    va_list    args_copy;
    int        should_clear;
    const char *current_command;
    yed_attrs  cmd_attr, attn_attr, err_attr;
    char       attr_buff[128];

    va_copy(args_copy, args);

/*     should_clear = yed_vlog(fmt, args) || ys->clear_cmd_output; */
/*     if (should_clear) { ys->clear_cmd_output = 0; } */

    yed_vlog(fmt, args);
    should_clear = 1;
    ys->clear_cmd_output = 0;

    if (ys->interactive_command) { return; }

    current_command = yed_top_log_name();

    if (should_clear) {
        yed_clear_cmd_buff();
        ys->cmd_prompt = YED_CMD_PROMPT;
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff(current_command);
        yed_append_text_to_cmd_buff(") ");
        cprinted_len = 3 + strlen(current_command);

        cmd_attr    = yed_active_style_get_command_line();
        attn_attr   = yed_active_style_get_attention();
        err_attr    = cmd_attr;
        err_attr.fg = attn_attr.fg;
        yed_get_attr_str(err_attr, attr_buff);
        yed_append_non_text_to_cmd_buff(attr_buff);
    }

    len = vsnprintf(buff, sizeof(buff), fmt, args_copy);
    va_end(args_copy);

    if (len > sizeof(buff) - 1) {
        len = sizeof(buff) - 1;
    }

    spc = ' ';
    dot = '.';
    for (i = 0; i < len && cprinted_len < ys->term_cols - 3; i += 1) {
        if (buff[i] == '\n') {
            for (j = 0; j < 4 && cprinted_len < ys->term_cols - 3; j += 1) {
                array_push(ys->cmd_buff, spc);
                cprinted_len += 1;
            }
        } else if (buff[i] == '\t') {
            for (j = 0; j < ys->tabw && cprinted_len < ys->term_cols - 3; j += 1) {
                array_push(ys->cmd_buff, spc);
                cprinted_len += 1;
            }
        } else {
            array_push(ys->cmd_buff, buff[i]);
            cprinted_len += 1;
        }

        if (cprinted_len == ys->term_cols - 3) {
            array_push(ys->cmd_buff, dot);
            array_push(ys->cmd_buff, dot);
            array_push(ys->cmd_buff, dot);
            cprinted_len += 3;
        }
    }

    if (should_clear && !ys->interactive_command) {
        yed_get_attr_str(cmd_attr, attr_buff);
        yed_append_non_text_to_cmd_buff(attr_buff);
    }
}

void yed_cprint(char *fmt, ...) {
    va_list va;

    va_start(va, fmt);
    yed_vcprint(fmt, va);
    va_end(va);
}

void yed_cerr(char *fmt, ...) {
    char    fmt_buff[256];
    va_list va;

    fmt_buff[0] = 0;
    strcat(fmt_buff, "[!] ");
    strcat(fmt_buff, fmt);
    fmt = fmt_buff;

    va_start(va, fmt);
    yed_vcerr(fmt, va);
    va_end(va);
}

void yed_cprint_clear(void) { ys->clear_cmd_output = 1; }

void yed_cmd_buff_push(char c) {
    array_push(ys->cmd_buff, c);
    ys->cmd_cursor_x += yed_get_glyph_width(G(c));
}

void yed_cmd_buff_pop(void) {
    char *c_p;

    c_p = array_last(ys->cmd_buff);
    ys->cmd_cursor_x -= yed_get_glyph_width(G(*c_p));
    array_pop(ys->cmd_buff);
}

void yed_cmd_buff_insert(int idx, char c) {
    array_insert(ys->cmd_buff, idx, c);
    ys->cmd_cursor_x += yed_get_glyph_width(G(c));
}

void yed_cmd_buff_delete(int idx) {
    array_delete(ys->cmd_buff, idx);
    ys->cmd_cursor_x -= 1;
}

void yed_draw_command_line() {
    int i;
    int j;

    yed_set_cursor(ys->term_rows, 1);
    yed_set_attr(yed_active_style_get_command_line());
    append_n_to_output_buff(ys->_4096_spaces, ys->term_cols);
    yed_set_cursor(ys->term_rows, 1);

    if (ys->interactive_command) {
        if (ys->cmd_prompt) {
            append_to_output_buff(ys->cmd_prompt);
        }
    }

    for (i = 0; i < array_len(ys->cmd_buff); i += 1) {
        if (*(char*)(array_data(ys->cmd_buff) + i) == '\t') {
            for (j = 0; j < ys->tabw; j += 1) {
                append_n_to_output_buff(" ", 1);
            }
        } else {
            append_n_to_output_buff(array_data(ys->cmd_buff) + i, 1);
        }
    }

    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
}

void yed_default_command_command_prompt(int n_args, char **args) {
    int key;

    if (!ys->interactive_command) {
        yed_builtin_cmd_prompt_start(n_args, args);
    } else {
        sscanf(args[0], "%d", &key);
        yed_builtin_cmd_prompt_take_key(key);
    }
}

void yed_default_command_quit(int n_args, char **args) {
    ys->status = YED_QUIT;
}

void yed_default_command_reload(int n_args, char **args) {
    ys->status = YED_RELOAD;
    yed_cprint("issued reload");
}

void yed_default_command_reload_core(int n_args, char **args) {
#ifdef CAN_RELOAD_CORE
    ys->status = YED_RELOAD_CORE;
    yed_cprint("issued core reload");
#else
    yed_cerr("core reloading is not supported on this platform");
#endif
}

void yed_default_command_redraw(int n_args, char **args) {
    ys->redraw = ys->redraw_cls = 1;
    yed_cprint("redrawing");
}

void yed_default_command_set(int n_args, char **args) {
    if (n_args != 2) {
        yed_cerr("expected 2 arguments, but got %d", n_args);
        return;
    }

    yed_set_var(args[0], args[1]);

    yed_cprint("set '%s' to '%s'", args[0], args[1]);
}

void yed_default_command_get(int n_args, char **args) {
    char *result;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    result = yed_get_var(args[0]);

    if (result) {
        yed_cprint("'%s' = '%s'", args[0], result);
    } else {
        yed_cprint("'%s' = undefined", args[0]);
    }
}

void yed_default_command_unset(int n_args, char **args) {
    char *result;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    result = yed_get_var(args[0]);

    if (result) {
        yed_unset_var(args[0]);
        yed_cprint("unset '%s'", args[0]);
    } else {
        yed_cerr("'%s' was not set", args[0]);
    }
}

void yed_default_command_toggle_var(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (yed_var_is_truthy(args[0])) {
        yed_set_var(args[0], "no");
    } else {
        yed_set_var(args[0], "yes");
    }
}

void yed_default_command_sh(int n_args, char **args) {
    char buff[1024],
         cmd_buff[512];
    const char *lazy_space;
    int i;

    buff[0]     = 0;
    cmd_buff[0] = 0;

    strcat(buff, "bash -c '(");

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        strcat(cmd_buff, lazy_space);
        strcat(cmd_buff, args[i]);
        lazy_space = " ";
    }

    strcat(buff, cmd_buff);
    strcat(buff, "); printf \"\\r\\n[ Hit any key to return to yed. ]\"; read -n 1'; printf \"\\n\\n\"");

    printf(TERM_CLEAR_SCREEN);
    printf(TERM_CURSOR_HOME);
    fflush(stdout);
    system(buff);
    printf(TERM_CLEAR_SCREEN);
    fflush(stdout);

    ys->redraw = 1;
}

void yed_default_command_sh_silent(int n_args, char **args) {
    char buff[1024],
         cmd_buff[512];
    const char *lazy_space;
    int i;
    int err;

    buff[0]     = 0;
    cmd_buff[0] = 0;

    strcat(buff, "bash -c '(");

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        strcat(cmd_buff, lazy_space);
        strcat(cmd_buff, args[i]);
        lazy_space = " ";
    }

    strcat(buff, cmd_buff);
    strcat(buff, ") 2>&1 > /dev/null' 2>&1 > /dev/null");

    err = system(buff);

    if (err == 0) {
        yed_cprint("%s", cmd_buff);
    } else {
        yed_cerr("'%s' exited with non-zero status %d", cmd_buff, err);
    }

    ys->redraw = 1;
}

void yed_default_command_less(int n_args, char **args) {
    char buff[1024], cmd_buff[1024];
    const char *lazy_space;
    int i;
    int err;

    buff[0] = 0;

    strcat(buff, "bash -c '(");

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        strcat(buff, lazy_space);
        strcat(buff, args[i]);
        lazy_space = " ";
    }

    strcpy(cmd_buff, buff + strlen("bash -c '("));

    strcat(buff, ") 2>&1 | less -cR'");

    printf(TERM_STD_SCREEN);
    fflush(stdout);
    err = system(buff);
    printf(TERM_ALT_SCREEN);
    fflush(stdout);

    if (err == 0) {
        yed_cprint("%s", cmd_buff);
    } else {
        yed_cerr("'%s' exited with non-zero status %d", cmd_buff, err);
    }

    ys->redraw = 1;
}

void yed_default_command_echo(int n_args, char **args) {
    int         i;
    const char *space;

    yed_cprint("");
    space = "";
    for (i = 0; i < n_args; i += 1) {
        yed_cprint("%s", (char*)space);
        yed_cprint("%s", args[i]);
        space = " ";
    }
}

void yed_default_command_cursor_move(int n_args, char **args) {
    yed_frame *frame;
    int        rows;
    int        cols;

    if (n_args < 1 || n_args > 2) {
        yed_cerr("expected 1 or 2 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    rows = s_to_i(args[0]);

    if (n_args == 1) {
        cols = 0;
    } else {
        cols = s_to_i(args[1]);
    }

    yed_move_cursor_within_active_frame(rows, cols);
}

void yed_default_command_cursor_down(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    if (n_args == 0) {
        rows = 1;
    } else {
        rows = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(rows, 0);
}

void yed_default_command_cursor_up(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }


    if (n_args == 0) {
        rows = -1;
    } else {
        rows = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(rows, 0);
}

void yed_default_command_cursor_left(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }


    if (n_args == 0) {
        cols = -1;
    } else {
        cols = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, cols);
}

void yed_default_command_cursor_right(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    if (n_args == 0) {
        cols = 1;
    } else {
        cols = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, cols);
}

void yed_default_command_cursor_line_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }


    yed_set_cursor_within_frame(frame, frame->cursor_line, 1);
}

void yed_default_command_cursor_line_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    yed_set_cursor_within_frame(frame, frame->cursor_line, line->visual_width + 1);
}

void yed_default_command_cursor_buffer_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    yed_set_cursor_far_within_frame(frame, 1, 1);
}

void yed_default_command_cursor_buffer_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *last_line;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    last_line = bucket_array_last(frame->buffer->lines);
    yed_set_cursor_far_within_frame(frame, bucket_array_len(frame->buffer->lines), last_line->visual_width + 1);
}

void yed_default_command_cursor_line(int n_args, char **args) {
    yed_frame *frame;
    int        line;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    sscanf(args[0], "%d", &line);
    yed_set_cursor_far_within_frame(frame, line, 1);
}

void yed_default_command_word_under_cursor(int n_args, char **args) {
    yed_frame *frame;
    char      *word;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    word = yed_word_under_cursor();

    if (word) {
        yed_cprint("'%s'", word);
        free(word);
    } else {
        yed_cerr("cursor is not on a word");
    }
}

void yed_default_command_cursor_prev_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        col, row;
    char       c;
    int        save_row;
    int        save_col;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

again:
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    col = frame->cursor_col - 1;

    if (col <= 0)    { goto skip_lines; }

    c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

    if (is_space(c)) {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (!is_space(c)) {
                break;
            }

            col -= 1;
        }
    }

    if (is_alnum(c) || c == '_') {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (!is_alnum(c) && c != '_') {
                break;
            }

            col -= 1;
        }
    } else {
        while (col > 1) {
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col - 1))->c;

            if (is_alnum(c) || c == '_' || is_space(c)) {
                break;
            }

            col -= 1;
        }
    }

    if (col == 0 || (col == 1 && (is_space(c)))) {
skip_lines:
        row = frame->cursor_line;
        do {
            row -= 1;
            line = yed_buff_get_line(frame->buffer, row);
        } while (line && !line->visual_width);

        if (line) {
            save_row = frame->cursor_line;
            save_col = frame->cursor_col;
            yed_move_cursor_within_frame(frame, row - frame->cursor_line, line->visual_width);
            if (save_row == frame->cursor_line && save_col == frame->cursor_col) { goto out; }
            goto again;
        }
    } else {
        yed_set_cursor_within_frame(frame, frame->cursor_line, col);
    }
out:;
}

void yed_default_command_cursor_next_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        col, row;
    char       c;
    int        save_row;
    int        save_col;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

again:
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    col  = frame->cursor_col;

    if (col >= line->visual_width) {
        goto skip_lines;
    }

    c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

    if (is_alnum(c) || c == '_') {
        while (col <= line->visual_width) {
            col += 1;
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

            if (!is_alnum(c) && c != '_') {
                break;
            }
        }
    } else if (!is_space(c)) {
        while (col <= line->visual_width) {
            col += 1;
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

            if (is_alnum(c) || c == '_' || is_space(c)) {
                break;
            }
        }
    }

    if (is_space(c)) {
        while (col <= line->visual_width) {
            col += 1;
            c = ((yed_glyph*)yed_line_col_to_glyph(line, col))->c;

            if (!is_space(c)) {
                break;
            }
        }
    }

    if (col == line->visual_width + 1) {
skip_lines:
        row = frame->cursor_line;
        do {
            row += 1;
            line = yed_buff_get_line(frame->buffer, row);
        } while (line && !line->visual_width);

        if (line) {
            save_row = frame->cursor_line;
            save_col = frame->cursor_col;

            yed_set_cursor_far_within_frame(frame, row, 1);

            if (save_row == frame->cursor_line && save_col == frame->cursor_col) { goto out; }

            line = yed_buff_get_line(frame->buffer, row);
            c    = ((yed_glyph*)yed_line_col_to_glyph(line, 1))->c;
            if (c && is_space(c)) {
                goto again;
            }
        }
    } else {
        yed_set_cursor_within_frame(frame, frame->cursor_line, col);
    }
out:;
}

void yed_default_command_cursor_prev_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
    int         i;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    i    = 0;


    /* Move through lines in this paragraph. */
    if (line && line->visual_width != 0) {
        while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
        &&      line->visual_width != 0) {
            i -= 1;
        }
    }

    /* Move through empty lines */
    while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
    &&      line->visual_width == 0) {
        i -= 1;
    }

    if (line && line->visual_width != 0) {
        while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i - 1))
        &&      line->visual_width != 0) {
            i -= 1;
        }
    }

    yed_move_cursor_within_frame(frame, i, 0);
}

void yed_default_command_cursor_next_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
    int         i;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    i    = 0;

    /* Move through lines in this paragraph. */
    if (line && line->visual_width != 0) {
        while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i + 1))
        &&      line->visual_width != 0) {
            i += 1;
        }
    }

    /* Move through empty lines */
    while ((line = yed_buff_get_line(frame->buffer, frame->cursor_line + i + 1))
    &&      line->visual_width == 0) {
        i += 1;
    }

    i += 1;

    yed_move_cursor_within_frame(frame, i, 0);
}

void yed_default_command_cursor_page_up(int n_args, char **args) {
    yed_frame  *frame;
    int         top_line;
    int         want_top_line;
    int         save_row;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    if (bucket_array_len(frame->buffer->lines) <= frame->height) {
        return;
    }

    top_line      = frame->buffer_y_offset + 1;
    want_top_line = top_line - frame->height;
    if (want_top_line <= 0) {
        want_top_line = 1;
    }
    while (top_line != want_top_line) {
        save_row = frame->cursor_line;
        yed_move_cursor_within_frame(frame, -1, 0);
        if (save_row == frame->cursor_line) { break; }
        top_line = frame->buffer_y_offset + 1;
    }
}

void yed_default_command_cursor_page_down(int n_args, char **args) {
    yed_frame  *frame;
    int         top_line;
    int         want_top_line;
    int         max_top_line;
    int         save_row;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }

    if (bucket_array_len(frame->buffer->lines) <= frame->height) {
        return;
    }

    top_line      = frame->buffer_y_offset + 1;
    want_top_line = top_line + frame->height;
    max_top_line  = bucket_array_len(frame->buffer->lines) - frame->height + 1;
    if (want_top_line >= max_top_line) {
        want_top_line = max_top_line;
    }
    while (top_line != want_top_line) {
        save_row = frame->cursor_line;
        yed_move_cursor_within_frame(frame, 1, 0);
        if (save_row == frame->cursor_line) { break; }
        top_line = frame->buffer_y_offset + 1;
    }
}

void yed_default_command_buffer(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_buffer                                   *lookup;
    char                                          a_path[4096];
    char                                          r_path[4096];
    char                                          h_path[4096];
    char                                         *name;
    yed_event                                     event;
    int                                           status;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    memset(&event, 0, sizeof(event));

    if (!ys->active_frame
    &&  array_len(ys->frames) == 0) {
        YEXE("frame-new");
    }

    abs_path(args[0], a_path);
    relative_path_if_subtree(a_path, r_path);
    if (homeify_path(r_path, h_path)) {
        name = h_path;
    } else {
        name = r_path;
    }

    lookup = yed_get_buffer(name);

    if (!lookup) {
        lookup = yed_get_buffer_by_path(a_path);
    }

    if (lookup) {
        buffer = lookup;

        if (ys->active_frame) {
            yed_frame_set_buff(ys->active_frame, buffer);
        }
    } else {
        event.kind = EVENT_BUFFER_PRE_LOAD;
        event.path = a_path;
        yed_trigger_event(&event);

        if (event.cancel) { goto out; }

        buffer = yed_create_buffer(name);
        status = yed_fill_buff_from_file(buffer, a_path);

        switch (status) {
            case BUFF_FILL_STATUS_ERR_DIR:
                yed_cerr("did not create buffer '%s' -- path is a directory", a_path);
                goto cleanup;
            case BUFF_FILL_STATUS_ERR_PER:
                yed_cerr("did not create buffer '%s' -- permission denied", a_path);
                goto cleanup;
            case BUFF_FILL_STATUS_ERR_MAP:
                yed_cerr("did not create buffer '%s' -- mmap() failed", a_path);
                goto cleanup;
            case BUFF_FILL_STATUS_ERR_UNK:
                yed_cerr("did not create buffer '%s' -- unknown error", a_path);
                goto cleanup;
            case BUFF_FILL_STATUS_ERR_NOF:
            case BUFF_FILL_STATUS_SUCCESS:
                yed_cprint("'%s' (new buffer)", name);

                event.buffer = buffer;

                if (status == BUFF_FILL_STATUS_ERR_NOF) {
                    yed_cprint(" (new file)");
                    buffer->path = strdup(a_path);
                    buffer->kind = BUFF_KIND_FILE;
                    yed_buffer_set_ft(buffer, FT_UNKNOWN);

                    event.buffer_is_new_file = 1;
                } else {
                    event.buffer_is_new_file = 0;
                }

                if (ys->active_frame) {
                    yed_frame_set_buff(ys->active_frame, buffer);
                }

                event.kind = EVENT_BUFFER_POST_LOAD;

                yed_trigger_event(&event);

                break;
        }
    }

    goto out;

cleanup:
    yed_free_buffer(buffer);

out:;
}

void yed_default_command_buffer_delete(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_frame                                    *frame;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_cerr("no active frame");
            return;
        }

        frame = ys->active_frame;

        if (!frame->buffer) {
            yed_cerr("active frame has no buffer");
            return;
        }

        buffer = frame->buffer;
    } else if (n_args == 1) {
        it = tree_lookup(ys->buffers, args[0]);
        if (tree_it_good(it)) {
            buffer = tree_it_val(it);
        } else {
            yed_cerr("no such buffer '%s'", args[0]);
            return;
        }
    } else {
        yed_cerr("expected 1 argument, but got %d");
        return;
    }

    if (buffer->flags & BUFF_SPECIAL) {
        yed_cerr("can't delete a special buffer");
        return;
    }


    yed_free_buffer(buffer);
}

#define BUFF_IT_NEXT_CYCLE(it)          \
do {                                    \
    tree_it_next(it);                   \
    if (!tree_it_good(it)) {            \
        (it) = tree_begin(ys->buffers); \
    }                                   \
} while (0)

#define BUFF_IT_PREV_CYCLE(it)         \
do {                                   \
    tree_it_prev(it);                  \
    if (!tree_it_good(it)) {           \
        (it) = tree_last(ys->buffers); \
    }                                  \
} while (0)

void yed_default_command_buffer_next(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_frame                                    *frame;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    int                                           i;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!tree_len(ys->buffers)) {
        yed_cerr("no buffers");
        return;
    }

    if (!frame->buffer) {
        it = tree_begin(ys->buffers);
    } else {
        buffer = frame->buffer;

        it = tree_lookup(ys->buffers, buffer->name);
        BUFF_IT_NEXT_CYCLE(it);
    }

    buffer = tree_it_val(it);

    if (buffer->flags & BUFF_SPECIAL) {
        for (i = 0; i < tree_len(ys->buffers); i += 1) {
            BUFF_IT_NEXT_CYCLE(it);
            buffer = tree_it_val(it);
            if (!(buffer->flags & BUFF_SPECIAL)) {
                break;
            }
        }
    }

    if (buffer->flags & BUFF_SPECIAL) {
        yed_cerr("no buffers");
        return;
    }

    yed_frame_set_buff(ys->active_frame, buffer);
}

void yed_default_command_buffer_prev(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_frame                                    *frame;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    int                                           i;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!tree_len(ys->buffers)) {
        yed_cerr("no buffers");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        it = tree_last(ys->buffers);
    } else {
        buffer = frame->buffer;

        it = tree_lookup(ys->buffers, buffer->name);
        BUFF_IT_PREV_CYCLE(it);
    }

    buffer = tree_it_val(it);

    if (buffer->flags & BUFF_SPECIAL) {
        for (i = 0; i < tree_len(ys->buffers); i += 1) {
            BUFF_IT_PREV_CYCLE(it);
            buffer = tree_it_val(it);
            if (!(buffer->flags & BUFF_SPECIAL)) {
                break;
            }
        }
    }

    if (buffer->flags & BUFF_SPECIAL) {
        yed_cerr("no buffers");
        return;
    }

    yed_frame_set_buff(ys->active_frame, buffer);
}

void yed_default_command_buffer_name(int n_args, char **args) {
    yed_buffer *buffer;
    yed_frame  *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    yed_cprint("'%s'", buffer->name);
}

void yed_default_command_buffer_path(int n_args, char **args) {
    yed_buffer *buffer;
    yed_frame  *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    if (buffer->name) {
        yed_cprint("%s", buffer->path);
    } else {
        yed_cerr("buffer has no path");
    }
}

void yed_default_command_buffer_set_ft(int n_args, char **args) {
    yed_buffer *buffer;
    yed_frame  *frame;
    char       *ft_str;
    int         ft_new;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }
    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    if (buffer->kind != BUFF_KIND_FILE
    ||  buffer->flags & BUFF_SPECIAL) {
        yed_cerr("can't change the file type of this buffer");
        return;
    }

    ft_str = args[0];
    ft_new = yed_get_ft(ft_str);

    if (ft_new == FT_ERR_NOT_FOUND) {
        yed_cerr("invalid file type name '%s'", args[0]);
        return;
    }

    yed_buffer_set_ft(buffer, ft_new);

    ys->redraw = 1;
}

void yed_default_command_buffer_reload(int n_args, char **args) {
    yed_frame                                    *frame;
    yed_buffer                                   *buffer;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;
    yed_event                                     event;
    int                                           line;
    int                                           col;
    int                                           status;

    frame = NULL;

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_cerr("no active frame");
            return;
        }

        frame = ys->active_frame;

        if (!frame->buffer) {
            yed_cerr("active frame has no buffer");
            return;
        }

        buffer = frame->buffer;
    } else if (n_args == 1) {
        it = tree_lookup(ys->buffers, args[0]);
        if (tree_it_good(it)) {
            buffer = tree_it_val(it);
        } else {
            yed_cerr("no such buffer '%s'", args[0]);
            return;
        }
    } else {
        yed_cerr("expected 0 or 1 arguments, but got %d");
        return;
    }

    if (buffer->path == NULL) {
        yed_cerr("buffer has no path");
        return;
    }

    event.kind = EVENT_BUFFER_PRE_LOAD;
    event.path = buffer->path;
    yed_trigger_event(&event);

    if (event.cancel) { return; }

    if (frame != NULL) {
        line = frame->cursor_line;
        col  = frame->cursor_col;
    }

    status = yed_fill_buff_from_file(buffer, buffer->path);

    switch (status) {
        case BUFF_FILL_STATUS_ERR_DIR:
            yed_cerr("did not reload buffer '%s' -- path is a directory", buffer->path);
            break;
        case BUFF_FILL_STATUS_ERR_PER:
            yed_cerr("did not reload buffer '%s' -- permission denied", buffer->path);
            break;
        case BUFF_FILL_STATUS_ERR_MAP:
            yed_cerr("did not reload buffer '%s' -- mmap() failed", buffer->path);
            break;
        case BUFF_FILL_STATUS_ERR_NOF:
            yed_cerr("did not reload buffer '%s' -- file not found", buffer->path);
            break;
        case BUFF_FILL_STATUS_ERR_UNK:
            yed_cerr("did not reload buffer '%s' -- unknown error", buffer->path);
            break;
        case BUFF_FILL_STATUS_SUCCESS:
            event.buffer             = buffer;
            event.buffer_is_new_file = 0;
            event.kind               = EVENT_BUFFER_POST_LOAD;
            yed_trigger_event(&event);

            if (frame != NULL && frame->buffer == buffer) {
                yed_set_cursor_far_within_frame(frame, 1, 1);
                yed_set_cursor_far_within_frame(frame, line, col);
            }

            break;
    }
}

void yed_default_command_frame_new(int n_args, char **args) {
    yed_frame *frame;
    float top_f, left_f, height_f, width_f;

    if (n_args == 0) {
        frame = yed_add_new_frame_full();
    } else if (n_args == 4) {
        sscanf(args[0], "%f", &top_f);
        sscanf(args[1], "%f", &left_f);
        sscanf(args[2], "%f", &height_f);
        sscanf(args[3], "%f", &width_f);

        LIMIT(top_f,    0.0, 1.0);
        LIMIT(left_f,   0.0, 1.0);
        LIMIT(height_f, 0.1, 1.0);
        LIMIT(width_f,  0.1, 1.0);

        frame = yed_add_new_frame(top_f, left_f, height_f, width_f);
    } else {
        yed_cerr("expected 0 or 4 arguments, but got %d", n_args);
        return;
    }

    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void yed_default_command_frame_delete(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }
    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    yed_delete_frame(frame);
}

void yed_default_command_frame_vsplit(int n_args, char **args) {
    yed_frame *new_frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    new_frame = yed_vsplit_frame(ys->active_frame);

    yed_activate_frame(new_frame);
}

void yed_default_command_frame_hsplit(int n_args, char **args) {
    yed_frame *new_frame;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    new_frame = yed_hsplit_frame(ys->active_frame);

    yed_activate_frame(new_frame);
}

void yed_default_command_frame_next(int n_args, char **args) {
    yed_frame *cur_frame, *frame, **frame_it;
    int        take_next;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    if (array_len(ys->frames) == 1) { return; }

    frame     = NULL;
    cur_frame = ys->active_frame;

    if (cur_frame == *(yed_frame**)array_last(ys->frames)) {
        frame = *(yed_frame**)array_item(ys->frames, 0);
    } else {
        take_next = 0;
        array_traverse(ys->frames, frame_it) {
            if (take_next) {
                frame = *frame_it;
                break;
            }

            if (*frame_it == cur_frame) {
                take_next = 1;
            }
        }
    }

    yed_activate_frame(frame);
}

void yed_default_command_frame_prev(int n_args, char **args) {
    yed_frame *cur_frame, *frame, **frame_it;
    int        i, cur_frame_idx;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    if (array_len(ys->frames) == 1) { return; }

    frame     = NULL;
    cur_frame = ys->active_frame;

    if (cur_frame == *(yed_frame**)array_item(ys->frames, 0)) {
        frame = *(yed_frame**)array_last(ys->frames);
    } else {
        i = cur_frame_idx = 0;
        array_traverse(ys->frames, frame_it) {
            if (*frame_it == cur_frame) {
                cur_frame_idx = i;
                break;
            }
            i += 1;
        }
        frame = *(yed_frame**)array_item(ys->frames, cur_frame_idx - 1);
    }

    yed_activate_frame(frame);
}

void frame_move_take_key(int key) {
    yed_frame *f;
    int        save;
    float      unit_x, unit_y;

    f      = ys->active_frame;
    unit_x = 1.0 / (float)ys->term_cols;
    unit_y = 1.0 / (float)ys->term_rows;

    if (key == CTRL_C || key == ENTER) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
        yed_frame_reset_cursor(f);
        yed_frame_hard_reset_cursor_x(f);
        ys->redraw     = 1;
        ys->redraw_cls = 1;
    } else if (key == ARROW_UP) {
        if (f->btop > 1) {
            yed_undraw_frame(f);
            save = f->btop;
            do {
                f->top_f -= unit_y;
                FRAME_RESET_RECT(f);
            } while (f->btop == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_DOWN) {
        if ((f->btop + 1) + f->bheight < ys->term_rows) {
            yed_undraw_frame(f);
            save = f->btop;
            do {
                f->top_f += unit_y;
                FRAME_RESET_RECT(f);
            } while (f->btop == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_LEFT) {
        if (f->bleft > 1) {
            yed_undraw_frame(f);
            save = f->bleft;
            do {
                f->left_f -= unit_x;
                FRAME_RESET_RECT(f);
            } while (f->bleft == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_RIGHT) {
        if ((f->bleft + 1) + f->bwidth - 1 < ys->term_cols + 1) {
            yed_undraw_frame(f);
            save = f->bleft;
            do {
                f->left_f += unit_x;
                FRAME_RESET_RECT(f);
            } while (f->bleft == save);
            ys->redraw = 1;
        }
    }
}

void start_frame_move(void) {
    ys->interactive_command = "frame-move";
    ys->cmd_prompt          =
        "(frame-move) Use arrows to move frame. ENTER to stop.";
    yed_clear_cmd_buff();
}

void yed_default_command_frame_move(int n_args, char **args) {
    yed_frame *frame;
    int        key;

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!ys->interactive_command) {
        if (!yed_frame_is_tree_root(frame)) {
            yed_cerr("frame is part of a split tree -- can't move");
            return;
        }

        start_frame_move();
    } else {
        sscanf(args[0], "%d", &key);
        frame_move_take_key(key);
    }
}

void frame_resize_take_key(int key) {
    yed_frame *f;
    int        save;
    float      unit_x, unit_y;

    f      = ys->active_frame;
    unit_x = 1.0 / (float)ys->term_cols;
    unit_y = 1.0 / (float)ys->term_rows;

    if (key == CTRL_C || key == ENTER) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
        yed_frame_reset_cursor(f);
        yed_frame_hard_reset_cursor_x(f);
        ys->redraw     = 1;
        ys->redraw_cls = 1;
    } else if (key == ARROW_UP) {
        if (f->height > 1) {
            yed_undraw_frame(f);
            save = f->bheight;
            do {
                f->height_f -= unit_y;
                FRAME_RESET_RECT(f);
            } while (f->bheight == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_DOWN) {
        if ((f->btop + 1) + f->bheight < ys->term_rows) {
            yed_undraw_frame(f);
            save = f->bheight;
            do {
                f->height_f += unit_y;
                FRAME_RESET_RECT(f);
            } while (f->bheight == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_LEFT) {
        if (f->width > 1) {
            yed_undraw_frame(f);
            save = f->bwidth;
            do {
                f->width_f -= unit_x;
                FRAME_RESET_RECT(f);
            } while (f->bwidth == save);
            ys->redraw = 1;
        }
    } else if (key == ARROW_RIGHT) {
        if ((f->bleft + 1) + f->bwidth - 1 < ys->term_cols + 1) {
            yed_undraw_frame(f);
            save = f->bwidth;
            do {
                f->width_f += unit_x;
                FRAME_RESET_RECT(f);
            } while (f->bwidth == save);
            ys->redraw = 1;
        }
    }
}

void start_frame_resize(void) {
    ys->interactive_command = "frame-resize";
    ys->cmd_prompt          =
        "(frame-resize) Use arrows to reize frame. ENTER to stop.";
    yed_clear_cmd_buff();
}

void yed_default_command_frame_resize(int n_args, char **args) {
    yed_frame *frame;
    int        key;

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!ys->interactive_command) {
        if (!yed_frame_is_tree_root(frame)) {
            yed_cerr("frame part of a split tree -- can't resize");
            return;
        }

        start_frame_resize();
    } else {
        sscanf(args[0], "%d", &key);
        frame_resize_take_key(key);
    }
}

void yed_default_command_insert(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    yed_event  event;
    int        key,
               col, idx,
               i, len,
               tabw;
    yed_glyph  g, *git;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    sscanf(args[0], "%d", &key);

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    col = frame->cursor_col;

    memset(&event, 0, sizeof(event));

    event.kind   = EVENT_BUFFER_PRE_INSERT;
    event.frame  = frame;
    event.buffer = frame->buffer;
    event.row    = frame->cursor_line;
    event.col    = col;
    event.key    = key;

    yed_trigger_event(&event);

    if (event.cancel) { return; }

    yed_start_undo_record(frame, frame->buffer);

    if (key == ENTER) {
        /*
         * Must get 'line' _after_ we've added the new line because
         * the array might have expanded and so the reference to
         * 'line' could be invalid if we had acquired it before.
         */
        yed_buff_insert_line(frame->buffer, frame->cursor_line + 1);
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);

        len = 0;
        idx = yed_line_col_to_idx(line, col);
        yed_line_glyph_traverse_from(*line, git, idx) {
            yed_append_to_line(frame->buffer, frame->cursor_line + 1, *git);
            len += 1;
        }
        for (; len > 0; len -= 1) {
            yed_pop_from_line(frame->buffer, frame->cursor_line);
        }

        yed_set_cursor_within_frame(frame, frame->cursor_line + 1, 1);
    } else {
        if (key == TAB) {
            tabw = yed_get_tab_width();
            g.c = ' ';
            for (i = 0; i < tabw; i += 1) {
                yed_insert_into_line(frame->buffer, frame->cursor_line, col + i, g);
            }
            yed_move_cursor_within_frame(frame, 0, tabw);
        } else {
            if (key == SHIFT_TAB) {
                g.c = '\t';
                yed_insert_into_line(frame->buffer, frame->cursor_line, col, g);
            } else if (key == MBYTE) {
                g = ys->mbyte;
                yed_insert_into_line(frame->buffer, frame->cursor_line, col, g);
            } else {
                g.c = key;
                yed_insert_into_line(frame->buffer, frame->cursor_line, col, g);
            }
            yed_move_cursor_within_frame(frame, 0, 1);
        }
    }

    yed_end_undo_record(frame, frame->buffer);

    frame->cursor_line_is_dirty = 1;

    event.kind = EVENT_BUFFER_POST_INSERT;
    yed_trigger_event(&event);

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }
}

void yed_default_command_simple_insert_string(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    int         row, col;
    int         tabw;
    char       *git;
    yed_glyph   g;
    int         i;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    row  = frame->cursor_line;
    col  = frame->cursor_col;
    tabw = yed_get_tab_width();
    git  = args[0];

    yed_start_undo_record(frame, buff);
    while (*git) {
        g = *(yed_glyph*)git;
        switch (*git) {
            case ENTER:
                yed_buff_insert_line(buff, row + 1);
                row += 1;
                col  = 1;
                break;
            case TAB:
                g.c = ' ';
                for (i = 0; i < tabw; i += 1) {
                    yed_insert_into_line(buff, row, col, g);
                    col += 1;
                }
                break;
            default:
                yed_insert_into_line(buff, row, col, g);
                col += 1;
        }
        git += yed_get_glyph_len(g);
    }
    yed_end_undo_record(frame, buff);

    frame->cursor_line_is_dirty = 1;

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }
}

void yed_default_command_delete_back(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *previous_line;
    yed_event  event;
    int        r1, c1, r2, c2,
               col,
               buff_n_lines,
               prev_line_width, old_line_nr;
    yed_glyph *git;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    memset(&event, 0, sizeof(event));
    event.kind   = EVENT_BUFFER_PRE_DELETE_BACK;
    event.frame  = frame;
    event.buffer = frame->buffer;
    event.row    = frame->cursor_line;
    yed_trigger_event(&event);

    yed_start_undo_record(frame, frame->buffer);

    if (frame->buffer->has_selection) {
        r1 = c1 = r2 = c2 = 0;
        yed_range_sorted_points(&frame->buffer->selection, &r1, &c1, &r2, &c2);
        frame->buffer->selection.locked = 1;
        if (frame->buffer->selection.kind == RANGE_LINE) {
            r1 = MIN(r1, yed_buff_n_lines(frame->buffer) - (r2 - r1) - 1);
            yed_set_cursor_far_within_frame(frame, r1, 1);
        } else {
            yed_set_cursor_far_within_frame(frame, r1, c1);
        }
        yed_buff_delete_selection(frame->buffer);
    } else {
        col  = frame->cursor_col;
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == 1) {
            if (frame->cursor_line > 1) {
                old_line_nr     = frame->cursor_line;
                previous_line   = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
                prev_line_width = previous_line->visual_width;

                yed_line_glyph_traverse(*line, git) {
                    yed_append_to_line(frame->buffer, frame->cursor_line - 1, *git);
                }

                /*
                 * Move to the previous line.
                 */
                yed_set_cursor_within_frame(frame, frame->cursor_line - 1, prev_line_width + 1);

                /*
                 * Kinda hacky, but this will help us pull the buffer
                 * up if there is a buffer_y_offset and there's content
                 * to pull up.
                 */
                buff_n_lines = yed_buff_n_lines(frame->buffer);
                if (frame->buffer_y_offset >= buff_n_lines - frame->height) {
                    yed_frame_reset_cursor(frame);
                }

                /*
                 * Delete the old line.
                 */
                yed_buff_delete_line(frame->buffer, old_line_nr);
            }
        } else {
            yed_move_cursor_within_frame(frame, 0, -1);
            yed_delete_from_line(frame->buffer, frame->cursor_line, col - 1);
        }
    }

    yed_end_undo_record(frame, frame->buffer);

    frame->cursor_line_is_dirty = 1;

    event.kind  = EVENT_BUFFER_POST_DELETE_BACK;
    yed_trigger_event(&event);

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }
}

void yed_default_command_delete_forward(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *next_line;
    int        col;
    int        r1, c1, r2, c2;
    int        buff_n_lines;
    yed_glyph *git;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    col = frame->cursor_col;

    yed_start_undo_record(frame, frame->buffer);

    if (frame->buffer->has_selection) {
        r1 = c1 = r2 = c2 = 0;
        yed_range_sorted_points(&frame->buffer->selection, &r1, &c1, &r2, &c2);
        frame->buffer->selection.locked = 1;
        if (frame->buffer->selection.kind == RANGE_LINE) {
            r1 = MIN(r1, yed_buff_n_lines(frame->buffer) - (r2 - r1) - 1);
            yed_set_cursor_far_within_frame(frame, r1, 1);
        } else {
            yed_set_cursor_far_within_frame(frame, r1, c1);
        }
        yed_buff_delete_selection(frame->buffer);
    } else {
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == line->visual_width + 1) {
            if (frame->cursor_line < yed_buff_n_lines(frame->buffer)) {
                next_line = yed_buff_get_line(frame->buffer, frame->cursor_line + 1);

                yed_line_glyph_traverse(*next_line, git) {
                    yed_append_to_line(frame->buffer, frame->cursor_line, *git);
                }

                /*
                 * Kinda hacky, but this will help us pull the buffer
                 * up if there is a buffer_y_offset and there's content
                 * to pull up.
                 */
                buff_n_lines = yed_buff_n_lines(frame->buffer);
                if (frame->buffer_y_offset >= buff_n_lines - frame->height) {
                    yed_frame_reset_cursor(frame);
                }

                /*
                 * Delete the old line.
                 */
                yed_buff_delete_line(frame->buffer, frame->cursor_line + 1);
            }
        } else {
            yed_delete_from_line(frame->buffer, frame->cursor_line, col);
        }
    }

    yed_end_undo_record(frame, frame->buffer);

    frame->cursor_line_is_dirty = 1;

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }
}

void yed_default_command_delete_line(int n_args, char **args) {
    yed_frame *frame;
    int        row,
               n_lines;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    n_lines = bucket_array_len(frame->buffer->lines);
    row     = frame->cursor_line;

    yed_start_undo_record(frame, frame->buffer);

    if (n_lines > 1) {
        if (row == n_lines) {
            yed_move_cursor_within_frame(frame, -1, 0);
        }
        yed_buff_delete_line(frame->buffer, row);
    } else {
        ASSERT(row == 1, "only one line, but not on line one");

        yed_set_cursor_within_frame(frame, 1, 1);
        yed_line_clear(frame->buffer, row);
    }

    /*
     * Kinda hacky, but this will help us pull the buffer
     * up if there is a buffer_y_offset and there's content
     * to pull up.
     */
    n_lines = bucket_array_len(frame->buffer->lines);
    if (frame->buffer_y_offset >= n_lines - frame->height) {
        yed_frame_reset_cursor(frame);
    }

    yed_frame_hard_reset_cursor_x(frame);

    yed_end_undo_record(frame, frame->buffer);

    frame->cursor_line_is_dirty = 1;

    if (ys->current_search) {
        if (yed_var_is_truthy("cursor-move-clears-search")) {
            ys->current_search = NULL;
            frame->dirty = 1;
        }
    }
}

void yed_default_command_write_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    char       *pretty_path;
    char       *path;
    char        exp_path[4096];
    int         status;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (n_args == 0) {
        if (buff->flags & BUFF_SPECIAL) {
            yed_cerr("special buffer doesn't have a path -- you must provide the file name");
            return;
        } else if (buff->path == NULL) {
            yed_cerr("buffer is not associated with a file yet -- provide a file name");
            return;
        }
        pretty_path = buff->name;
        relative_path_if_subtree(buff->name, exp_path);
        path = exp_path;
    } else if (n_args == 1) {
        pretty_path = args[0];
        relative_path_if_subtree(args[0], exp_path);
        path = exp_path;
    } else {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    status = yed_write_buff_to_file(buff, path);

    switch (status) {
        case BUFF_WRITE_STATUS_ERR_DIR:
            yed_cerr("did not write to '%s' -- path is a directory", pretty_path);
            break;
        case BUFF_WRITE_STATUS_ERR_PER:
            yed_cerr("did not write to '%s' -- permission denied", pretty_path);
            break;
        case BUFF_WRITE_STATUS_ERR_UNK:
            yed_cerr("did not write to '%s' -- unknown error", pretty_path);
            break;
        case BUFF_WRITE_STATUS_SUCCESS:
            yed_cprint("wrote to '%s'", pretty_path);
            break;
    }
}

void yed_default_command_plugin_load(int n_args, char **args) {
    int   err;
    char *dlerr;
    char  err_buff[512];

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    err = yed_load_plugin(args[0]);
    if (err == YED_PLUG_SUCCESS) {
        yed_cprint("loaded plugin '%s'", args[0]);
    } else {
        err_buff[0] = 0;
        sprintf(err_buff, "('%s') -- ", args[0]);

        switch (err) {
            case YED_PLUG_NOT_FOUND:
                sprintf(err_buff + strlen(err_buff), "could not find plugin");
                break;
            case YED_PLUG_DLOAD_FAIL:
                dlerr = dlerror();
                if (dlerr) {
                    sprintf(err_buff + strlen(err_buff), "%s\nthe plugin failed to load due to dynamic-loading errors", dlerr);
                } else {
                    sprintf(err_buff + strlen(err_buff), "failed to load plugin for unknown reason");
                }
                break;
            case YED_PLUG_NO_BOOT:
                sprintf(err_buff + strlen(err_buff), "could not find symbol 'yed_plugin_boot'");
                break;
            case YED_PLUG_BOOT_FAIL:
                sprintf(err_buff + strlen(err_buff), "'yed_plugin_boot' failed");
                break;
            case YED_PLUG_VER_MIS:
                yed_log("\n[!] the plugin was rejected because it was compiled against an older version of yed and is not compatible with this version");
                break;
            case YED_PLUG_LOAD_CANCEL:
                yed_log("\n[!] the plugin was not loaded because an event handler cancelled the load");
                break;
        }

        yed_cerr("%s", err_buff);
    }
}

void yed_default_command_plugin_unload(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    it = tree_lookup(ys->plugins, args[0]);

    if (!tree_it_good(it)) {
        yed_cerr("no plugin named '%s' is loaded", args[0]);
        return;
    }

    if (yed_unload_plugin(args[0]) == 0) {
        yed_cprint("successfully unloaded plugin '%s'", args[0]);
    } else {
        yed_cerr("did not unload plugin '%s'", args[0]);
    }
}

void yed_default_command_plugin_toggle(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t)  it;
    int                                           err;
    char                                         *dlerr;
    char                                          err_buff[512];


    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    it = tree_lookup(ys->plugins, args[0]);

    if (tree_it_good(it)) {
        if (yed_unload_plugin(args[0]) == 0) {
            yed_cprint("successfully unloaded plugin '%s'", args[0]);
        } else {
            yed_cerr("did not unload plugin '%s'", args[0]);
        }
    } else {
        err = yed_load_plugin(args[0]);
        if (err == YED_PLUG_SUCCESS) {
            yed_cprint("loaded plugin '%s'", args[0]);
        } else {
            err_buff[0] = 0;
            sprintf(err_buff, "('%s') -- ", args[0]);

            switch (err) {
                case YED_PLUG_NOT_FOUND:
                    sprintf(err_buff + strlen(err_buff), "could not find plugin");
                    break;
                case YED_PLUG_DLOAD_FAIL:
                    dlerr = dlerror();
                    if (dlerr) {
                        sprintf(err_buff + strlen(err_buff), "%s\nthe plugin failed to load due to dynamic-loading errors", dlerr);
                    } else {
                        sprintf(err_buff + strlen(err_buff), "failed to load plugin for unknown reason");
                    }
                    break;
                case YED_PLUG_NO_BOOT:
                    sprintf(err_buff + strlen(err_buff), "could not find symbol 'yed_plugin_boot'");
                    break;
                case YED_PLUG_BOOT_FAIL:
                    sprintf(err_buff + strlen(err_buff), "'yed_plugin_boot' failed");
                    break;
                case YED_PLUG_VER_MIS:
                    yed_log("\n[!] the plugin was rejected because it was compiled against an older version of yed and is not compatible with this version");
                    break;
                case YED_PLUG_LOAD_CANCEL:
                    yed_log("\n[!] the plugin was not loaded because an event handler cancelled the load");
                    break;
            }

            yed_cerr("%s", err_buff);
        }
    }
}

void yed_default_command_plugin_path(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    it = tree_lookup(ys->plugins, args[0]);

    if (!tree_it_good(it)) {
        yed_cerr("no plugin named '%s' is loaded", args[0]);
        return;
    }

    yed_cprint("%s", tree_it_val(it)->path);
}

void yed_default_command_plugins_list(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("");
    tree_traverse(ys->plugins, it) {
        yed_cprint("\n%s", tree_it_key(it));
    }
}

void yed_default_command_plugins_add_dir(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    yed_add_plugin_dir(args[0]);
    yed_cprint("added plugin directory '%s'", args[0]);
}

void yed_default_command_plugins_list_dirs(int n_args, char **args) {
    char **dir_it;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("");
    array_traverse(ys->plugin_dirs, dir_it) {
        yed_cprint("\n%s", *dir_it);
    }
}

void yed_default_command_select(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    buff->has_selection        = !buff->has_selection || buff->selection.kind != RANGE_NORMAL;
    buff->selection.kind       = RANGE_NORMAL;
    buff->selection.locked     = 0;
    buff->selection.anchor_row = frame->cursor_line;
    buff->selection.anchor_col = frame->cursor_col;
    buff->selection.cursor_row = frame->cursor_line;
    buff->selection.cursor_col = frame->cursor_col;
    frame->dirty               = 1;
}


void yed_default_command_select_lines(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    buff->has_selection        = !buff->has_selection || buff->selection.kind != RANGE_LINE;
    buff->selection.kind       = RANGE_LINE;
    buff->selection.locked     = 0;
    buff->selection.anchor_row = frame->cursor_line;
    buff->selection.anchor_col = frame->cursor_col;
    buff->selection.cursor_row = frame->cursor_line;
    buff->selection.cursor_col = frame->cursor_col;
    frame->dirty               = 1;
}

void yed_default_command_select_off(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    frame->dirty        = frame->dirty || buff->has_selection;
    buff->has_selection = 0;
}

void yed_default_command_yank_selection(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_buffer *yank_buff;
    yed_line   *line_it;
    yed_range  *sel;
    int         preserve_selection;
    int         row, col, yrow, r1, c1, r2, c2;
    yed_glyph  *g;

    if (n_args > 1) {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }

    if (n_args == 1) {
        preserve_selection = s_to_i(args[0]);
    } else {
        preserve_selection = 0;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind == BUFF_KIND_YANK) {
        yed_cerr("can't yank from yank buffer");
        return;
    }

    if (!buff->has_selection) {
        yed_cerr("nothing is selected");
        return;
    }

    yank_buff = yed_get_yank_buffer();
    yank_buff->flags &= ~BUFF_RD_ONLY;

    /*
     * Clear out what's in the yank buffer.
     * yed_buff_clear() leaves a line in the buffer,
     * so we need to delete that too.
     */
    yed_buff_clear_no_undo(yank_buff);
    yed_buff_delete_line(yank_buff, 1);


    /* Copy the selection into the yank buffer. */
    sel = &buff->selection;
    r1  = c1 = r2 = c2 = 0;
    yed_range_sorted_points(sel, &r1, &c1, &r2, &c2);
    if (sel->kind == RANGE_LINE) {
        yank_buff->flags |= BUFF_YANK_LINES;
        for (row = r1; row <= r2; row += 1) {
            yrow    = yed_buffer_add_line(yank_buff);
            line_it = yed_buff_get_line(buff, row);
            for (col = 1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(yank_buff, yrow, *g);
                col += yed_get_glyph_width(*g);
            }
        }
    } else {
        yank_buff->flags &= ~(BUFF_YANK_LINES);
        yrow    = yed_buffer_add_line(yank_buff);
        line_it = yed_buff_get_line(buff, r1);
        if (r1 == r2) {
            for (col = c1; col < c2;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(yank_buff, yrow, *g);
                col += yed_get_glyph_width(*g);
            }
        } else {
            for (col = c1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(yank_buff, yrow, *g);
                col += yed_get_glyph_width(*g);
            }
            for (row = r1 + 1; row <= r2 - 1; row += 1) {
                yrow    = yed_buffer_add_line(yank_buff);
                line_it = yed_buff_get_line(buff, row);
                for (col = 1; col <= line_it->visual_width;) {
                    g = yed_line_col_to_glyph(line_it, col);
                    yed_append_to_line(yank_buff, yrow, *g);
                    col += yed_get_glyph_width(*g);
                }
            }
            yrow    = yed_buffer_add_line(yank_buff);
            line_it = yed_buff_get_line(buff, r2);
            for (col = 1; col < c2;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(yank_buff, yrow, *g);
                col += yed_get_glyph_width(*g);
            }
        }
    }

    yank_buff->flags |= BUFF_RD_ONLY;

    if (!preserve_selection) {
        buff->has_selection = 0;
    }
    frame->dirty = 1;
}

void yed_default_command_paste_yank_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_buffer *yank_buff;
    yed_line   *line_it,
               *first_line;
    int         yank_buff_n_lines, first_row, last_row, new_row, row, col;
    yed_glyph  *g;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind == BUFF_KIND_YANK) {
        yed_cerr("can't paste into yank buffer");
        return;
    }

    if (buff->flags & BUFF_RD_ONLY) {
        yed_cerr("buffer is read-only");
        return;
    }

    yed_start_undo_record(frame, frame->buffer);

    yank_buff = yed_get_yank_buffer();
    yank_buff_n_lines = yed_buff_n_lines(yank_buff);

    ASSERT(yank_buff_n_lines, "yank buffer has no lines");

    if (yank_buff->flags & BUFF_YANK_LINES) {
        for (row = 1; row <= yank_buff_n_lines; row += 1) {
            new_row = frame->cursor_line + row;
            yed_buff_insert_line(buff, new_row);
            line_it = yed_buff_get_line(yank_buff, row);
            for (col = 1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(buff, new_row, *g);
                col += yed_get_glyph_width(*g);
            }
        }
        yed_set_cursor_far_within_frame(frame, frame->cursor_line + 1, 1);
    } else {
        if (yank_buff_n_lines == 1) {
            line_it  = yed_buff_get_line(yank_buff, 1);
            for (col = 1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_insert_into_line(buff,
                    frame->cursor_line,
                    frame->cursor_col + col - 1,
                    *g);
                col += yed_get_glyph_width(*g);
            }
        } else {
            last_row   = frame->cursor_line + 1;
            yed_buff_insert_line(buff, last_row);
            first_row  = frame->cursor_line;
            first_line = yed_buff_get_line(buff, first_row);
            for (col = frame->cursor_col; col <= first_line->visual_width;) {
                g = yed_line_col_to_glyph(first_line, col);
                yed_append_to_line(buff, last_row, *g);
                col += yed_get_glyph_width(*g);
            }
            while (first_line->visual_width >= frame->cursor_col) {
                yed_pop_from_line(buff, first_row);
            }
            line_it  = yed_buff_get_line(yank_buff, 1);
            for (col = 1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_append_to_line(buff, first_row, *g);
                col += yed_get_glyph_width(*g);
            }
            for (row = 2; row <= yank_buff_n_lines - 1; row += 1) {
                new_row = frame->cursor_line + row - 1;
                yed_buff_insert_line(buff, new_row);
                line_it = yed_buff_get_line(yank_buff, row);
                for (col = 1; col <= line_it->visual_width;) {
                    g = yed_line_col_to_glyph(line_it, col);
                    yed_append_to_line(buff, new_row, *g);
                    col += yed_get_glyph_width(*g);
                }
            }
            line_it  = yed_buff_get_line(yank_buff, yank_buff_n_lines);
            for (col = 1; col <= line_it->visual_width;) {
                g = yed_line_col_to_glyph(line_it, col);
                yed_insert_into_line(buff,
                    frame->cursor_line + yank_buff_n_lines - 1,
                    col,
                    *g);
                col += yed_get_glyph_width(*g);
            }
        }
    }

    yed_end_undo_record(frame, frame->buffer);
}

int yed_inc_find_in_buffer(void) {
    int r, c;

    yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_row, ys->search_save_col);
    if (yed_find_next(ys->active_frame->cursor_line, ys->active_frame->cursor_col, &r, &c)) {
        yed_set_cursor_far_within_frame(ys->active_frame, r, c);
        return 1;
    }
    return 0;
}

void yed_find_in_buffer_take_key(int key) {
    int         found;
    char      **mru_search;
    char       *cpy;
    yed_attrs   cmd_attr;
    yed_attrs   attn_attr;
    yed_attrs   err_attr;
    char        attr_buff[128];

    switch (key) {
        case ENTER:
            found = yed_inc_find_in_buffer();
            if (ys->save_search) {
                free(ys->save_search);
            }
            ys->save_search    = strdup(ys->current_search);
            ys->current_search = ys->save_search;

            ys->interactive_command = NULL;
            yed_clear_cmd_buff();

            if (!found) {
                cmd_attr    = yed_active_style_get_command_line();
                attn_attr   = yed_active_style_get_attention();
                err_attr    = cmd_attr;
                err_attr.fg = attn_attr.fg;
                yed_get_attr_str(err_attr, attr_buff);

                yed_append_text_to_cmd_buff(ys->cmd_prompt);
                yed_append_non_text_to_cmd_buff(attr_buff);
                yed_append_text_to_cmd_buff("[!] '");
                yed_append_text_to_cmd_buff(ys->save_search);
                yed_append_text_to_cmd_buff("' not found");
            }

            mru_search = array_last(ys->search_hist);

            if (strlen(ys->save_search)) {
                if (!mru_search || strcmp(*mru_search, ys->save_search) != 0) {
                    cpy = strdup(ys->save_search);
                    array_push(ys->search_hist, cpy);
                }
            }

            break;
        case CTRL_C:
        case ESC:
            ys->interactive_command = NULL;
            ys->current_search      = NULL;
            yed_clear_cmd_buff();
            yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_row, ys->search_save_col);
            break;
        default:
            yed_cmd_line_readline_take_key(ys->search_readline, key);

            array_zero_term(ys->cmd_buff);
            ys->current_search = array_data(ys->cmd_buff);

            yed_inc_find_in_buffer();
    }
}

void yed_start_find_in_buffer(void) {
    ys->interactive_command = "find-in-buffer";
    ys->cmd_prompt          = "(find-in-buffer) ";
    ys->search_save_row     = ys->active_frame->cursor_line;
    ys->search_save_col     = ys->active_frame->cursor_col;

    yed_clear_cmd_buff();
    yed_cmd_line_readline_reset(ys->search_readline, &ys->search_hist);

    ys->current_search = array_data(ys->cmd_buff);
}

void yed_default_command_find_in_buffer(int n_args, char **args) {
    yed_frame *frame;
    int        key;
    int        found;
    yed_attrs  cmd_attr;
    yed_attrs  attn_attr;
    yed_attrs  err_attr;
    char       attr_buff[128];

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (!ys->interactive_command) {
        if (n_args > 0) {
            if (n_args > 1) {
                yed_cerr("expected one search string (use quotes to search for a pattern with whitespace)");
                return;
            }

            ys->current_search  = strdup(args[0]);
            ys->save_search     = ys->current_search;
            ys->search_save_row = ys->active_frame->cursor_line;
            ys->search_save_col = ys->active_frame->cursor_col;

            found = yed_inc_find_in_buffer();

            if (!found) {
                cmd_attr    = yed_active_style_get_command_line();
                attn_attr   = yed_active_style_get_attention();
                err_attr    = cmd_attr;
                err_attr.fg = attn_attr.fg;
                yed_get_attr_str(err_attr, attr_buff);

                yed_append_text_to_cmd_buff(ys->cmd_prompt);
                yed_append_non_text_to_cmd_buff(attr_buff);
                yed_append_text_to_cmd_buff("[!] '");
                yed_append_text_to_cmd_buff(ys->current_search);
                yed_append_text_to_cmd_buff("' not found");

                ys->current_search = NULL;
            }

        } else {
            yed_start_find_in_buffer();
        }
    } else {
        sscanf(args[0], "%d", &key);
        yed_find_in_buffer_take_key(key);
        frame->dirty = 1;
    }
}

void yed_default_command_find_next_in_buffer(int n_args, char **args) {
    yed_frame *frame;
    yed_attrs  cmd_attr;
    yed_attrs  attn_attr;
    yed_attrs  err_attr;
    char       attr_buff[128];


    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (!ys->save_search) {
        yed_cerr("no previous search");
        return;
    }

    ys->current_search  = ys->save_search;
    ys->search_save_row = ys->active_frame->cursor_line;
    ys->search_save_col = ys->active_frame->cursor_col;

    if (!yed_inc_find_in_buffer()) {
        cmd_attr    = yed_active_style_get_command_line();
        attn_attr   = yed_active_style_get_attention();
        err_attr    = cmd_attr;
        err_attr.fg = attn_attr.fg;
        yed_get_attr_str(err_attr, attr_buff);

        yed_append_non_text_to_cmd_buff(attr_buff);
        yed_append_text_to_cmd_buff("[!] '");
        yed_append_text_to_cmd_buff(ys->save_search);
        yed_append_text_to_cmd_buff("' not found");
    } else {
        yed_append_text_to_cmd_buff(ys->current_search);
    }
}

int yed_inc_find_prev_in_buffer(void) {
    int r, c;

    yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_row, ys->search_save_col);
    if (yed_find_prev(ys->active_frame->cursor_line, ys->active_frame->cursor_col, &r, &c)) {
        yed_set_cursor_far_within_frame(ys->active_frame, r, c);
        return 1;
    }
    return 0;
}

void yed_default_command_find_prev_in_buffer(int n_args, char **args) {
    yed_frame *frame;
    yed_attrs  cmd_attr;
    yed_attrs  attn_attr;
    yed_attrs  err_attr;
    char       attr_buff[128];

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (!ys->save_search) {
        yed_cerr("no previous search");
        return;
    }

    ys->current_search  = ys->save_search;
    ys->search_save_row = ys->active_frame->cursor_line;
    ys->search_save_col = ys->active_frame->cursor_col;

    if (!yed_inc_find_prev_in_buffer()) {
        cmd_attr    = yed_active_style_get_command_line();
        attn_attr   = yed_active_style_get_attention();
        err_attr    = cmd_attr;
        err_attr.fg = attn_attr.fg;
        yed_get_attr_str(err_attr, attr_buff);

        yed_append_non_text_to_cmd_buff(attr_buff);
        yed_append_text_to_cmd_buff("[!] '");
        yed_append_text_to_cmd_buff(ys->current_search);
        yed_append_text_to_cmd_buff("' not found");
    } else {
        yed_append_text_to_cmd_buff(ys->current_search);
    }
}

void yed_replace_current_search_update_line(int idx, int row) {
    char       *replacement;
    int         i, len, it,
               *mark;
    yed_buffer *buff;
    yed_line   *line,
               *working_line;
    array_t    *markers;
    yed_glyph   g;

    buff = ys->active_frame->buffer;

    working_line = *(yed_line**)array_item(ys->replace_working_lines, idx);
    line         = yed_copy_line(working_line);
    yed_buff_set_line(buff, row, line);
    line = NULL;

    replacement = array_data(ys->cmd_buff);
    len         = strlen(replacement);
    markers     = array_item(ys->replace_markers, idx);

    it = 0;
    array_traverse(*markers, mark) {
        for (i = len - 1; i >= 0; i -= 1) {
            g = G(replacement[i]);
            yed_insert_into_line(buff, row, *mark + (it * len), g);
        }
        it += 1;
    }
}

void yed_replace_current_search_update(void) {
    int row, r1, c1, r2, c2, i;

    array_zero_term(ys->cmd_buff);

    if (ys->active_frame->buffer->has_selection) {
        yed_range_sorted_points(&ys->active_frame->buffer->selection,
                                &r1, &c1, &r2, &c2);
        ys->active_frame->dirty = 1;
    } else {
        r1 = r2 = ys->active_frame->cursor_line;
    }

    i = 0;
    for (row = r1; row <= r2; row += 1) {
        yed_replace_current_search_update_line(i, row);
        i += 1;
    }
}

void replace_abort(void) {
    int       row, r1, c1, r2, c2, i;
    yed_line *save_line;

    if (ys->active_frame->buffer->has_selection) {
        yed_range_sorted_points(&ys->active_frame->buffer->selection,
                                &r1, &c1, &r2, &c2);
    } else {
        r1 = r2 = ys->active_frame->cursor_line;
    }

    i = 0;
    for (row = r1; row <= r2; row += 1) {
        save_line = *(yed_line**)array_item(ys->replace_save_lines, i);
        yed_buff_set_line(ys->active_frame->buffer, row, save_line);
        i += 1;
    }
}

void replace_free(void) {
    yed_line **line_it;
    array_t   *markers_it;

    array_traverse(ys->replace_save_lines, line_it) {
        yed_free_line(*line_it);
    }
    array_clear(ys->replace_save_lines);
    array_traverse(ys->replace_working_lines, line_it) {
        yed_free_line(*line_it);
    }
    array_clear(ys->replace_working_lines);
    array_traverse(ys->replace_markers, markers_it) {
        array_free(*markers_it);
    }
    array_clear(ys->replace_markers);
}

void yed_replace_current_search_take_key(int key) {
    char *cpy;

    switch (key) {
        case ESC:
        case CTRL_C:
            ys->interactive_command = NULL;
            yed_clear_cmd_buff();
            replace_abort();
            replace_free();
            yed_cancel_undo_record(ys->active_frame, ys->active_frame->buffer);
            break;
        case ENTER:
            yed_replace_current_search_update();

            replace_free();
            yed_end_undo_record(ys->active_frame, ys->active_frame->buffer);

            ys->interactive_command = NULL;
            cpy = strdup(array_data(ys->cmd_buff));

            YEXE("select-off");

            yed_clear_cmd_buff();

            yed_append_text_to_cmd_buff(ys->cmd_prompt);
            yed_append_text_to_cmd_buff("replaced ");
            yed_append_int_to_cmd_buff(ys->replace_count);
            yed_append_text_to_cmd_buff(" occurances of '");
            yed_append_text_to_cmd_buff(ys->current_search);
            yed_append_text_to_cmd_buff("' with '");
            yed_append_text_to_cmd_buff(cpy);
            yed_append_text_to_cmd_buff("'");

            free(cpy);
            break;
        default:
            yed_cmd_line_readline_take_key(NULL, key);
            yed_replace_current_search_update();
            ys->active_frame->dirty = 1;
            break;
    }
}

void replace_add_line(yed_buffer *buff, int row, int len) {
    yed_line  *line,
              *save_line,
              *working_line;
    int        idx, col, j;
    array_t    markers;
    yed_glyph *git;

    line      = yed_buff_get_line(buff, row);
    markers   = array_make(int);
    save_line = yed_copy_line(line);

    array_push(ys->replace_save_lines, save_line);

    while (1) {
        yed_line_glyph_traverse(*line, git) {
            if (((void*)git) + len - 1 > array_last(line->chars)) { break; }
            if (strncmp(&git->c, ys->current_search, len) == 0) {
                idx = ((void*)git - line->chars.data);
                col = yed_line_idx_to_col(line, idx);
                array_push(markers, col);
                ys->replace_count += 1;


                for (j = 0; j < len; j += 1) {
                    yed_delete_from_line(buff, row, col);
                }

                goto cont;
            }
        }
        break;
    cont:
        continue;
    }

    array_push(ys->replace_markers, markers);
    working_line = yed_copy_line(line);
    array_push(ys->replace_working_lines, working_line);
}

void yed_start_replace_current_search(void) {
    yed_buffer *buff;
    int         len;
    int         row, r1, c1, r2, c2;

    ys->interactive_command  = "replace-current-search";
    ys->cmd_prompt           = "(replace-current-search) ";
    ys->search_save_row      = ys->active_frame->cursor_line;
    ys->search_save_col      = ys->active_frame->cursor_col;
    ys->current_search       = ys->save_search;
    ys->replace_count        = 0;

    buff = ys->active_frame->buffer;
    len  = strlen(ys->current_search);

    yed_start_undo_record(ys->active_frame, buff);

    yed_set_cursor_within_frame(ys->active_frame, ys->active_frame->cursor_line, 1);

    if (buff->has_selection) {
        yed_range_sorted_points(&ys->active_frame->buffer->selection,
                                &r1, &c1, &r2, &c2);
    } else {
        r1 = r2 = ys->active_frame->cursor_line;
    }

    for (row = r1; row <= r2; row += 1) {
        replace_add_line(buff, row, len);
    }

    yed_clear_cmd_buff();

    yed_replace_current_search_update();
}

void yed_default_command_replace_current_search(int n_args, char **args) {
    yed_frame *frame;
    int        key;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    if (!ys->save_search || !strlen(ys->save_search)) {
        yed_cerr("no previous search");
        return;
    }

    if (!ys->interactive_command) {
        yed_start_replace_current_search();
    } else {
        sscanf(args[0], "%d", &key);
        yed_replace_current_search_take_key(key);
        frame->dirty = 1;
    }
}

void yed_default_command_style(int n_args, char **args) {
    if (n_args == 0) {
        if (ys->active_style) {
            yed_cprint("'%s'", ys->active_style->_name);
        } else {
            yed_cerr("no style activated");
        }
    } else if (n_args == 1) {
        if (yed_activate_style(args[0])) {
            yed_cprint("activated style '%s'", args[0]);
        } else {
            yed_cerr("no such style '%s'", args[0]);
        }
    } else {
        yed_cerr("expected 0 or 1 arguments, but got %d", n_args);
        return;
    }
}

void yed_default_command_style_off(int n_args, char **args) {
    yed_event event;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("");

    if (ys->active_style) {
        free(ys->active_style);
        ys->active_style = NULL;
    }

    memset(&event, 0, sizeof(event));
    event.kind = EVENT_STYLE_CHANGE;
    yed_trigger_event(&event);

    ys->redraw = 1;
}

void yed_default_command_styles_list(int n_args, char **args) {
    tree_it(yed_style_name_t, yed_style_ptr_t) it;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("");
    tree_traverse(ys->styles, it) {
        yed_cprint("\n%s", tree_it_key(it));
    }
}

void yed_default_command_undo(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    if (buffer->flags & BUFF_SPECIAL) {
        yed_cerr("can't undo in a special buffer");
        return;
    }

    if (!yed_undo(frame, buffer)) {
        yed_cerr("nothing to undo");
    }
}

void yed_default_command_redo(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    if (buffer->flags & BUFF_SPECIAL) {
        yed_cerr("can't redo in a special buffer");
        return;
    }

    if (!yed_redo(frame, buffer)) {
        yed_cerr("nothing to redo");
    }
}

void yed_default_command_bind(int n_args, char **args) {
    char            *cmd, **cmd_args;
    int              n_keys, keys[MAX_SEQ_LEN], n_cmd_args;
    int              seq_key;
    yed_key_binding  binding;

    if (n_args == 0) {
        yed_cerr("missing 'keys' as first argument");
        return;
    }

    if (n_args < 2) {
        yed_cerr("missing 'command', 'command_args'... as second and up arguments");
        return;
    }

    n_keys = yed_string_to_keys(args[0], keys);
    if (n_keys == -1) {
        yed_cerr("invalid string of keys '%s'", args[0]);
        return;
    }
    if (n_keys == -2) {
        yed_cerr("too many keys to be a sequence in '%s'", args[0]);
        return;
    }

    cmd        = args[1];
    n_cmd_args = n_args - 2;
    cmd_args   = args + 2;

    if (n_keys > 1) {
        seq_key = yed_add_key_sequence(n_keys, keys);
    } else {
        seq_key = keys[0];
    }

    binding.key    = seq_key;
    binding.cmd    = cmd;
    binding.n_args = n_cmd_args;
    binding.args   = cmd_args;
    yed_bind_key(binding);
}

void yed_default_command_unbind(int n_args, char **args) {
    int  n_keys, keys[MAX_SEQ_LEN];
    int  seq_key;

    if (n_args != 1) {
        yed_cerr("expected 'keys' as first and only argument");
        return;
    }

    n_keys = yed_string_to_keys(args[0], keys);
    if (n_keys == -1) {
        yed_cerr("invalid string of keys '%s'", args[0]);
        return;
    }
    if (n_keys == -2) {
        yed_cerr("too many keys to be a sequence in '%s'", args[0]);
        return;
    }

    if (n_keys > 1) {
        seq_key = yed_get_key_sequence(n_keys, keys);
    } else {
        seq_key = keys[0];
    }

    yed_unbind_key(seq_key);

    if (n_keys > 1) {
        yed_delete_key_sequence(seq_key);
    }
}

void yed_default_command_multi(int n_args, char **args) {
    int     i;
    array_t split;

    for (i = 0; i < n_args; i += 1) {
        split = sh_split(args[i]);
        if (array_len(split)) {
            yed_execute_command_from_split(split);
        }
        free_string_array(split);
    }
}

void yed_default_command_suspend(int n_args, char **args) {
    yed_cprint("suspending yed..\n");
    ys->redraw = ys->redraw_cls = 1;
    kill(0, SIGTSTP);
    yed_cprint_clear();
    yed_cprint("continuing from suspend");
}


void yed_default_command_scomps_list(int n_args, char **args) {
    char **it;

    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("");
    array_traverse(ys->scomp_strings, it) {
        yed_cprint("\n%s", *it);
    }
}

void yed_default_command_version(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    yed_cprint("%d", YED_VERSION);
}

void yed_default_command_show_bindings(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*bindings");
    YEXE("buffer", "*bindings");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
}

void yed_default_command_show_vars(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*vars");
    YEXE("buffer", "*vars");
    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
}

void yed_default_command_special_buffer_prepare_focus(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (ys->active_frame
    &&  ys->active_frame->buffer
    &&  strcmp(ys->active_frame->buffer->name, args[0]) == 0) {
        /* It's already focused. Don't do anything. */
        return;
    }

    frame = yed_add_new_frame(0.15, 0.15, 0.7, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void yed_default_command_special_buffer_prepare_jump_focus(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (ys->active_frame != NULL) {
        yed_delete_frame(ys->active_frame);
    }

    /*
    ** This will ensure that the position of the cursor in the old buffer
    ** can be restored.
    */
    if (ys->active_frame != NULL) {
        yed_frame_set_buff(ys->active_frame, NULL);
    }

    if (array_len(ys->frames) == 0) {
        YEXE("frame-new");
    }
}

void yed_default_command_special_buffer_prepare_unfocus(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    if (ys->active_frame != NULL) {
        yed_delete_frame(ys->active_frame);
    }
}

void yed_default_command_log(int n_args, char **args) {
    if (n_args != 0) {
        yed_cerr("expected 0 arguments, but got %d", n_args);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*log");
    YEXE("buffer",                       "*log");
}

void yed_default_command_nop(int n_args, char **args) {}

void yed_default_command_cursor_style(int n_args, char **args) {
    char *style;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    style = args[0];

    if (     strcmp(style, "default")            == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_DEFAULT);            }
    else if (strcmp(style, "blinking-block")     == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_BLINKING_BLOCK);     }
    else if (strcmp(style, "steady-block")       == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_STEADY_BLOCK);       }
    else if (strcmp(style, "blinking-underline") == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_BLINKING_UNDERLINE); }
    else if (strcmp(style, "steady-underline")   == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_STEADY_UNDERLINE);   }
    else if (strcmp(style, "blinking-bar")       == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_BLINKING_BAR);       }
    else if (strcmp(style, "steady-bar")         == 0) { yed_set_cursor_style(TERM_CURSOR_STYLE_STEADY_BAR);         }
    else {
        yed_cerr("invalid cursor style '%s'. Options are default, blinking-block, steady-block, blinking-underline, steady-underline, blinking-bar, and steady-bar.", style);
    }
}

void yed_default_command_feed_keys(int n_args, char **args) {
    int i;
    int n;
    int keys[MAX_SEQ_LEN];

    for (i = 0; i < n_args; i += 1) {
        n = yed_string_to_keys(args[i], keys);
        if (n <= 0) { continue; }

        yed_feed_keys(n, keys);
    }
}

void yed_default_command_alias(int n_args, char **args) {
    yed_command cmd;

    if (n_args != 2) {
        yed_cerr("expected 2 arguments, but got %d", n_args);
        return;
    }

    cmd = yed_get_command(args[1]);

    if (cmd == NULL) {
        yed_cerr("unknown command '%s'", args[1]);
        return;
    }

    yed_set_command(args[0], cmd);
}

void yed_default_command_unalias(int n_args, char **args) {
    if (n_args != 1) {
        yed_cerr("expected 1 argument, but got %d", n_args);
        return;
    }

    yed_unset_command(args[0]);
}

void yed_default_command_repeat(int n_args, char **args) {
    int i;

    if (n_args < 2) {
        yed_cerr("expected 2 or more arguments, but got %d", n_args);
        return;
    }

    if (!sscanf(args[0], "%d", &i)) {
        yed_cerr("cound't parse int from argument '%s'", args[0]);
    }

    for (; i > 0; i -= 1) {
        yed_execute_command(args[1], n_args - 2, args + 2);
    }
}


int yed_execute_command_from_split(array_t split) {
    char  *cmd_name,
         **args;

    if (array_len(split) == 0) {
        return 1;
    }

    cmd_name = *(char**)array_item(split, 0);
    args     = ((char**)array_data(split)) + 1;

    return yed_execute_command(cmd_name, array_len(split) - 1, args);
}

int yed_execute_command(char *name, int n_args, char **args) {
    tree_it(yed_command_name_t, yed_command)   it;
    yed_command                                cmd;
    yed_attrs                                  cmd_attr, attn_attr;
    char                                       attr_buff[128];
    yed_event                                  evt;
    char                                       name_cpy[256];

    cprinted_len   = 0;
    if (!ys->interactive_command) {
        yed_clear_cmd_buff();
    }

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        cmd_attr    = yed_active_style_get_command_line();
        attn_attr   = yed_active_style_get_attention();
        cmd_attr.fg = attn_attr.fg;
        yed_get_attr_str(cmd_attr, attr_buff);
        yed_append_non_text_to_cmd_buff(attr_buff);
        yed_append_text_to_cmd_buff("unknown command '");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff("'");
        yed_append_non_text_to_cmd_buff(TERM_RESET);
        yed_append_non_text_to_cmd_buff(TERM_CURSOR_HIDE);

LOG_FN_ENTER();
        yed_log("[!] unknown command '%s'", name);
LOG_EXIT();

        return 1;
    }

    cmd = tree_it_val(it);

    if (!ys->interactive_command) {
        ys->cmd_prompt = YED_CMD_PROMPT;
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff(") ");
        cprinted_len = 3 + strlen(name);
    }

    memset(&evt, 0, sizeof(evt));
    evt.kind     = EVENT_CMD_PRE_RUN;
    evt.cmd_name = name;
    evt.n_args   = n_args;
    evt.args     = (const char * const *)args;
    yed_trigger_event(&evt);

    if (!evt.cancel) {
        name_cpy[0] = 0;
        strcat(name_cpy, name);
        LOG_CMD_ENTER(name_cpy);
        cmd(n_args, args);
        LOG_EXIT();

        evt.kind = EVENT_CMD_POST_RUN;
        yed_trigger_event(&evt);
    }


    yed_draw_command_line();

    return 0;
}
