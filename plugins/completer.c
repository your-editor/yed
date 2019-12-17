#include <yed/plugin.h>

void completer(int n_args, char **args);
void completer_buff_pre_insert_handler(yed_event *event);

static int    compl_num_items = 0;
static char **compl_items     = NULL;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    h.kind = EVENT_BUFFER_PRE_INSERT;
    h.fn   = completer_buff_pre_insert_handler;

    yed_plugin_set_command(self, "completer", completer);

    yed_plugin_add_event_handler(self, h);

    return 0;
}

void completer(int n_args, char **args) {
    yed_frame *frame;
    int        save_col;
    char      *word, key_str[32];
    int        i, word_len, cpl;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame ");
        return;
    }

    frame = ys->active_frame;

    save_col = frame->cursor_col;
    yed_move_cursor_within_frame(frame, -1, 0);
    word = yed_word_under_cursor();
    yed_set_cursor_within_frame(frame, save_col, frame->cursor_line);

    if (!word)    { return; }

    word_len = strlen(word);

    compl_num_items =
        yed_get_completion(COMPL_BUFF, word, &compl_items, &cpl);

    if (compl_num_items) {
        compl_items[0][cpl] = 0;
        for (i = word_len; i < cpl; i += 1) {
            sprintf(key_str, "%d", compl_items[0][i]);
            YEXE("insert", key_str);
        }

        if (compl_num_items > 1) {
            yed_append_int_to_cmd_buff(compl_num_items);
            yed_append_text_to_cmd_buff(" words with same prefix");
        }

        /* Cleanup */
        for (i = 0; i < compl_num_items; i += 1) {
            free(compl_items[i]);
        }
        free(compl_items);
    }

    free(word);
}

void completer_buff_pre_insert_handler(yed_event *event) {
    char      c, prev_c;
    yed_line *line;

    if (event->key != TAB)    { return; }
    if (event->col == 1)      { return; }

    line = yed_buff_get_line(event->frame->buffer, event->row);

    if (event->col == line->visual_width + 1) {
        c = 0;
    } else {
        c = yed_line_col_to_char(line, event->col);
    }

    prev_c = yed_line_col_to_char(line, event->col - 1);

    /* There must be part of a word behind the cursor. */
    if (!isalnum(prev_c) && prev_c != '_') {
        return;
    }

    /* The cursor must NOT be on part of a word. */
    if (isalnum(c) || c == '_') {
        return;
    }

    event->cancel = 1;

    YEXE("completer");
}
