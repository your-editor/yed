#include <yed/plugin.h>

void completer(int n_args, char **args);
void completer_buff_pre_insert_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_BUFFER_PRE_INSERT;
    h.fn   = completer_buff_pre_insert_handler;

    yed_plugin_set_command(self, "completer", completer);

    yed_plugin_add_event_handler(self, h);

    return 0;
}

void completer(int n_args, char **args) {
    yed_frame              *frame;
    int                     save_col;
    char                   *word, key_str[32];
    int                     i, word_len, cpl;
    int                     compl_status;
    int                     compl_num_items;
    yed_completion_results  results;
    char                   *first;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    save_col = frame->cursor_col;
    yed_move_cursor_within_frame(frame, 0, -1);
    word = yed_word_under_cursor();
    yed_set_cursor_within_frame(frame, frame->cursor_line, save_col);

    if (!word)    { return; }

    word_len = strlen(word);

    compl_status    = yed_complete("word", word, &results);
    compl_num_items = array_len(results.strings);
    cpl             = results.common_prefix_len;

    if (compl_status == COMPL_ERR_NO_ERR && compl_num_items > 0) {
        first = *(char**)array_item(results.strings, 0);
        for (i = word_len; i < cpl; i += 1) {
            sprintf(key_str, "%d", first[i]);
            YEXE("insert", key_str);
        }

        if (compl_num_items > 1) {
            yed_cerr("%d words with same prefix", compl_num_items);
        }

        free_string_array(results.strings);
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
        c = ((yed_glyph*)yed_line_col_to_glyph(line, event->col))->c;
    }

    prev_c = ((yed_glyph*)yed_line_col_to_glyph(line, event->col - 1))->c;

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
