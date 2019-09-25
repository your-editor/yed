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
    ADD_DEFAULT_COMMAND("buffer-new",           buffer_new);
    ADD_DEFAULT_COMMAND("frame",                frame);
    ADD_DEFAULT_COMMAND("frames-list",          frames_list);
    ADD_DEFAULT_COMMAND("frame-set-buffer",     frame_set_buffer);
    ADD_DEFAULT_COMMAND("frame-new-file",       frame_new_file);
    ADD_DEFAULT_COMMAND("frame-split-new-file", frame_split_new_file);
    ADD_DEFAULT_COMMAND("frame-next",           frame_next);
    ADD_DEFAULT_COMMAND("insert",               insert);
    ADD_DEFAULT_COMMAND("delete-back",          delete_back);
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
    RELOAD_DEFAULT_COMMAND("buffer-new",           buffer_new);
    RELOAD_DEFAULT_COMMAND("frame",                frame);
    RELOAD_DEFAULT_COMMAND("frames-list",          frames_list);
    RELOAD_DEFAULT_COMMAND("frame-set-buffer",     frame_set_buffer);
    RELOAD_DEFAULT_COMMAND("frame-new-file",       frame_new_file);
    RELOAD_DEFAULT_COMMAND("insert",               insert);
    RELOAD_DEFAULT_COMMAND("delete-back",          delete_back);
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
        cmd_fn(n_split - 1, cmd_split + 1);
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
    int rows;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("cursor-down: expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
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
    int rows;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("cursor-up: expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
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
    int cols;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("cursor-left: expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
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
    int cols;

    if (n_args > 1) {
        yed_append_text_to_cmd_buff("cursor-right: expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (n_args == 0) {
        cols = 1;
    } else {
        cols = s_to_i(args[0]);
    }

    yed_move_cursor_within_active_frame(cols, 0);
}

static void yed_default_command_buffer_new(int n_args, char **args) {
    int         buff_nr;
    yed_buffer *buffer;

    yed_set_cursor(1, ys->term_rows);

    if (n_args > 2) {
        yed_append_text_to_cmd_buff("expected zero or one arguments but got ");
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

    if (n_args == 0) {
        if (!ys->active_frame) {
            yed_append_text_to_cmd_buff("no active frame");
            return;
        }
        frame = ys->active_frame;
    } else if (n_args == 1) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            yed_append_text_to_cmd_buff("(new frame '");
            yed_append_text_to_cmd_buff(args[0]);
            yed_append_text_to_cmd_buff("') ");
        }
        frame = yed_get_or_add_frame(args[0]);
    } else if (n_args == 5) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            yed_append_text_to_cmd_buff("(new frame '");
            yed_append_text_to_cmd_buff(args[0]);
            yed_append_text_to_cmd_buff("') ");
        }
        frame = yed_get_or_add_frame(args[0]);

        yed_clear_frame(frame);

        frame->top    = s_to_i(args[1]);
        frame->left   = s_to_i(args[2]);
        frame->height = s_to_i(args[3]);
        frame->width  = s_to_i(args[4]);
    } else {
        yed_append_text_to_cmd_buff("expected zero, one, or five arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
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
        yed_append_text_to_cmd_buff("expected zero arguments but got ");
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
        yed_append_text_to_cmd_buff("expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("no active frame");
        return;
    }

    buff_nr = s_to_i(args[0]);

    if (buff_nr >= ys->n_buffers) {
        yed_append_text_to_cmd_buff("no buffer ");
        yed_append_int_to_cmd_buff(buff_nr);
        return;
    }

    buffer = ys->buff_list[buff_nr];
    yed_frame_set_buff(ys->active_frame, buffer);
    yed_clear_frame(ys->active_frame);
    ys->active_frame->dirty = 1;
}

static void yed_default_command_frame_new_file(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer;
    int         buff_nr;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    frame = yed_get_or_add_frame(args[0]);

    yed_add_new_buff();
    buff_nr = ys->n_buffers - 1;
    buffer  = ys->buff_list[buff_nr];

    yed_fill_buff_from_file(buffer, args[0]);

    yed_frame_set_buff(frame, buffer);

    yed_activate_frame(frame);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

static void yed_default_command_frame_split_new_file(int n_args, char **args) {
    yed_frame  *frame1,
               *frame2;
    yed_buffer *buffer;
    int         buff_nr;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("frame-split-new-file: expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("frame-split-new-file: no active frame");
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

    yed_fill_buff_from_file(buffer, args[0]);

    yed_frame_set_buff(frame2, buffer);

    yed_clear_frame(frame1);
    yed_clear_frame(frame2);

    yed_activate_frame(frame2);
    yed_append_text_to_cmd_buff("active frame is '");
    yed_append_text_to_cmd_buff((char*)ys->active_frame->id);
    yed_append_text_to_cmd_buff("'");
}

static void yed_default_command_frame_next(int n_args, char **args) {
    tree_it(yed_frame_id_t, yed_frame_ptr_t)  it;
    yed_frame                                *frame;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("frame-next: expected zero arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("frame-next: no active frame");
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
    int        col,
               len;
    char      *char_it;

    if (n_args != 1) {
        yed_append_text_to_cmd_buff("insert: expected one argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("insert: no active frame ");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("insert: active frame has no buffer");
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    col  = frame->cur_x - frame->left + 1 + frame->buffer_x_offset;

    if (args[0][0] == '\n') {
        new_line = yed_buff_insert_line(frame->buffer, frame->cursor_line + 1);

        len = 0;
        array_traverse_from(line->chars, char_it, col - 1) {
            array_push(new_line->chars, *char_it);
            len += 1;
        }
        for (; len > 0; len -= 1)    { array_pop(line->chars); }

        frame->buffer_x_offset = 0;
        yed_set_cursor_within_frame(frame, 1, frame->cur_y + 1);
        frame->dirty = 1;
    } else {
        yed_insert_into_line(frame->buffer, line, col, args[0][0]);
        yed_move_cursor_within_frame(frame, 1, 0);
    }

}

static void yed_default_command_delete_back(int n_args, char **args) {
    yed_frame *frame;
    yed_line  *line,
              *previous_line;
    int        col,
               prev_line_len, old_line_nr;
    char      *char_it;

    if (n_args != 0) {
        yed_append_text_to_cmd_buff("delete-back: expected zero argument but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("delete-back: active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("delete-back: active frame has no buffer");
        return;
    }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    col  = frame->cur_x - frame->left + 1 + frame->buffer_x_offset;

    if (col == 1) {
        if (frame->cursor_line > 1) {
            old_line_nr = frame->cursor_line;
            previous_line = yed_buff_get_line(frame->buffer, frame->cursor_line - 1);
            prev_line_len = array_len(previous_line->chars);

            array_traverse(line->chars, char_it) {
                array_push(previous_line->chars, *char_it);
            }

            /*
             * Move to the previous line.
             */
            if (frame->buffer_y_offset
            && frame->buffer_y_offset + frame->height < array_len(frame->buffer->lines)) {

                frame->buffer_y_offset -= 1;
                frame->cursor_line     -= 1;
            } else {
                frame->buffer_x_offset = 0;
                yed_move_cursor_within_frame(frame, 0, -1);
            }

            yed_set_cursor_within_frame(frame, 1, frame->cur_y);
            yed_move_cursor_within_frame(frame, prev_line_len, 0);

            /*
             * Delete the old line.
             */
            yed_buff_delete_line(frame->buffer, old_line_nr);
        }
    } else {
        yed_delete_from_line(frame->buffer, line, col - 1);
        yed_move_cursor_within_frame(frame, -1, 0);
    }

}

static void yed_default_command_write_buffer(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    const char *path;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("write-buffer: no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("write-buffer: active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (n_args == 0) {
        if (buff->path == NULL) {
            yed_append_text_to_cmd_buff("write-buffer: buffer is not associated with a file yet -- provide a file name");
            return;
        }
        path = buff->path;
    } else if (n_args == 1) {
        path = args[0];
    } else {
        yed_append_text_to_cmd_buff("write-buffer: expected zero or one arguments but got ");
        yed_append_int_to_cmd_buff(n_args);
        return;
    }

    yed_write_buff_to_file(buff, path);

    yed_append_text_to_cmd_buff("write-buffer: wrote to '");
    yed_append_text_to_cmd_buff((char*)path);
    yed_append_text_to_cmd_buff("'");
}

static int yed_execute_command(const char *name, int n_args, char **args) {
    tree_it(yed_command_name_t, yed_command_t) it;
    yed_command_t                              cmd;

    if (strcmp(name, "command-prompt") != 0) {
        yed_clear_cmd_buff();
    }

    it = tree_lookup(ys->commands, name);

    if (!tree_it_good(it))    { return 1; }

    cmd = tree_it_val(it);

    cmd(n_args, args);

    yed_draw_command_line();

    return 0;
}

