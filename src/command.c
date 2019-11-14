#include "internal.h"

void yed_init_commands(void) {
    ys->commands         = tree_make_c(yed_command_name_t, yed_command, strcmp);
    ys->default_commands = tree_make_c(yed_command_name_t, yed_command, strcmp);
    yed_set_default_commands();
    ys->cmd_buff = array_make(char);
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

void yed_set_default_commands(void) {
#define SET_DEFAULT_COMMAND(name1, name2)                         \
do {                                                              \
    yed_set_command(name1, &yed_default_command_##name2);         \
    yed_set_default_command(name1, &yed_default_command_##name2); \
} while (0)

    SET_DEFAULT_COMMAND("command-prompt",         command_prompt);
    SET_DEFAULT_COMMAND("quit",                   quit);
    SET_DEFAULT_COMMAND("reload",                 reload);
    SET_DEFAULT_COMMAND("redraw",                 redraw);
    SET_DEFAULT_COMMAND("set",                    set);
    SET_DEFAULT_COMMAND("get",                    get);
    SET_DEFAULT_COMMAND("unset",                  unset);
    SET_DEFAULT_COMMAND("sh",                     sh);
    SET_DEFAULT_COMMAND("echo",                   echo);
    SET_DEFAULT_COMMAND("cursor-down",            cursor_down);
    SET_DEFAULT_COMMAND("cursor-up",              cursor_up);
    SET_DEFAULT_COMMAND("cursor-left",            cursor_left);
    SET_DEFAULT_COMMAND("cursor-right",           cursor_right);
    SET_DEFAULT_COMMAND("cursor-line-begin",      cursor_line_begin);
    SET_DEFAULT_COMMAND("cursor-line-end",        cursor_line_end);
    SET_DEFAULT_COMMAND("cursor-prev-word",       cursor_prev_word);
    SET_DEFAULT_COMMAND("cursor-next-word",       cursor_next_word);
    SET_DEFAULT_COMMAND("cursor-next-word",       cursor_next_word);
    SET_DEFAULT_COMMAND("cursor-prev-paragraph",  cursor_prev_paragraph);
    SET_DEFAULT_COMMAND("cursor-next-paragraph",  cursor_next_paragraph);
    SET_DEFAULT_COMMAND("cursor-page-up",         cursor_page_up);
    SET_DEFAULT_COMMAND("cursor-page-down",       cursor_page_down);
    SET_DEFAULT_COMMAND("cursor-buffer-begin",    cursor_buffer_begin);
    SET_DEFAULT_COMMAND("cursor-buffer-end",      cursor_buffer_end);
    SET_DEFAULT_COMMAND("cursor-line",            cursor_line);
    SET_DEFAULT_COMMAND("word-under-cursor",      word_under_cursor);
    SET_DEFAULT_COMMAND("buffer",                 buffer);
    SET_DEFAULT_COMMAND("buffer-delete",          buffer_delete);
    SET_DEFAULT_COMMAND("buffer-next",            buffer_next);
    SET_DEFAULT_COMMAND("buffer-name",            buffer_name);
    SET_DEFAULT_COMMAND("frame-new",              frame_new);
    SET_DEFAULT_COMMAND("frame-delete",           frame_delete);
    SET_DEFAULT_COMMAND("frame-vsplit",           frame_vsplit);
    SET_DEFAULT_COMMAND("frame-hsplit",           frame_hsplit);
    SET_DEFAULT_COMMAND("frame-next",             frame_next);
    SET_DEFAULT_COMMAND("insert",                 insert);
    SET_DEFAULT_COMMAND("delete-back",            delete_back);
    SET_DEFAULT_COMMAND("delete-forward",         delete_forward);
    SET_DEFAULT_COMMAND("delete-line",            delete_line);
    SET_DEFAULT_COMMAND("write-buffer",           write_buffer);
    SET_DEFAULT_COMMAND("plugin-load",            plugin_load);
    SET_DEFAULT_COMMAND("plugin-unload",          plugin_unload);
    SET_DEFAULT_COMMAND("plugins-list",           plugins_list);
    SET_DEFAULT_COMMAND("plugins-list-dirs",      plugins_list_dirs);
    SET_DEFAULT_COMMAND("plugins-add-dir",        plugins_add_dir);
    SET_DEFAULT_COMMAND("select",                 select);
    SET_DEFAULT_COMMAND("select-lines",           select_lines);
    SET_DEFAULT_COMMAND("select-off",             select_off);
    SET_DEFAULT_COMMAND("yank-selection",         yank_selection);
    SET_DEFAULT_COMMAND("paste-yank-buffer",      paste_yank_buffer);
    SET_DEFAULT_COMMAND("find-in-buffer",         find_in_buffer);
    SET_DEFAULT_COMMAND("find-next-in-buffer",    find_next_in_buffer);
    SET_DEFAULT_COMMAND("find-prev-in-buffer",    find_prev_in_buffer);
    SET_DEFAULT_COMMAND("replace-current-search", replace_current_search);
    SET_DEFAULT_COMMAND("style",                  style);
    SET_DEFAULT_COMMAND("style-off",              style_off);
}

void yed_clear_cmd_buff(void) {
    array_clear(ys->cmd_buff);
    array_zero_term(ys->cmd_buff);
    ys->cmd_cursor_x = 1;

    if (ys->cmd_prompt) {
        ys->cmd_cursor_x += strlen(ys->cmd_prompt);
    }
}

void yed_append_to_cmd_buff(char *s) {
    int i, len;

    len = strlen(s);
    for (i = 0; i < len; i += 1) {
        array_push(ys->cmd_buff, s[i]);
    }
}

void yed_append_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
    ys->cmd_cursor_x += strlen(s);
}

void yed_append_non_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
}

void yed_append_int_to_cmd_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    yed_append_text_to_cmd_buff(s);
}

void yed_cmd_buff_push(char c) {
    array_push(ys->cmd_buff, c);
    ys->cmd_cursor_x += 1;
}

void yed_cmd_buff_pop(void) {
    array_pop(ys->cmd_buff);
    ys->cmd_cursor_x -= 1;
}

void yed_draw_command_line() {
    yed_set_cursor(1, ys->term_rows);
    append_to_output_buff(TERM_CLEAR_LINE);
    if (ys->interactive_command) {
        if (ys->cmd_prompt) {
            append_to_output_buff(ys->cmd_prompt);
        }
    }
    append_n_to_output_buff(array_data(ys->cmd_buff), array_len(ys->cmd_buff));
}

void yed_start_command_prompt(void) {
    ys->interactive_command = "command-prompt";
    ys->cmd_prompt          = ": ";
    yed_set_small_message(NULL);
    yed_clear_cmd_buff();
}

void yed_do_command(void) {
    char *cmd_cpy,
         *cmd_beg,
         *cmd_end,
         *cmd_curs;
    int   cmd_len;
    int   n_split,
          last_was_space;
    char *cmd_split[32];

    array_zero_term(ys->cmd_buff);
    cmd_cpy = malloc(array_len(ys->cmd_buff) + 1);
    memcpy(cmd_cpy, array_data(ys->cmd_buff), array_len(ys->cmd_buff));
    cmd_cpy[array_len(ys->cmd_buff)] = 0;

    yed_clear_cmd_buff();

    cmd_beg = cmd_curs = cmd_cpy;
    cmd_len = strlen(cmd_beg);
    cmd_end = cmd_beg + cmd_len;
    n_split = 0;

    cmd_split[n_split++] = cmd_beg;
    last_was_space       = 0;

    while (cmd_curs != cmd_end) {
        if (*cmd_curs == ' ') {
            last_was_space = 1;
            *cmd_curs      = 0;
        } else {
            if (last_was_space) {
                cmd_split[n_split++] = cmd_curs;
            }
            last_was_space = 0;
        }
        cmd_curs += 1;
    }

    ys->interactive_command = NULL;

    yed_execute_command(cmd_beg, n_split - 1, cmd_split + 1);

    free(cmd_cpy);
}

void yed_command_take_key(int key) {
    tree_it(yed_command_name_t, yed_command)  it;
    char                                     *c;

    if (key == CTRL_F || key == CTRL_C) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
    } else if (key == TAB) {
        array_traverse(ys->cmd_buff, c) {
            if (*c == ' ') {
                return;
            }
        }

        array_zero_term(ys->cmd_buff);
        it = tree_gtr(ys->commands, array_data(ys->cmd_buff));
        if (tree_it_good(it)) {
            yed_clear_cmd_buff();
            yed_append_text_to_cmd_buff((char*)tree_it_key(it));
        }
    } else if (key == ENTER) {
        ys->interactive_command = NULL;
        yed_do_command();
    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }
    }
}

void yed_default_command_command_prompt(int n_args, char **args) {
    int key;

    if (!ys->interactive_command) {
        yed_start_command_prompt();
    } else {
        sscanf(args[0], "%d", &key);
        yed_command_take_key(key);
    }
}

void yed_default_command_quit(int n_args, char **args) {
    ys->status = YED_QUIT;
}

void yed_default_command_reload(int n_args, char **args) {
    ys->status = YED_RELOAD;
    yed_append_text_to_cmd_buff("issued reload");
}

void yed_default_command_redraw(int n_args, char **args) {
    ys->redraw = 1;
}

void yed_default_command_set(int n_args, char **args) {
    if (n_args != 2) {
        yed_append_text_to_cmd_buff("[!] expected two arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_set_var(args[0], args[1]);

    yed_append_text_to_cmd_buff("set '");
    yed_append_text_to_cmd_buff(args[0]);
    yed_append_text_to_cmd_buff("' to '");
    yed_append_text_to_cmd_buff(args[1]);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_get(int n_args, char **args) {
    char *result;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    result = yed_get_var(args[0]);

    if (result) {
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(result);
        yed_append_text_to_cmd_buff("'");
    } else {
        yed_append_text_to_cmd_buff("undefined");
    }
}

void yed_default_command_unset(int n_args, char **args) {
    char *result;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    result = yed_get_var(args[0]);

    if (result) {
        yed_unset_var(args[0]);
        yed_append_text_to_cmd_buff("unset '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("'");
    } else {
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("' was not set");
    }
}

void yed_default_command_sh(int n_args, char **args) {
    char buff[1024];
    const char *lazy_space;
    int i;
    int err;

    buff[0] = 0;

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        strcat(buff, lazy_space);
        strcat(buff, args[i]);
        lazy_space = " ";
    }

    printf(TERM_STD_SCREEN);
    err = system(buff);
    printf(TERM_ALT_SCREEN);

    yed_append_text_to_cmd_buff("'");
    yed_append_text_to_cmd_buff(buff);
    yed_append_text_to_cmd_buff("'");

    if (err != 0) {
        yed_append_text_to_cmd_buff(" exited with non-zero status ");
        yed_append_int_to_cmd_buff(err);
    }

    ys->redraw = 1;
}

void yed_default_command_echo(int n_args, char **args) {
    int         i;
    const char *space;

    space = "";
    for (i = 0; i < n_args; i += 1) {
        yed_append_text_to_cmd_buff((char*)space);
        yed_append_text_to_cmd_buff(args[i]);
        space = " ";
    }
}

void yed_default_command_cursor_down(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

    if (n_args == 0) {
        rows = 1;
    } else {
        rows = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

void yed_default_command_cursor_up(int n_args, char **args) {
    int        rows;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }


    if (n_args == 0) {
        rows = -1;
    } else {
        rows = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

void yed_default_command_cursor_left(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }


    if (n_args == 0) {
        cols = -1;
    } else {
        cols = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

void yed_default_command_cursor_right(int n_args, char **args) {
    int        cols;
    yed_frame *frame;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }


    if (n_args == 0) {
        cols = 1;
    } else {
        cols = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

void yed_default_command_cursor_line_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }


    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);
}

void yed_default_command_cursor_line_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    yed_set_cursor_within_frame(frame, line->visual_width + 1, frame->cursor_line);
}

void yed_default_command_cursor_buffer_begin(int n_args, char **args) {
    yed_frame *frame;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

    yed_set_cursor_far_within_frame(frame, 1, 1);
}

void yed_default_command_cursor_buffer_end(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *last_line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

    last_line = bucket_array_last(frame->buffer->lines);
    yed_set_cursor_far_within_frame(frame, last_line->visual_width + 1, bucket_array_len(frame->buffer->lines));
}

void yed_default_command_cursor_line(int n_args, char **args) {
    yed_frame *frame;
    int        line;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

    sscanf(args[0], "%d", &line);
    yed_set_cursor_far_within_frame(frame, 1, line);
}

void yed_default_command_word_under_cursor(int n_args, char **args) {
    yed_frame *frame;
    char      *word;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    word = yed_word_under_cursor();

    if (word) {
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(word);
        yed_append_text_to_cmd_buff("'");
        free(word);
    } else {
        yed_append_text_to_cmd_buff("[!] cursor is not on a word");
    }
}

void yed_default_command_cursor_prev_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        col, row;
    char       c;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

again:
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    col = frame->cursor_col - 1;

    if (col <= 0)    { goto skip_lines; }

    c = yed_line_col_to_char(line, col);

    if (isspace(c)) {
        while (col > 1) {
            c = yed_line_col_to_char(line, col - 1);

            if (!isspace(c)) {
                break;
            }

            col -= 1;
        }
    }

    if (isalnum(c) || c == '_') {
        while (col > 1) {
            c = yed_line_col_to_char(line, col - 1);

            if (!isalnum(c) && c != '_') {
                break;
            }

            col -= 1;
        }
    } else {
        while (col > 1) {
            c = yed_line_col_to_char(line, col - 1);

            if (isalnum(c) || c == '_' || isspace(c)) {
                break;
            }

            col -= 1;
        }
    }

    if (col == 0 || (col == 1 && (isspace(c)))) {
skip_lines:
        row = frame->cursor_line;
        do {
            row -= 1;
            line = yed_buff_get_line(frame->buffer, row);
        } while (line && !line->visual_width);

        if (line) {
            yed_move_cursor_within_frame(frame, line->visual_width, row - frame->cursor_line);
            goto again;
        }
    } else {
        yed_move_cursor_within_frame(frame, col - frame->cursor_col, 0);
    }
}

void yed_default_command_cursor_next_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    int        col, row;
    char       c;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
    }

again:
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    col  = frame->cursor_col;

    if (col >= line->visual_width) {
        goto skip_lines;
    }

    c = yed_line_col_to_char(line, col);

    if (isalnum(c) || c == '_') {
        while (col <= line->visual_width) {
            col += 1;
            c    = yed_line_col_to_char(line, col);

            if (!isalnum(c) && c != '_') {
                break;
            }
        }
    } else if (!isspace(c)) {
        while (col <= line->visual_width) {
            col += 1;
            c    = yed_line_col_to_char(line, col);

            if (isalnum(c) || c == '_' || isspace(c)) {
                break;
            }
        }
    }

    if (isspace(c)) {
        while (col <= line->visual_width) {
            col += 1;
            c = yed_line_col_to_char(line, col);

            if (!isspace(c)) {
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
            yed_set_cursor_far_within_frame(frame, 1, row);
            line = yed_buff_get_line(frame->buffer, row);
            c    = yed_line_col_to_char(line, 1);
            if (c && isspace(c)) {
                goto again;
            }
        }
    } else {
        yed_move_cursor_within_frame(frame, col - frame->cursor_col, 0);
    }
}

void yed_default_command_cursor_prev_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
    int         i;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
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

    yed_move_cursor_within_frame(frame, 0, i);
}

void yed_default_command_cursor_next_paragraph(int n_args, char **args) {
    yed_frame  *frame;
    yed_line   *line;
    int         i;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
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

    yed_move_cursor_within_frame(frame, 0, i);
}

void yed_default_command_cursor_page_up(int n_args, char **args) {
    yed_frame  *frame;
    int         top_line;
    int         want_top_line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
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
        yed_move_cursor_within_frame(frame, 0, -1);
        top_line = frame->buffer_y_offset + 1;
    }
}

void yed_default_command_cursor_page_down(int n_args, char **args) {
    yed_frame  *frame;
    int         top_line;
    int         want_top_line;
    int         max_top_line;

    if (n_args > 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (ys->current_search) {
        ys->current_search = NULL;
        frame->dirty = 1;
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
        yed_move_cursor_within_frame(frame, 0, 1);
        top_line = frame->buffer_y_offset + 1;
    }
}

void yed_default_command_buffer(int n_args, char **args) {
    yed_buffer                                   *buffer;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_append_text_to_cmd_buff("'");
    yed_append_text_to_cmd_buff(args[0]);
    yed_append_text_to_cmd_buff("'");

    it = tree_lookup(ys->buffers, args[0]);

    if (tree_it_good(it)) {
        buffer = tree_it_val(it);
    } else {
        buffer = yed_create_buffer(args[0]);
        yed_append_text_to_cmd_buff(" (new buffer)");
        if (!yed_fill_buff_from_file(buffer, args[0])) {
            buffer->path    = strdup(args[0]);
            buffer->file.ft = yed_get_ft(args[0]);
            buffer->kind    = BUFF_KIND_FILE;
            yed_append_text_to_cmd_buff(" (new file)");
        }
    }

    if (ys->active_frame) {
        yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
        ys->active_frame->buffer = buffer;
        ys->active_frame->dirty  = 1;
    }
}

void yed_default_command_buffer_delete(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_frame                                    *frame;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_append_text_to_cmd_buff("[!] no active frame ");
            return;
        }

        frame = ys->active_frame;

        if (!frame->buffer) {
            yed_append_text_to_cmd_buff("[!] active frame has no buffer");
            return;
        }

        buffer = frame->buffer;
    } else if (n_args == 1) {
        it = tree_lookup(ys->buffers, args[0]);
        if (tree_it_good(it)) {
            buffer = tree_it_val(it);
        } else {
            yed_append_text_to_cmd_buff("[!] no such buffer '");
            yed_append_text_to_cmd_buff(args[0]);
            yed_append_text_to_cmd_buff("'");
            return;
        }
    } else {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_free_buffer(buffer);
}

void yed_default_command_buffer_next(int n_args, char **args) {
    yed_buffer                                   *buffer;
    yed_frame                                    *frame;
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t)  it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }
    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        if (!tree_len(ys->buffers)) {
            yed_append_text_to_cmd_buff("[!] no buffers");
            return;
        }
        it = tree_begin(ys->buffers);
    } else {
        buffer = frame->buffer;

        it = tree_lookup(ys->buffers, buffer->name);
        tree_it_next(it);

        if (!tree_it_good(it)) {
            it = tree_begin(ys->buffers);
        }
    }

    buffer = tree_it_val(it);

    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    ys->active_frame->buffer = buffer;
    ys->active_frame->dirty  = 1;

    yed_append_text_to_cmd_buff("'");
    yed_append_text_to_cmd_buff(buffer->name);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_buffer_name(int n_args, char **args) {
    yed_buffer *buffer;
    yed_frame  *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }
    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buffer = frame->buffer;

    yed_append_text_to_cmd_buff("'");
    yed_append_text_to_cmd_buff(buffer->name);
    yed_append_text_to_cmd_buff("'");
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
        yed_append_text_to_cmd_buff("[!] expected zero or four arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void yed_default_command_frame_delete(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }
    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    yed_delete_frame(frame);
}

void yed_default_command_frame_vsplit(int n_args, char **args) {
    yed_frame *new_frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    new_frame = yed_vsplit_frame(ys->active_frame);

    yed_activate_frame(new_frame);
}

void yed_default_command_frame_hsplit(int n_args, char **args) {
    yed_frame *new_frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    new_frame = yed_hsplit_frame(ys->active_frame);

    yed_activate_frame(new_frame);
}

void yed_default_command_frame_next(int n_args, char **args) {
    yed_frame *cur_frame, *frame, **frame_it;
    int        take_next;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    if (array_len(ys->frames) == 1) {
        yed_append_text_to_cmd_buff("[!] no next frame");
        return;
    }

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

void yed_default_command_insert(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *new_line;
    yed_event  event;
    int        key,
               col, idx,
               i, len,
               tabw;
    char      *c,
              *tabw_str;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    sscanf(args[0], "%d", &key);

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_append_text_to_cmd_buff("[!] buffer is read-only");
        return;
    }

    col = frame->cursor_col;

    event.kind  = EVENT_BUFFER_PRE_INSERT;
    event.frame = frame;
    event.row   = frame->cursor_line;
    event.col   = col;
    event.key   = key;

    yed_trigger_event(&event);

    event.kind = EVENT_BUFFER_PRE_MOD;
    yed_trigger_event(&event);

    if (key == ENTER) {
        /*
         * Must get 'line' _after_ we've added 'new_line' because
         * the array might have expanded and so the reference to
         * 'line' could be invalid if we had acquired it before.
         */
        new_line = yed_buff_insert_line(frame->buffer, frame->cursor_line + 1);
        line     = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == 1) {
            new_line->chars        = line->chars;
            line->chars            = array_make(char);
            new_line->visual_width = line->visual_width;
            line->visual_width     = 0;
        } else {
            len = 0;
            idx = yed_line_col_to_idx(line, col);
            array_traverse_from(line->chars, c, idx) {
                yed_line_append_char(new_line, *c);
                len += 1;
            }
            for (; len > 0; len -= 1)    { yed_line_pop_char(line); }
        }

        yed_set_cursor_within_frame(frame, 1, frame->cursor_line + 1);
    } else {
        if (key == TAB) {
            tabw_str = yed_get_var("tab-width");
            if (tabw_str) {
                sscanf(tabw_str, "%d", &tabw);
            } else {
                tabw = 4;
            }
            for (i = 0; i < tabw; i += 1) {
                yed_insert_into_line(frame->buffer, frame->cursor_line, col + i, ' ');
            }
            yed_move_cursor_within_frame(frame, tabw, 0);
        } else {
            yed_insert_into_line(frame->buffer, frame->cursor_line, col, key);
            yed_move_cursor_within_frame(frame, 1, 0);
        }
    }

    event.row  = frame->cursor_line;
    event.col  = frame->cursor_col;
    event.kind = EVENT_BUFFER_POST_MOD;
    yed_trigger_event(&event);

    event.kind = EVENT_BUFFER_POST_INSERT;
    yed_trigger_event(&event);
}

void yed_default_command_delete_back(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *previous_line;
    yed_event  event;
    int        r1, c1, r2, c2,
               col,
               buff_n_lines,
               prev_line_len, old_line_nr;
    char      *c;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_append_text_to_cmd_buff("[!] buffer is read-only");
        return;
    }

    event.kind  = EVENT_BUFFER_PRE_MOD;
    event.frame = frame;
    event.row   = frame->cursor_line;

    yed_trigger_event(&event);

    event.kind  = EVENT_BUFFER_PRE_DELETE_BACK;
    yed_trigger_event(&event);

    if (frame->buffer->has_selection) {
        r1 = c1 = r2 = c2 = 0;
        yed_range_sorted_points(&frame->buffer->selection, &r1, &c1, &r2, &c2);
        frame->buffer->selection.locked = 1;
        if (frame->buffer->selection.kind == RANGE_LINE) {
            r1 = MIN(r1, bucket_array_len(frame->buffer->lines) - (r2 - r1) - 1);
            yed_set_cursor_far_within_frame(frame, 1, r1);
        } else {
            yed_set_cursor_far_within_frame(frame, c1, r1);
        }
        yed_buff_delete_selection(frame->buffer);
    } else {
        col  = frame->cursor_col;
        line = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == 1) {
            if (frame->cursor_line > 1) {
                old_line_nr   = frame->cursor_line;
                previous_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
                prev_line_len = previous_line->visual_width;

                if (prev_line_len == 0) {
                    previous_line->chars        = line->chars;
                    line->chars                 = array_make(char);
                    previous_line->visual_width = line->visual_width;
                    line->visual_width          = 0;
                } else {
                    array_traverse(line->chars, c) {
                        yed_line_append_char(previous_line, *c);
                    }
                }

                /*
                 * Move to the previous line.
                 */
                yed_set_cursor_within_frame(frame, prev_line_len + 1, frame->cursor_line - 1);

                /*
                 * Kinda hacky, but this will help us pull the buffer
                 * up if there is a buffer_y_offset and there's content
                 * to pull up.
                 */
                buff_n_lines = bucket_array_len(frame->buffer->lines);
                if (frame->buffer_y_offset >= buff_n_lines - frame->height) {
                    yed_frame_reset_cursor(frame);
                }

                /*
                 * Delete the old line.
                 */
                yed_buff_delete_line(frame->buffer, old_line_nr);
            }
        } else {
            yed_delete_from_line(frame->buffer, frame->cursor_line, col - 1);
            yed_move_cursor_within_frame(frame, -1, 0);
        }
    }

    event.kind  = EVENT_BUFFER_POST_DELETE_BACK;
    yed_trigger_event(&event);

    event.kind = EVENT_BUFFER_POST_MOD;
    yed_trigger_event(&event);
}

void yed_default_command_delete_forward(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    yed_event  event;
    int        row, col;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_append_text_to_cmd_buff("[!] buffer is read-only");
        return;
    }

    row = frame->cursor_line;
    col = frame->cursor_col;

    line = yed_buff_get_line(frame->buffer, row);

    if (line->visual_width == 0) {
        return;
    }

    event.kind  = EVENT_BUFFER_PRE_MOD;
    event.frame = frame;
    event.row   = row;
    event.col   = col;

    yed_trigger_event(&event);

    if (col == line->visual_width + 1) {
        yed_move_cursor_within_frame(frame, -1, 0);
        col = frame->cursor_col;
    }
    yed_delete_from_line(frame->buffer, row, col);

    event.kind = EVENT_BUFFER_POST_MOD;
    yed_trigger_event(&event);
}

void yed_default_command_delete_line(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    yed_event  event;
    int        row,
               n_lines;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (frame->buffer->flags & BUFF_RD_ONLY) {
        yed_append_text_to_cmd_buff("[!] buffer is read-only");
        return;
    }

    n_lines = bucket_array_len(frame->buffer->lines);
    row     = frame->cursor_line;

    event.kind  = EVENT_BUFFER_PRE_MOD;
    event.frame = frame;
    event.row   = frame->cursor_line;

    yed_trigger_event(&event);

    if (n_lines > 1) {
        if (row == n_lines) {
            yed_move_cursor_within_frame(frame, 0, -1);
        }
        yed_buff_delete_line(frame->buffer, row);
    } else {
        ASSERT(row == 1, "only one line, but not on line one");

        line = yed_buff_get_line(frame->buffer, row);
        yed_set_cursor_within_frame(frame, 1, 1);
        yed_line_clear(line);
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

    event.kind = EVENT_BUFFER_POST_MOD;
    yed_trigger_event(&event);
}

void yed_default_command_write_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    char       *path;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (n_args == 0) {
        if (buff->path == NULL) {
            yed_append_text_to_cmd_buff("[!] buffer is not associated with a file yet -- provide a file name");
            return;
        }
        path = buff->path;
    } else if (n_args == 1) {
        path = args[0];
    } else {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_write_buff_to_file(buff, path);

    yed_append_text_to_cmd_buff("wrote to '");
    yed_append_text_to_cmd_buff((char*)path);
    yed_append_text_to_cmd_buff("'");
}

void yed_default_command_plugin_load(int n_args, char **args) {
    int err;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    err = yed_load_plugin(args[0]);
    if (err == YED_PLUG_SUCCESS) {
        yed_append_text_to_cmd_buff("loaded plugin '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("'");
    } else {
        yed_append_text_to_cmd_buff("[!] ('");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("') -- ");

        switch (err) {
            case YED_PLUG_NOT_FOUND:
                yed_append_text_to_cmd_buff("could not find plugin -- ");
                yed_append_text_to_cmd_buff(dlerror());
                break;
            case YED_PLUG_NO_BOOT:
                yed_append_text_to_cmd_buff("could not find symbol 'yed_plugin_boot'");
                break;
            case YED_PLUG_BOOT_FAIL:
                yed_append_text_to_cmd_buff("'yed_plugin_boot' failed");
                break;
        }
    }
}

void yed_default_command_plugin_unload(int n_args, char **args) {
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    it = tree_lookup(ys->plugins, args[0]);

    if (!tree_it_good(it)) {
        yed_append_text_to_cmd_buff("[!] no plugin named '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("' is loaded");
        return;
    }

    yed_unload_plugin(args[0]);
    yed_append_text_to_cmd_buff("success");
}

void yed_default_command_plugins_list(int n_args, char **args) {
    const char *comma;
    tree_it(yed_plugin_name_t, yed_plugin_ptr_t) it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

    tree_traverse(ys->plugins, it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff((char*)tree_it_key(it));
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}

void yed_default_command_plugins_add_dir(int n_args, char **args) {
    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_add_plugin_dir(args[0]);
}

void yed_default_command_plugins_list_dirs(int n_args, char **args) {
    char  *comma;
    char **dir_it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

    array_traverse(ys->plugin_dirs, dir_it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(*dir_it);
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}

void yed_default_command_select(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
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
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
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
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    frame->dirty        = frame->dirty || buff->has_selection;
    buff->has_selection = 0;
}

void yed_default_command_yank_selection(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line_it,
               *new_line;
    yed_range  *sel;
    int         preserve_selection;
    int         row, col, r1, c1, r2, c2;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (n_args == 1) {
        preserve_selection = s_to_i(args[0]);
    } else {
        preserve_selection = 0;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind == BUFF_KIND_YANK) {
        yed_append_text_to_cmd_buff("[!] can't yank from yank buffer");
        return;
    }

    if (!buff->has_selection) {
        yed_append_text_to_cmd_buff("[!] nothing is selected");
        return;
    }

    /*
     * Clear out what's in the yank buffer.
     * We should make a cleaner way to do this in buffer.c
     */
    bucket_array_traverse(ys->yank_buff->lines, line_it) {
        yed_free_line(line_it);
    }
    bucket_array_clear(ys->yank_buff->lines);

    /* Copy the selection into the yank buffer. */
    sel = &buff->selection;
    r1 = c1 = r2 = c2 = 0;
    yed_range_sorted_points(sel, &r1, &c1, &r2, &c2);
    if (sel->kind == RANGE_LINE) {
        ys->yank_buff->flags |= BUFF_YANK_LINES;
        for (row = r1; row <= r2; row += 1) {
            new_line = yed_buffer_add_line(ys->yank_buff);
            line_it  = yed_buff_get_line(buff, row);
            for (col = 1; col <= line_it->visual_width; col += 1) {
                yed_append_to_line(new_line,
                    yed_line_col_to_char(line_it, col));
            }
        }
    } else {
        ys->yank_buff->flags &= ~(BUFF_YANK_LINES);
        new_line = yed_buffer_add_line(ys->yank_buff);
        line_it  = yed_buff_get_line(buff, r1);
        if (r1 == r2) {
            for (col = c1; col < c2; col += 1) {
                yed_append_to_line(new_line,
                    yed_line_col_to_char(line_it, col));
            }
        } else {
            for (col = c1; col <= line_it->visual_width; col += 1) {
                yed_append_to_line(new_line,
                    yed_line_col_to_char(line_it, col));
            }
            for (row = r1 + 1; row <= r2 - 1; row += 1) {
                new_line = yed_buffer_add_line(ys->yank_buff);
                line_it  = yed_buff_get_line(buff, row);
                for (col = 1; col <= line_it->visual_width; col += 1) {
                    yed_append_to_line(new_line,
                        yed_line_col_to_char(line_it, col));
                }
            }
            new_line = yed_buffer_add_line(ys->yank_buff);
            line_it  = yed_buff_get_line(buff, r2);
            for (col = 1; col < c2; col += 1) {
                yed_append_to_line(new_line,
                    yed_line_col_to_char(line_it, col));
            }
        }
    }

    if (!preserve_selection) {
        buff->has_selection = 0;
    }
    frame->dirty = 1;

}

void yed_default_command_paste_yank_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line_it,
               *new_line,
               *first_line,
               *last_line;
    yed_event   event;
    int         yank_buff_n_lines, row, col;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind == BUFF_KIND_YANK) {
        yed_append_text_to_cmd_buff("[!] can't paste into yank buffer");
        return;
    }

    if (buff->flags & BUFF_RD_ONLY) {
        yed_append_text_to_cmd_buff("[!] buffer is read-only");
        return;
    }

    event.kind  = EVENT_BUFFER_PRE_MOD;
    event.frame = frame;

    yed_trigger_event(&event);

    yank_buff_n_lines = bucket_array_len(ys->yank_buff->lines);

    ASSERT(yank_buff_n_lines, "yank buffer has no lines");

    if (ys->yank_buff->flags & BUFF_YANK_LINES) {
        for (row = 1; row <= yank_buff_n_lines; row += 1) {
            line_it  = yed_buff_get_line(ys->yank_buff, row);
            new_line = yed_buff_insert_line(buff, frame->cursor_line + row);
            for (col = 1; col <= line_it->visual_width; col += 1) {
                yed_append_to_line(new_line,
                    yed_line_col_to_char(line_it, col));
            }
        }
        yed_set_cursor_far_within_frame(frame, 1, frame->cursor_line + 1);
    } else {
        if (yank_buff_n_lines == 1) {
            line_it  = yed_buff_get_line(ys->yank_buff, 1);
            for (col = 1; col <= line_it->visual_width; col += 1) {
                yed_insert_into_line(buff,
                    frame->cursor_line,
                    frame->cursor_col + col - 1,
                    yed_line_col_to_char(line_it, col));
            }
        } else {
            first_line = yed_buff_get_line(buff, frame->cursor_line);
            last_line  = yed_buff_insert_line(buff, frame->cursor_line + 1);
            for (col = frame->cursor_col; col <= first_line->visual_width; col += 1) {
                yed_append_to_line(last_line,
                    yed_line_col_to_char(first_line, col));
            }
            for (col = first_line->visual_width; col >= frame->cursor_col; col -= 1) {
                yed_line_pop_char(first_line);
            }
            line_it  = yed_buff_get_line(ys->yank_buff, 1);
            for (col = 1; col <= line_it->visual_width; col += 1) {
                yed_append_to_line(first_line,
                    yed_line_col_to_char(line_it, col));
            }
            for (row = 2; row <= yank_buff_n_lines - 1; row += 1) {
                line_it  = yed_buff_get_line(ys->yank_buff, row);
                new_line = yed_buff_insert_line(buff, frame->cursor_line + row - 1);
                for (col = 1; col <= line_it->visual_width; col += 1) {
                    yed_append_to_line(new_line,
                        yed_line_col_to_char(line_it, col));
                }
            }
            line_it  = yed_buff_get_line(ys->yank_buff, yank_buff_n_lines);
            for (col = 1; col <= line_it->visual_width; col += 1) {
                yed_insert_into_line(buff,
                    frame->cursor_line + yank_buff_n_lines - 1,
                    col,
                    yed_line_col_to_char(line_it, col));
            }
        }
    }

    event.kind = EVENT_BUFFER_POST_MOD;
    yed_trigger_event(&event);
}

int yed_inc_find_in_buffer(void) {
    int r, c;

    yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_col, ys->search_save_row);
    if (yed_find_next(ys->active_frame->cursor_line, ys->active_frame->cursor_col, &r, &c)) {
        yed_set_cursor_far_within_frame(ys->active_frame, c, r);
        return 1;
    }
    return 0;
}

void yed_find_in_buffer_take_key(int key) {
    int found;

    if (key == CTRL_F || key == CTRL_C) {
        ys->interactive_command = NULL;
        ys->current_search      = NULL;
        yed_clear_cmd_buff();
        yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_col, ys->search_save_row);
    } else if (key == ENTER) {
        found = yed_inc_find_in_buffer();
        if (ys->save_search) {
            free(ys->save_search);
        }
        ys->save_search    = strdup(ys->current_search);
        ys->current_search = ys->save_search;

        ys->interactive_command = NULL;
        yed_clear_cmd_buff();

        if (!found) {
            yed_append_text_to_cmd_buff(ys->cmd_prompt);
            yed_append_text_to_cmd_buff("[!] '");
            yed_append_text_to_cmd_buff(ys->save_search);
            yed_append_text_to_cmd_buff("' not found");
        }

    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }

        array_zero_term(ys->cmd_buff);
        ys->current_search = array_data(ys->cmd_buff);

        yed_inc_find_in_buffer();
    }
}

void yed_start_find_in_buffer(void) {
    ys->interactive_command = "find-in-buffer";
    ys->cmd_prompt          = "(find-in-buffer) ";
    yed_set_small_message(NULL);
    ys->search_save_row     = ys->active_frame->cursor_line;
    ys->search_save_col     = ys->active_frame->cursor_col;
    yed_clear_cmd_buff();

    ys->current_search = array_data(ys->cmd_buff);
}

void yed_default_command_find_in_buffer(int n_args, char **args) {
    yed_frame *frame;
    int        key;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (!ys->interactive_command) {
        yed_start_find_in_buffer();
    } else {
        sscanf(args[0], "%d", &key);
        yed_find_in_buffer_take_key(key);
        frame->dirty = 1;
    }
}

void yed_default_command_find_next_in_buffer(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (!ys->save_search) {
        yed_append_text_to_cmd_buff("[!] no previous search");
        return;
    }

    ys->current_search  = ys->save_search;
    ys->search_save_row = ys->active_frame->cursor_line;
    ys->search_save_col = ys->active_frame->cursor_col;

    if (!yed_inc_find_in_buffer()) {
        yed_append_text_to_cmd_buff("[!] '");
        yed_append_text_to_cmd_buff(ys->current_search);
        yed_append_text_to_cmd_buff("' not found");
    } else {
        yed_append_text_to_cmd_buff(ys->current_search);
    }
}

int yed_inc_find_prev_in_buffer(void) {
    int r, c;

    yed_set_cursor_far_within_frame(ys->active_frame, ys->search_save_col, ys->search_save_row);
    if (yed_find_prev(ys->active_frame->cursor_line, ys->active_frame->cursor_col, &r, &c)) {
        yed_set_cursor_far_within_frame(ys->active_frame, c, r);
        return 1;
    }
    return 0;
}

void yed_default_command_find_prev_in_buffer(int n_args, char **args) {
    yed_frame *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (!ys->save_search) {
        yed_append_text_to_cmd_buff("[!] no previous search");
        return;
    }

    ys->current_search  = ys->save_search;
    ys->search_save_row = ys->active_frame->cursor_line;
    ys->search_save_col = ys->active_frame->cursor_col;

    if (!yed_inc_find_prev_in_buffer()) {
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
            yed_insert_into_line(buff, row, *mark + (it * len), replacement[i]);
        }
        it += 1;
    }
}

void yed_replace_current_search_update(void) {
    int row, r1, c1, r2, c2, i;

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

    if (key == CTRL_F || key == CTRL_C) {
        ys->interactive_command = NULL;
        yed_clear_cmd_buff();
        replace_abort();
        replace_free();
    } else if (key == ENTER) {
        yed_replace_current_search_update();

        replace_free();

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
    } else {
        if (key == BACKSPACE) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }

        array_zero_term(ys->cmd_buff);

        yed_replace_current_search_update();
    }
}

void replace_add_line(yed_buffer *buff, int row, int len) {
    yed_line *line,
             *save_line,
             *working_line;
    int       i, j;
    array_t   markers;

    line      = yed_buff_get_line(buff, row);
    markers   = array_make(int);
    save_line = yed_copy_line(line);

    array_push(ys->replace_save_lines, save_line);

    while (1) {
        for (i = 1; i + len - 1 <= array_len(line->chars); i += 1) {
            if (strncmp(array_data(line->chars) + i - 1,
                        ys->current_search, len) == 0) {

                array_push(markers, i);
                ys->replace_count += 1;

                for (j = 0; j < len; j += 1) {
                    yed_delete_from_line(buff, row, i);
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
    yed_set_small_message(NULL);
    ys->search_save_row      = ys->active_frame->cursor_line;
    ys->search_save_col      = ys->active_frame->cursor_col;
    ys->current_search       = ys->save_search;
    ys->replace_count        = 0;

    buff = ys->active_frame->buffer;
    len  = strlen(ys->current_search);

    yed_set_cursor_within_frame(ys->active_frame, 1, ys->active_frame->cursor_line);

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
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    if (!ys->save_search || !strlen(ys->save_search)) {
        yed_append_text_to_cmd_buff("[!] no previous search");
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
            yed_append_text_to_cmd_buff("'");
            yed_append_text_to_cmd_buff(ys->active_style);
            yed_append_text_to_cmd_buff("'");
        } else {
            yed_append_text_to_cmd_buff("[!] no style activated");
        }
    } else if (n_args == 1) {
        if (!yed_activate_style(args[0])) {
            yed_append_text_to_cmd_buff("no such style '");
            yed_append_text_to_cmd_buff(args[0]);
            yed_append_text_to_cmd_buff("'");
        }
    } else {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }
}

void yed_default_command_style_off(int n_args, char **args) {
    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    ys->active_style = NULL;
    ys->redraw       = 1;
}

int yed_execute_command(char *name, int n_args, char **args) {
    tree_it(yed_command_name_t, yed_command)  it;
    yed_command                               cmd;

    if (!ys->interactive_command) {
        yed_clear_cmd_buff();
    }

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        yed_append_non_text_to_cmd_buff(TERM_RED);
        yed_append_text_to_cmd_buff("unknown command '");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff("'");
        yed_append_non_text_to_cmd_buff(TERM_RESET);
        yed_append_non_text_to_cmd_buff(TERM_CURSOR_HIDE);

        return 1;
    }

    cmd = tree_it_val(it);

    if (!ys->interactive_command) {
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff(name);
        yed_append_text_to_cmd_buff(") ");
    }

    cmd(n_args, args);

    yed_draw_command_line();

    return 0;
}
