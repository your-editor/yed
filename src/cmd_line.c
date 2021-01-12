#include "cmd_line.h"

void yed_cmd_line_readline_make(yed_cmd_line_readline *rdline, array_t *hist) {
    memset(rdline, 0, sizeof(*rdline));
    rdline->hist       = hist;
}

void yed_cmd_line_readline_reset(yed_cmd_line_readline *rdline, array_t *hist) {
    if (rdline->save_line != NULL) {
        free(rdline->save_line);
    }
    yed_cmd_line_readline_make(rdline, hist);
}

static void yed_cmd_line_readline_hist_prev(yed_cmd_line_readline *rdline) {
    if (rdline == NULL)         { return; }
    if (rdline->hist_item        == 1
    ||  array_len(*rdline->hist) == 0) {
        return;
    }

    if (rdline->hist_item == 0) {
        rdline->hist_item = array_len(*rdline->hist);
        rdline->save_line = yed_cmd_line_readline_get_string();
        rdline->save_x    = ys->cmd_cursor_x;
    } else {
        rdline->hist_item -= 1;
    }

    yed_clear_cmd_buff();
    ys->cmd_cursor_x = strlen(ys->cmd_prompt) + 1;
    yed_append_text_to_cmd_buff(*(char**)array_item(*rdline->hist, rdline->hist_item - 1));
}

static void yed_cmd_line_readline_hist_next(yed_cmd_line_readline *rdline) {
    if (rdline == NULL)         { return; }
    if (rdline->hist_item == 0) { return; }

    if (rdline->hist_item == array_len(*rdline->hist)) {
        rdline->hist_item = 0;
        yed_clear_cmd_buff();
        yed_append_text_to_cmd_buff(rdline->save_line);
        free(rdline->save_line);
        rdline->save_line = NULL;
        ys->cmd_cursor_x = rdline->save_x;
        return;
    } else {
        rdline->hist_item += 1;
    }

    yed_clear_cmd_buff();
    ys->cmd_cursor_x = strlen(ys->cmd_prompt) + 1;
    yed_append_text_to_cmd_buff(*(char**)array_item(*rdline->hist, rdline->hist_item - 1));
}

int yed_cmd_line_readline_take_key(yed_cmd_line_readline *rdline, int key) {
    int width;
    int prompt_width;
    int cursor_idx;

    array_zero_term(ys->cmd_buff);
    width        = yed_get_string_width(array_data(ys->cmd_buff));
    prompt_width = yed_get_string_width(ys->cmd_prompt);
    cursor_idx   = ys->cmd_cursor_x - prompt_width - 1;

    switch (key) {
        case ARROW_UP:
            yed_cmd_line_readline_hist_prev(rdline);
            break;
        case ARROW_DOWN:
            yed_cmd_line_readline_hist_next(rdline);
            break;

        case ARROW_LEFT:
            if (ys->cmd_cursor_x > prompt_width + 1) {
                ys->cmd_cursor_x -= 1;
            }
            break;
        case ARROW_RIGHT:
            if (ys->cmd_cursor_x < prompt_width + width + 1) {
                ys->cmd_cursor_x += 1;
            }
            break;
        case HOME_KEY:
        case CTRL_A:
            ys->cmd_cursor_x = 1 + prompt_width;
            break;
        case END_KEY:
        case CTRL_E:
            ys->cmd_cursor_x = 1 + prompt_width + width;
            break;
        case BACKSPACE:
            if (width && cursor_idx > 0) {
                yed_cmd_buff_delete(cursor_idx - 1);
            }
            break;
        case DEL_KEY:
            if (width && cursor_idx < width) {
                yed_cmd_buff_delete(cursor_idx);
                ys->cmd_cursor_x += 1;
            }
            break;

        /* Explicitly ignored: */
        case ESC:
            break;

        default:
            if (key < ASCII_KEY_MAX && !iscntrl(key)) {
                yed_cmd_buff_insert(cursor_idx, key);
            }
    }

    return 0;
}

char *yed_cmd_line_readline_get_string(void) {
    int   len;
    char *cpy;

    array_zero_term(ys->cmd_buff);
    len = array_len(ys->cmd_buff);
    cpy = malloc(len + 1);
    memcpy(cpy, array_data(ys->cmd_buff), len);
    cpy[len] = 0;

    return cpy;
}

void yed_builtin_cmd_prompt_start(int n_args, char **args) {
    char *prompt;
    int   i;
    char *lazy_space;

    ys->interactive_command = "command-prompt";
    prompt = yed_get_var("command-prompt-string");
    if (prompt == NULL) { prompt = DEFAULT_CMD_PROMPT_STRING; }
    ys->cmd_prompt = prompt;
    yed_clear_cmd_buff();

    lazy_space = "";
    for (i = 0; i < n_args; i += 1) {
        yed_append_text_to_cmd_buff(lazy_space);
        yed_append_text_to_cmd_buff(args[i]);
        lazy_space = " ";
    }

    yed_cmd_line_readline_reset(ys->cmd_prompt_readline, &ys->cmd_prompt_hist);
}

static void yed_builtin_cmd_prompt_compl_cleanup(void) {
    array_t *strings;

    strings = &(ys->cmd_prompt_compl_results.strings);

    if (ys->cmd_prompt_has_compl_results) {
        free_string_array(*strings);
    }

    ys->cmd_prompt_compl_item        = 0;
    ys->cmd_prompt_has_compl_results = 0;
    ys->cmd_prompt_compl_start_idx   = 0;
    ys->cmd_prompt_compl_string_len  = 0;
}

static void yed_builtin_cmd_prompt_cancel(void) {
    ys->interactive_command = NULL;
    yed_clear_cmd_buff();
    yed_builtin_cmd_prompt_compl_cleanup();
}

static void yed_builtin_cmd_prompt_run_cmd(void) {
    char     *cmd_string;
    char    **mru_cmd;
    array_t   split;

    ys->interactive_command = NULL;

    cmd_string = yed_cmd_line_readline_get_string();

    yed_clear_cmd_buff();

    split = sh_split(cmd_string);

    yed_execute_command_from_split(split);

    mru_cmd = array_last(ys->cmd_prompt_hist);

    if (strlen(cmd_string)
    &&  mru_cmd
    &&  strcmp(*mru_cmd, cmd_string) == 0) {
        free(cmd_string);
    } else {
        array_push(ys->cmd_prompt_hist, cmd_string);
    }

    free_string_array(split);
    yed_builtin_cmd_prompt_compl_cleanup();
}

static char compl_name_buff[4096];
static char compl_string_buff[4096];

static char * yed_builtin_cmd_prompt_get_compl_name_from_context(void) {
    char    *result;
    int      last_is_space;
    array_t  split;
    int      num_args;

    if (array_len(ys->cmd_buff) == 0) { return "command"; }

    result = NULL;

    array_zero_term(ys->cmd_buff);
    last_is_space = (*(char*)array_last(ys->cmd_buff)) == ' ';
    split         = sh_split(array_data(ys->cmd_buff));
    num_args      = array_len(split) + last_is_space;

    if (num_args <= 1) {
        result = "command";
    } else {
        snprintf(compl_name_buff, sizeof(compl_name_buff),
                 "%s-compl-arg-%d",
                 *(char**)array_item(split, 0),
                 num_args - 2);

        result = compl_name_buff;
    }

    free_string_array(split);

    return result;
}

static char * yed_builtin_cmd_prompt_get_compl_string_from_context(void) {
    array_t split;

    compl_string_buff[0] = 0;

    if (array_len(ys->cmd_buff) == 0
    ||  (*(char*)array_last(ys->cmd_buff)) == ' ') {
        goto out;
    }

    array_zero_term(ys->cmd_buff);
    split = sh_split(array_data(ys->cmd_buff));

    if (array_len(split) == 0) { goto out_free; }

    strncpy(compl_string_buff, *(char**)array_last(split), sizeof(compl_string_buff));

out_free:;
    free_string_array(split);
out:;
    return compl_string_buff;
}

static void yed_builtin_cmd_prompt_do_compl_fwd(void) {
    array_t *strings;
    char    *compl_string;
    char    *compl_name;
    int      compl_status;

    if (ys->cmd_cursor_x != strlen(ys->cmd_prompt) + array_len(ys->cmd_buff) + 1) {
        return;
    }

    strings = &(ys->cmd_prompt_compl_results.strings);

    if (ys->cmd_prompt_compl_item == 0) {
        yed_builtin_cmd_prompt_compl_cleanup();

        compl_name   = yed_builtin_cmd_prompt_get_compl_name_from_context();
        compl_string = yed_builtin_cmd_prompt_get_compl_string_from_context();
        compl_status = yed_complete(compl_name, compl_string, &(ys->cmd_prompt_compl_results));

        if (compl_status != COMPL_ERR_NO_ERR
        ||  array_len(*strings) == 0) {
            return;
        }

        ys->cmd_prompt_has_compl_results = 1;
        ys->cmd_prompt_compl_item        = 1;
        ys->cmd_prompt_compl_start_idx   = array_len(ys->cmd_buff);
        ys->cmd_prompt_compl_string_len  = strlen(compl_string);
    } else {
        ys->cmd_prompt_compl_item += 1;
        if (ys->cmd_prompt_compl_item > array_len(*strings)) {
            ys->cmd_prompt_compl_item = 1;
        }
    }

    while (array_len(ys->cmd_buff) > ys->cmd_prompt_compl_start_idx) {
        yed_cmd_buff_pop();
    }

    yed_append_text_to_cmd_buff(
          *(char**)array_item(*strings, ys->cmd_prompt_compl_item - 1)
        + ys->cmd_prompt_compl_string_len);

    if (ys->cmd_prompt_has_compl_results
    &&  ys->cmd_prompt_compl_item == 1
    &&  array_len(*strings) == 1) {
        yed_builtin_cmd_prompt_compl_cleanup();
    }
}

static void yed_builtin_cmd_prompt_do_compl_bwd(void) {
    array_t *strings;
    char    *compl_name;
    char    *compl_string;
    int      compl_status;

    if (ys->cmd_cursor_x != strlen(ys->cmd_prompt) + array_len(ys->cmd_buff) + 1) {
        return;
    }

    strings = &(ys->cmd_prompt_compl_results.strings);

    if (ys->cmd_prompt_compl_item == 0) {
        yed_builtin_cmd_prompt_compl_cleanup();

        compl_name   = yed_builtin_cmd_prompt_get_compl_name_from_context();
        compl_string = yed_builtin_cmd_prompt_get_compl_string_from_context();
        compl_status = yed_complete(compl_name, compl_string, &(ys->cmd_prompt_compl_results));

        if (compl_status != COMPL_ERR_NO_ERR
        ||  array_len(*strings) == 0) {
            return;
        }

        ys->cmd_prompt_has_compl_results = 1;
        ys->cmd_prompt_compl_item        = array_len(*strings);
        ys->cmd_prompt_compl_start_idx   = array_len(ys->cmd_buff);
        ys->cmd_prompt_compl_string_len  = strlen(compl_string);
    } else {
        ys->cmd_prompt_compl_item -= 1;
        if (ys->cmd_prompt_compl_item == 0) {
            ys->cmd_prompt_compl_item = array_len(*strings);
        }
    }

    while (array_len(ys->cmd_buff) > ys->cmd_prompt_compl_start_idx) {
        yed_cmd_buff_pop();
    }

    yed_append_text_to_cmd_buff(
          *(char**)array_item(*strings, ys->cmd_prompt_compl_item - 1)
        + ys->cmd_prompt_compl_string_len);

    if (ys->cmd_prompt_has_compl_results
    &&  ys->cmd_prompt_compl_item == 1
    &&  array_len(*strings) == 1) {
        yed_builtin_cmd_prompt_compl_cleanup();
    }
}

void yed_builtin_cmd_prompt_take_key(int key) {
    switch (key) {
        case ENTER:
            yed_builtin_cmd_prompt_run_cmd();
            break;
        case CTRL_C:
        case ESC:
            yed_builtin_cmd_prompt_cancel();
            break;
        case TAB:
            yed_builtin_cmd_prompt_do_compl_fwd();
            break;
        case SHIFT_TAB:
            yed_builtin_cmd_prompt_do_compl_bwd();
            break;
        default:
            yed_builtin_cmd_prompt_compl_cleanup();
            yed_cmd_line_readline_take_key(ys->cmd_prompt_readline, key);
    }
}
