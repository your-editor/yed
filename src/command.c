#include "internal.h"

static void yed_init_commands(void) {
    ys->commands = tree_make_c(yed_command_name_t, yed_command_t, strcmp);
    yed_add_default_commands();
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

    ADD_DEFAULT_COMMAND("quit",             quit);
    ADD_DEFAULT_COMMAND("reload",           reload);
    ADD_DEFAULT_COMMAND("echo",             echo);
    ADD_DEFAULT_COMMAND("cursor-down",      cursor_down);
    ADD_DEFAULT_COMMAND("cursor-up",        cursor_up);
    ADD_DEFAULT_COMMAND("cursor-left",      cursor_left);
    ADD_DEFAULT_COMMAND("cursor-right",     cursor_right);
    ADD_DEFAULT_COMMAND("buffer-new",       buffer_new);
    ADD_DEFAULT_COMMAND("frame",            frame);
    ADD_DEFAULT_COMMAND("frames-list",      frames_list);
    ADD_DEFAULT_COMMAND("frame-set-buffer", frame_set_buffer);
    ADD_DEFAULT_COMMAND("frame-new-file",   frame_new_file);
}

static void yed_reload_default_commands(void) {
#define RELOAD_DEFAULT_COMMAND(name1, name2) yed_reload_command(name1, &yed_default_command_##name2)

    RELOAD_DEFAULT_COMMAND("quit",             quit);
    RELOAD_DEFAULT_COMMAND("reload",           reload);
    RELOAD_DEFAULT_COMMAND("echo",             echo);
    RELOAD_DEFAULT_COMMAND("cursor-down",      cursor_down);
    RELOAD_DEFAULT_COMMAND("cursor-up",        cursor_up);
    RELOAD_DEFAULT_COMMAND("cursor-left",      cursor_left);
    RELOAD_DEFAULT_COMMAND("cursor-right",     cursor_right);
    RELOAD_DEFAULT_COMMAND("buffer-new",       buffer_new);
    RELOAD_DEFAULT_COMMAND("frame",            frame);
    RELOAD_DEFAULT_COMMAND("frames-list",      frames_list);
    RELOAD_DEFAULT_COMMAND("frame-set-buffer", frame_set_buffer);
    RELOAD_DEFAULT_COMMAND("frame-new-file",   frame_new_file);
}

static void yed_default_command_quit(int n_args, char **args) {
    ys->status = YED_QUIT;
}

static void yed_default_command_reload(int n_args, char **args) {
    ys->status = YED_RELOAD;
    append_to_output_buff("issued reload");
}

static void yed_default_command_echo(int n_args, char **args) {
    int         i;
    const char *space;

    yed_set_cursor(1, ys->term_rows);
    space = "";
    for (i = 0; i < n_args; i += 1) {
        append_to_output_buff(space);
        append_to_output_buff(args[i]);
        space = " ";
    }
}

static void yed_default_command_cursor_down(int n_args, char **args) {
    if (n_args == 0) {
        ys->save_cur_y += 1;
    }
}

static void yed_default_command_cursor_up(int n_args, char **args) {
    if (n_args == 0) {
        ys->save_cur_y -= 1;
    }
}

static void yed_default_command_cursor_left(int n_args, char **args) {
    if (n_args == 0) {
        ys->save_cur_x -= 1;
    }
}

static void yed_default_command_cursor_right(int n_args, char **args) {
    if (n_args == 0) {
        ys->save_cur_x += 1;
    }
}

static void yed_default_command_buffer_new(int n_args, char **args) {
    int         buff_nr;
    yed_buffer *buffer;

    yed_set_cursor(1, ys->term_rows);

    if (n_args != 1) {
        append_to_output_buff("expected one argument but got ");
        append_int_to_output_buff(n_args);
    } else {
        yed_add_new_buff();
        buff_nr = ys->n_buffers - 1;
        buffer  = ys->buff_list[buff_nr];

        yed_fill_buff_from_file(buffer, args[0]);

        append_to_output_buff("new buffer number is ");
        append_int_to_output_buff(buff_nr);
    }
}

static void yed_default_command_frame(int n_args, char**args) {
    yed_frame *frame;

    if (n_args == 0) {
        if (!ys->active_frame) {
            append_to_output_buff("no active frame");
            return;
        }
        frame = ys->active_frame;
    } else if (n_args == 1) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            append_to_output_buff("(new frame '");
            append_to_output_buff(args[0]);
            append_to_output_buff("') ");
        }
        frame = yed_get_or_add_frame(args[0]);
    } else if (n_args == 5) {
        frame = yed_get_frame(args[0]);
        if (!frame) {
            append_to_output_buff("(new frame '");
            append_to_output_buff(args[0]);
            append_to_output_buff("') ");
        }
        frame = yed_get_or_add_frame(args[0]);

        yed_clear_frame(frame);

        frame->top    = s_to_i(args[1]);
        frame->left   = s_to_i(args[2]);
        frame->height = s_to_i(args[4]);
        frame->width  = s_to_i(args[3]);
    } else {
        append_to_output_buff("expected zero, one, or five arguments but got ");
        append_int_to_output_buff(n_args);
        return;
    }

    yed_activate_frame(frame);
    append_to_output_buff("active frame is '");
    append_to_output_buff(ys->active_frame->id);
    append_to_output_buff("'");
}

static void yed_default_command_frames_list(int n_args, char **args) {
    const char *comma;
    tree_it(yed_frame_id_t, yed_frame_ptr_t) it;

    if (n_args != 0) {
        append_to_output_buff("expected zero arguments but got ");
        append_int_to_output_buff(n_args);
        return;
    }

    comma = "";

    tree_traverse(ys->frames, it) {
        append_to_output_buff(comma);
        append_to_output_buff("'");
        append_to_output_buff(tree_it_val(it)->id);
        append_to_output_buff("'");
        comma = ", ";
    }
}

static void yed_default_command_frame_set_buffer(int n_args, char **args) {
    yed_buffer *buffer;
    int         buff_nr;

    if (n_args != 1) {
        append_to_output_buff("expected one argument but got ");
        append_int_to_output_buff(n_args);
        return;
    }

    if (!ys->active_frame) {
        append_to_output_buff("no active frame");
        return;
    }

    buff_nr = s_to_i(args[0]);

    if (buff_nr >= ys->n_buffers) {
        append_to_output_buff("no buffer ");
        append_int_to_output_buff(buff_nr);
        return;
    }

    buffer = ys->buff_list[buff_nr];
    yed_frame_set_buff(ys->active_frame, buffer);
}

static void yed_default_command_frame_new_file(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buffer;
    int         buff_nr;

    if (n_args != 1) {
        append_to_output_buff("expected one argument but got ");
        append_int_to_output_buff(n_args);
        return;
    }

    frame = yed_get_or_add_frame(args[0]);

    yed_add_new_buff();
    buff_nr = ys->n_buffers - 1;
    buffer  = ys->buff_list[buff_nr];

    yed_fill_buff_from_file(buffer, args[0]);

    yed_frame_set_buff(frame, buffer);

    yed_activate_frame(frame);
    append_to_output_buff("active frame is '");
    append_to_output_buff(ys->active_frame->id);
    append_to_output_buff("'");
}



static void yed_command_prompt(void) {
    yed_set_cursor(1, ys->term_rows);
    append_to_output_buff(TERM_CLEAR_LINE);
    append_to_output_buff(": ");
    yed_set_cursor(3, ys->term_rows);
    ys->accepting_command = 1;
}

static void yed_do_command(void) {
    char          *cmd_beg,
                  *cmd_end,
                  *cmd_curs;
    int            cmd_len;
    yed_command_t  cmd_fn;
    int            n_split,
                   last_was_space;
    char          *cmd_split[32];

    cmd_beg = cmd_curs = ys->command_buff;
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

    yed_set_cursor(1, ys->term_rows);
    append_to_output_buff(TERM_CLEAR_LINE);
    ys->accepting_command = 0;

    cmd_fn = yed_get_command(cmd_beg);

    if (cmd_fn) {
        cmd_fn(n_split - 1, cmd_split + 1);
    } else {
        yed_set_cursor(1, ys->term_rows);
        append_to_output_buff("unknown command '");
        append_to_output_buff(ys->command_buff);
        append_to_output_buff("'");
        append_to_output_buff(TERM_RED);
    }

    ys->command_buff[0] = 0;
}

static void yed_command_take_key(int key) {
    int command_len;

    if (key == CTRL('f')) {
        ys->accepting_command = 0;
        append_to_output_buff(TERM_CLEAR_LINE);
        if (ys->active_frame) {
            yed_set_cursor(ys->active_frame->cur_x, ys->active_frame->cur_y);
        } else {
            yed_set_cursor(ys->save_cur_x, ys->save_cur_y);
        }
    } else if (key == '\n') {
        yed_do_command();
        if (ys->active_frame) {
            yed_set_cursor(ys->active_frame->cur_x, ys->active_frame->cur_y);
        } else {
            yed_set_cursor(ys->save_cur_x, ys->save_cur_y);
        }
    } else {
        yed_set_cursor(1, ys->term_rows);
        append_to_output_buff(TERM_CLEAR_LINE);

        if (key == 127) {
            command_len = strlen(ys->command_buff);
            if (command_len) {
                ys->command_buff[command_len - 1] = 0;
            }
        } else if (!iscntrl(key)) {
            strncat(ys->command_buff, (char*)&key, 1);
        }
        append_to_output_buff(": ");
        append_to_output_buff(ys->command_buff);
        yed_set_cursor(3 + strlen(ys->command_buff), ys->term_rows);
    }
}


