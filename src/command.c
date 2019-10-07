#include "internal.h"

static void yed_init_commands(void) {
    ys->commands = tree_make_c(yed_command_name_t, yed_command_t, strcmp);
    yed_add_default_commands();
    ys->cmd_buff = array_make(char);
}

static void yed_add_command(const char *name, yed_command_t command) {
    tree_insert(ys->commands, strdup(name), command);
}

static void yed_reload_command(const char *name, yed_command_t command) {
    tree_it(yed_command_name_t, yed_command_t) it;

    it = tree_lookup(ys->commands, name);

    if (tree_it_good(it)) {
        tree_it_val(it) = command;
    }
}

static yed_command_t yed_get_command(const char *name) {
    tree_it(yed_command_name_t, yed_command_t) it;

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it)) {
        return NULL;
    }

    return tree_it_val(it);
}

static void yed_add_default_commands(void) {
#define ADD_DEFAULT_COMMAND(name1, name2) yed_add_command(name1, &yed_default_command_##name2)

    ADD_DEFAULT_COMMAND("command-prompt",       command_prompt);
    ADD_DEFAULT_COMMAND("quit",                 quit);
    ADD_DEFAULT_COMMAND("reload",               reload);
    ADD_DEFAULT_COMMAND("echo",                 echo);
    ADD_DEFAULT_COMMAND("cursor-down",          cursor_down);
    ADD_DEFAULT_COMMAND("cursor-up",            cursor_up);
    ADD_DEFAULT_COMMAND("cursor-left",          cursor_left);
    ADD_DEFAULT_COMMAND("cursor-right",         cursor_right);
    ADD_DEFAULT_COMMAND("cursor-line-begin",    cursor_line_begin);
    ADD_DEFAULT_COMMAND("cursor-line-end",      cursor_line_end);
    ADD_DEFAULT_COMMAND("cursor-next-word",     cursor_next_word);
    ADD_DEFAULT_COMMAND("cursor-buffer-begin",  cursor_buffer_begin);
    ADD_DEFAULT_COMMAND("cursor-buffer-end",    cursor_buffer_end);
    ADD_DEFAULT_COMMAND("buffer-new",           buffer_new);
    ADD_DEFAULT_COMMAND("frame",                frame);
    ADD_DEFAULT_COMMAND("frames-list",          frames_list);
    ADD_DEFAULT_COMMAND("frame-set-buffer",     frame_set_buffer);
    ADD_DEFAULT_COMMAND("frame-new-file",       frame_new_file);
    ADD_DEFAULT_COMMAND("frame-split-new-file", frame_split_new_file);
    ADD_DEFAULT_COMMAND("frame-next",           frame_next);
    ADD_DEFAULT_COMMAND("insert",               insert);
    ADD_DEFAULT_COMMAND("delete-back",          delete_back);
    ADD_DEFAULT_COMMAND("delete-line",          delete_line);
    ADD_DEFAULT_COMMAND("write-buffer",         write_buffer);
}

static void yed_reload_default_commands(void) {
#define RELOAD_DEFAULT_COMMAND(name1, name2) yed_reload_command(name1, &yed_default_command_##name2)

    RELOAD_DEFAULT_COMMAND("command-prompt",       command_prompt);
    RELOAD_DEFAULT_COMMAND("quit",                 quit);
    RELOAD_DEFAULT_COMMAND("reload",               reload);
    RELOAD_DEFAULT_COMMAND("echo",                 echo);
    RELOAD_DEFAULT_COMMAND("cursor-down",          cursor_down);
    RELOAD_DEFAULT_COMMAND("cursor-up",            cursor_up);
    RELOAD_DEFAULT_COMMAND("cursor-left",          cursor_left);
    RELOAD_DEFAULT_COMMAND("cursor-right",         cursor_right);
    RELOAD_DEFAULT_COMMAND("cursor-line-begin",    cursor_line_begin);
    RELOAD_DEFAULT_COMMAND("cursor-line-end",      cursor_line_end);
    RELOAD_DEFAULT_COMMAND("cursor-buffer-begin",  cursor_buffer_begin);
    RELOAD_DEFAULT_COMMAND("cursor-buffer-end",    cursor_buffer_end);
    RELOAD_DEFAULT_COMMAND("buffer-new",           buffer_new);
    RELOAD_DEFAULT_COMMAND("frame",                frame);
    RELOAD_DEFAULT_COMMAND("frames-list",          frames_list);
    RELOAD_DEFAULT_COMMAND("frame-set-buffer",     frame_set_buffer);
    RELOAD_DEFAULT_COMMAND("frame-new-file",       frame_new_file);
    RELOAD_DEFAULT_COMMAND("insert",               insert);
    RELOAD_DEFAULT_COMMAND("delete-back",          delete_back);
    RELOAD_DEFAULT_COMMAND("delete-line",          delete_line);
    RELOAD_DEFAULT_COMMAND("write-buffer",         write_buffer);
    RELOAD_DEFAULT_COMMAND("frame-split-new-file", frame_split_new_file);
}

static void yed_clear_cmd_buff(void) {
    array_clear(ys->cmd_buff);
    array_zero_term(ys->cmd_buff);
    ys->cmd_cursor_x = 1 + strlen(YED_CMD_PROMPT);
}

static void yed_append_to_cmd_buff(char *s) {
    int i, len;

    len = strlen(s);
    for (i = 0; i < len; i += 1) {
        array_push(ys->cmd_buff, s[i]);
    }
}

static void yed_append_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
    ys->cmd_cursor_x += strlen(s);
}

static void yed_append_non_text_to_cmd_buff(char *s) {
    yed_append_to_cmd_buff(s);
}

static void yed_append_int_to_cmd_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    yed_append_text_to_cmd_buff(s);
}

static void yed_cmd_buff_push(char c) {
    array_push(ys->cmd_buff, c);
    ys->cmd_cursor_x += 1;
}

static void yed_cmd_buff_pop(void) {
    array_pop(ys->cmd_buff);
    ys->cmd_cursor_x -= 1;
}

static void yed_draw_command_line(void) {
    yed_set_cursor(1, ys->term_rows);
    append_to_output_buff(TERM_CLEAR_LINE);
    if (ys->accepting_command) {
        append_to_output_buff(YED_CMD_PROMPT);
    }
    append_n_to_output_buff(array_data(ys->cmd_buff), array_len(ys->cmd_buff));
}

static void yed_start_command_prompt(void) {
    ys->accepting_command = 1;
    yed_clear_cmd_buff();
}

static void yed_do_command(void) {
    char          *cmd_cpy,
                  *cmd_beg,
                  *cmd_end,
                  *cmd_curs;
    int            cmd_len;
    yed_command_t  cmd_fn;
    int            n_split,
                   last_was_space;
    char          *cmd_split[32];

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

    ys->accepting_command = 0;

    cmd_fn = yed_get_command(cmd_beg);

    if (cmd_fn) {
        yed_execute_command(cmd_beg, n_split - 1, cmd_split + 1);
/*         cmd_fn(n_split - 1, cmd_split + 1); */
    } else {
        yed_append_non_text_to_cmd_buff(TERM_RED);
        yed_append_text_to_cmd_buff("unknown command '");
        yed_append_text_to_cmd_buff(cmd_beg);
        yed_append_text_to_cmd_buff("'");
        yed_append_non_text_to_cmd_buff(TERM_RESET);
        yed_append_non_text_to_cmd_buff(TERM_CURSOR_HIDE);
    }

    free(cmd_cpy);
}

static void yed_command_take_key(int key) {
    tree_it(yed_command_name_t, yed_command_t) it;

    if (key == CTRL('f')) {
        ys->accepting_command = 0;
        yed_clear_cmd_buff();
    } else if (key == KEY_TAB) {
        array_zero_term(ys->cmd_buff);
        it = tree_gtr(ys->commands, array_data(ys->cmd_buff));
        if (tree_it_good(it)) {
            yed_clear_cmd_buff();
            yed_append_text_to_cmd_buff((char*)tree_it_key(it));
        }
    } else if (key == '\n') {
        yed_do_command();
        ys->accepting_command = 0;
    } else {
        if (key == 127) {
            if (array_len(ys->cmd_buff)) {
                yed_cmd_buff_pop();
            }
        } else if (!iscntrl(key)) {
            yed_cmd_buff_push(key);
        }
    }
}

static void yed_default_command_command_prompt(int n_args, char **args) {
    if (!ys->accepting_command) {
        yed_start_command_prompt();
    } else {
        yed_command_take_key(args[0][0]);
    }
}

static void yed_default_command_quit(int n_args, char **args) {
    ys->status = YED_QUIT;
}

static void yed_default_command_reload(int n_args, char **args) {
    ys->status = YED_RELOAD;
    yed_append_text_to_cmd_buff("issued reload");
}

static void yed_default_command_echo(int n_args, char **args) {
    int         i;
    const char *space;

    space = "";
    for (i = 0; i < n_args; i += 1) {
        yed_append_text_to_cmd_buff((char*)space);
        yed_append_text_to_cmd_buff(args[i]);
        space = " ";
    }
}

static void yed_default_command_cursor_down(int n_args, char **args) {
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


    if (n_args == 0) {
        rows = 1;
    } else {
        rows = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

static void yed_default_command_cursor_up(int n_args, char **args) {
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


    if (n_args == 0) {
        rows = -1;
    } else {
        rows = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(0, rows);
}

static void yed_default_command_cursor_left(int n_args, char **args) {
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


    if (n_args == 0) {
        cols = -1;
    } else {
        cols = -1 * s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

static void yed_default_command_cursor_right(int n_args, char **args) {
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

    if (n_args == 0) {
        cols = 1;
    } else {
        cols = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

static void yed_default_command_cursor_line_begin(int n_args, char **args) {
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

    yed_set_cursor_within_frame(frame, 1, frame->cursor_line);
}

static void yed_default_command_cursor_line_end(int n_args, char **args) {
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

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    yed_set_cursor_within_frame(frame, line->visual_width + 1, frame->cursor_line);
}

static void yed_default_command_cursor_buffer_begin(int n_args, char **args) {
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

    yed_set_cursor_far_within_frame(frame, 1, 1);
    frame->dirty = 1;
}

static void yed_default_command_cursor_buffer_end(int n_args, char **args) {
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

    last_line = bucket_array_last(frame->buffer->lines);
    yed_set_cursor_far_within_frame(frame, last_line->visual_width + 1, bucket_array_len(frame->buffer->lines));
}

static void yed_default_command_cursor_next_word(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
    yed_cell  *cell_it;
    int        cols;
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

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    cols    = 0;
    cell_it = array_item(line->cells, frame->cursor_col - 1);
    c       = cell_it->c;
    if (isspace(c)) {
        array_traverse_from(line->cells, cell_it, frame->cursor_col) {
            c = cell_it->c;
            cols += 1;
            if (!isspace(c)) {
                break;
            }
        }
    } else if (isalnum(c) || c == '_') {
        array_traverse_from(line->cells, cell_it, frame->cursor_col) {
            c = cell_it->c;
            cols += 1;
            if (!isalnum(c) && c != '_') {
                break;
            }
        }
        if (isspace(c)) {
            array_traverse_from(line->cells, cell_it, frame->cursor_col + cols) {
                c = cell_it->c;
                cols += 1;
                if (!isspace(c)) {
                    break;
                }
            }
        }
    } else {
        array_traverse_from(line->cells, cell_it, frame->cursor_col) {
            c = cell_it->c;
            cols += 1;
            if (!isalnum(c) && c != '_') {
                break;
            }
        }
        if (isspace(c)) {
            array_traverse_from(line->cells, cell_it, frame->cursor_col + cols) {
                c = cell_it->c;
                cols += 1;
                if (!isspace(c)) {
                    break;
                }
            }
        }
    }

    if (cols == 0 && frame->cursor_line < bucket_array_len(frame->buffer->lines)) {
        yed_move_cursor_within_frame(frame, 0, 1);
        yed_set_cursor_within_frame(frame, 1, frame->cursor_line);
    } else {
        yed_move_cursor_within_frame(frame, cols, 0);
    }
}

static void yed_default_command_buffer_new(int n_args, char **args) {
    int         buff_nr;
    yed_buffer *buffer;

    yed_set_cursor(1, ys->term_rows);

    if (n_args > 2) {
        yed_append_text_to_cmd_buff("[!] expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_add_new_buff();
    buff_nr = ys->n_buffers - 1;
    buffer  = ys->buff_list[buff_nr];

    if (n_args == 1) {
        yed_fill_buff_from_file(buffer, args[0]);
    }

    yed_append_text_to_cmd_buff("new buffer number is ");
    yed_append_int_to_cmd_buff(buff_nr);
}

static void yed_default_command_frame(int n_args, char**args) {
    yed_frame *frame;
    int        created_new;

    created_new = 0;

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_append_text_to_cmd_buff("[!] no active frame");
            return;
        }
        frame = ys->active_frame;
    } else if (n_args == 1) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            created_new = 1;
        }
        frame = yed_get_or_add_frame(args[0]);
    } else if (n_args == 5) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            created_new = 1;
        }
        frame = yed_get_or_add_frame(args[0]);

        yed_clear_frame(frame);

        frame->top    = s_to_i(args[1]);
        frame->left   = s_to_i(args[2]);
        frame->height = s_to_i(args[3]);
        frame->width  = s_to_i(args[4]);

        if (!created_new) {
            yed_frame_reset_cursor(frame);
        }
    } else {
        yed_append_text_to_cmd_buff("[!] expected zero, one, or five arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (created_new) {
        yed_append_text_to_cmd_buff("(new frame '");
        yed_append_text_to_cmd_buff(args[0]);
        yed_append_text_to_cmd_buff("') ");
    }

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

static void yed_default_command_frames_list(int n_args, char **args) {
    const char *comma;
    tree_it(yed_frame_id_t, yed_frame_ptr_t) it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("[!] expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    comma = "";

    tree_traverse(ys->frames, it) {
        yed_append_text_to_cmd_buff((char*)comma);
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff((char*)tree_it_val(it)->id);
        yed_append_text_to_cmd_buff("'");
        comma = ", ";
    }
}

static void yed_default_command_frame_set_buffer(int n_args, char **args) {
    yed_buffer *buffer;
    int         buff_nr;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    buff_nr = s_to_i(args[0]);

    if (buff_nr >= ys->n_buffers) {
        yed_append_text_to_cmd_buff("[!] no buffer ");
        yed_append_int_to_cmd_buff(buff_nr);
        return;
    }

    buffer = ys->buff_list[buff_nr];
    yed_frame_set_buff(ys->active_frame, buffer);
    yed_clear_frame(ys->active_frame);
}

static void yed_default_command_frame_new_file(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer;
    int         buff_nr, new_file;
    FILE       *f_test;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    frame = yed_get_or_add_frame(args[0]);

    yed_add_new_buff();
    buff_nr = ys->n_buffers - 1;
    buffer  = ys->buff_list[buff_nr];

    new_file = 0;
    f_test   = fopen(args[0], "r");

    if (f_test) {
        fclose(f_test);
        yed_fill_buff_from_file(buffer, args[0]);
    } else {
        buffer->path = args[0];
        new_file     = 1;
    }

    yed_frame_set_buff(frame, buffer);

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");

    if (new_file) {
        yed_append_text_to_cmd_buff(" (new file)");
    }
}

static void yed_default_command_frame_split_new_file(int n_args, char **args) {
    yed_frame  *frame1,
               *frame2;
    yed_buffer *buffer;
    int         buff_nr, new_file;
    FILE       *f_test;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("[!] expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame1 = ys->active_frame;
    frame2 = yed_get_or_add_frame(args[0]);

    if (frame1->width & 1) {
        frame1->width  >>= 1;
        frame2->width    = frame1->width + 1;
    } else {
        frame1->width  >>= 1;
        frame2->width    = frame1->width;
    }
    frame2->top       = frame1->top;
    frame2->left      = frame1->left + frame1->width;
    frame2->height    = frame1->height;
    frame2->cur_y     = frame2->top;
    frame2->cur_x     = frame2->left;
    frame2->desired_x = frame2->cur_x;

    yed_add_new_buff();
    buff_nr = ys->n_buffers - 1;
    buffer  = ys->buff_list[buff_nr];

    new_file = 0;
    f_test   = fopen(args[0], "r");

    if (f_test) {
        fclose(f_test);
        yed_fill_buff_from_file(buffer, args[0]);
    } else {
        buffer->path = args[0];
        new_file     = 1;
    }

    yed_frame_set_buff(frame2, buffer);

    yed_clear_frame(frame1);
    yed_clear_frame(frame2);

    yed_activate_frame(frame2);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");

    if (new_file) {
        yed_append_text_to_cmd_buff(" (new file)");
    }
}

static void yed_default_command_frame_next(int n_args, char **args) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

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

    it = tree_lookup(ys->frames, frame->id);

    tree_it_next(it);

    if (!tree_it_good(it)) {
        it = tree_begin(ys->frames);
    }

    frame = tree_it_val(it);

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

static void yed_default_command_insert(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *new_line;
    int        col, idx,
               i, len;
    yed_cell  *cell_it;

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

    col  = frame->cursor_col;

    if (args[0][0] == '\n') {
        /*
         * Must get 'line' _after_ we've added 'new_line' because
         * the array might have expanded and so the reference to
         * 'line' could be invalid if we had acquired it before.
         */
        new_line = yed_buff_insert_line(frame->buffer, frame->cursor_line + 1);
        line     = yed_buff_get_line(frame->buffer, frame->cursor_line);

        if (col == 1) {
            new_line->cells        = line->cells;
            line->cells            = array_make(yed_cell);
            new_line->visual_width = line->visual_width;
            line->visual_width     = 0;
        } else {
            len = 0;
            idx = yed_line_col_to_cell_idx(line, col);
            array_traverse_from(line->cells, cell_it, idx) {
                yed_line_append_cell(new_line, cell_it);
                len += 1;
            }
            for (; len > 0; len -= 1)    { yed_line_pop_cell(line); }
        }

        yed_set_cursor_within_frame(frame, 1, frame->cursor_line + 1);
    } else {
        if (args[0][0] == '\t') {
            for (i = 0; i < 4; i += 1) {
                yed_insert_into_line(frame->buffer, frame->cursor_line, col + i, ' ');
            }
            yed_move_cursor_within_frame(frame, 4, 0);
        } else {
            yed_insert_into_line(frame->buffer, frame->cursor_line, col, args[0][0]);
            yed_move_cursor_within_frame(frame, 1, 0);
        }
    }
}

static void yed_default_command_delete_back(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *previous_line;
    int        col,
               buff_n_lines,
               prev_line_len, old_line_nr;
    yed_cell  *cell_it;

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

    col  = frame->cursor_col;
    line = yed_buff_get_line(frame->buffer, frame->cursor_line);

    if (col == 1) {
        if (frame->cursor_line > 1) {
            old_line_nr   = frame->cursor_line;
            previous_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
            prev_line_len = previous_line->visual_width;

            if (prev_line_len == 0) {
                previous_line->cells        = line->cells;
                line->cells                 = array_make(yed_cell);
                previous_line->visual_width = line->visual_width;
                line->visual_width          = 0;
            } else {
                array_traverse(line->cells, cell_it) {
                    yed_line_append_cell(previous_line, cell_it);
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

static void yed_default_command_delete_line(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line;
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

    n_lines = bucket_array_len(frame->buffer->lines);
    row     = frame->cursor_line;

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
}

static void yed_default_command_write_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    const char *path;

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

static int yed_execute_command(const char *name, int n_args, char **args) {
    tree_it(yed_command_name_t, yed_command_t)  it;
    yed_command_t                               cmd;
    int                                         is_cmd_prompt;

    is_cmd_prompt = (strcmp(name, "command-prompt") == 0);

    if (!is_cmd_prompt) {
        yed_clear_cmd_buff();
    }

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it))    { return 1; }

    cmd = tree_it_val(it);

    if (!is_cmd_prompt) {
        yed_append_text_to_cmd_buff("(");
        yed_append_text_to_cmd_buff((char*)name);
        yed_append_text_to_cmd_buff(") ");
    }

    cmd(n_args, args);

    yed_draw_command_line();

    return 0;
}
