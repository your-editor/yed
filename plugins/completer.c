#include <yed/plugin.h>

typedef struct {
    yed_frame *frame;
    array_t    strings;
    array_t    dds;
    int        start_len;
    int        start_x;
    int        is_up;
    int        selection;
} completer_popup_t;

static completer_popup_t popup;

char *completer_get_current_word(void);
void completer_buff_pre_insert_handler(yed_event *event);
void completer_buff_post_insert_handler(yed_event *event);
void completer_buff_post_delete_back_handler(yed_event *event);
void completer_cursor_post_move_handler(yed_event *event);
void completer_frame_activated_handler(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    if (yed_get_var("completer-popup") == NULL) {
        yed_set_var("completer-popup", "yes");
    }

    if (yed_get_var("completer-auto") == NULL) {
        yed_set_var("completer-auto", "no");
    }

    if (yed_get_var("completer-popup-size") == NULL) {
        yed_set_var("completer-popup-size", "8");
    }


    h.kind = EVENT_BUFFER_PRE_INSERT;
    h.fn   = completer_buff_pre_insert_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_BUFFER_POST_INSERT;
    h.fn   = completer_buff_post_insert_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_BUFFER_POST_DELETE_BACK;
    h.fn   = completer_buff_post_delete_back_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_CURSOR_POST_MOVE;
    h.fn   = completer_cursor_post_move_handler;
    yed_plugin_add_event_handler(self, h);

    h.kind = EVENT_FRAME_ACTIVATED;
    h.fn   = completer_frame_activated_handler;
    yed_plugin_add_event_handler(self, h);

    return 0;
}

static void completer_kill_popup(void) {
    yed_direct_draw_t **dd;

    if (!popup.is_up) { return; }

    free_string_array(popup.strings);

    array_traverse(popup.dds, dd) {
        yed_kill_direct_draw(*dd);
    }

    array_free(popup.dds);

    popup.frame = NULL;

    popup.is_up = 0;
}

static void completer_draw_popup(void) {
    yed_direct_draw_t **dd_it;
    yed_attrs           active;
    yed_attrs           assoc;
    yed_attrs           merged;
    yed_attrs           merged_inv;
    char              **it;
    int                 max_width;
    int                 has_left_space;
    int                 size;
    int                 start_idx;
    int                 i;
    char                buff[512];
    yed_direct_draw_t  *dd;

    array_traverse(popup.dds, dd_it) {
        yed_kill_direct_draw(*dd_it);
    }
    array_free(popup.dds);

    popup.dds = array_make(yed_direct_draw_t*);

    active = yed_active_style_get_active();
    assoc  = yed_active_style_get_associate();
    merged = active;
    yed_combine_attrs(&merged, &assoc);
    merged_inv = merged;
    merged_inv.flags ^= ATTR_INVERSE;

    max_width = strlen("--MORE--");
    array_traverse(popup.strings, it) {
        max_width = MAX(max_width, strlen(*it));
    }

    has_left_space = popup.start_x > 1;

    size = 8;
    yed_get_var_as_int("completer-popup-size", &size);

    start_idx = MAX(0, popup.selection - size + 1);
    i         = 0;

    array_traverse_from(popup.strings, it, start_idx) {
        snprintf(buff, sizeof(buff), "%s%*s ", has_left_space ? " " : "", -max_width, *it);
        dd = yed_direct_draw(popup.frame->cur_y + i + 1,
                             popup.start_x - has_left_space,
                             start_idx + i == popup.selection ? merged_inv : merged,
                             buff);
        array_push(popup.dds, dd);

        i += 1;
        if (i == size) {
            if (start_idx + i == array_len(popup.strings)) {
                snprintf(buff, sizeof(buff), "%s%*s ", has_left_space ? " " : "", -max_width, "--END--");
            } else {
                snprintf(buff, sizeof(buff), "%s%*s ", has_left_space ? " " : "", -max_width, "--MORE--");
            }
            dd = yed_direct_draw(popup.frame->cur_y + i + 1,
                                 popup.start_x - has_left_space,
                                 merged,
                                 buff);
            array_push(popup.dds, dd);
            break;
        }
    }
}

static void completer_start_popup(yed_frame *frame, int start_len, array_t strings) {
    completer_kill_popup();

    popup.frame     = frame;
    popup.strings   = copy_string_array(strings);
    popup.dds       = array_make(yed_direct_draw_t*);
    popup.start_len = start_len;
    popup.start_x   = frame->cur_x - start_len;
    popup.selection = -1;

    completer_draw_popup();

    popup.is_up = 1;
}

char *completer_get_current_word(void) {
    yed_frame *frame;
    yed_line  *line;
    char       c;
    char       prev_c;

    frame = ys->active_frame;

    if (frame == NULL || frame->buffer == NULL) { return NULL; }

    line = yed_buff_get_line(frame->buffer, frame->cursor_line);
    if (line == NULL) { return NULL; }

    if (frame->cursor_col <= 1) { return NULL; }

    c = prev_c = 0;

    if (frame->cursor_col <= line->visual_width) {
        c = ((yed_glyph*)yed_line_col_to_glyph(line, frame->cursor_col))->c;
    }

    prev_c = ((yed_glyph*)yed_line_col_to_glyph(line, frame->cursor_col - 1))->c;

    /* There must be part of a word behind the cursor. */
    if (!isalnum(prev_c) && prev_c != '_') { return NULL; }

    /* The cursor must NOT be on part of a word. */
    if (isalnum(c) || c == '_') { return NULL; }

    return yed_word_at_point(frame, frame->cursor_line, frame->cursor_col - 1);
}

int completer_cmd_line(void) {
    yed_frame               *frame;
    yed_buffer              *buff;
    char                    *word;
    int                      word_len;
    int                      compl_status;
    yed_completion_results   results;
    int                      compl_num_items;
    int                      cpl;
    char                    *first;
    int                      i;
    char                   **it;

    frame = ys->active_frame;
    if (frame == NULL) { return 0; }

    buff = frame->buffer;
    if (buff == NULL) { return 0; }

    word = completer_get_current_word();
    if (word == NULL) { return 0; }

    word_len = strlen(word);

    compl_status    = yed_complete("word", word, &results);
    compl_num_items = array_len(results.strings);
    cpl             = results.common_prefix_len;

    if (compl_status == COMPL_ERR_NO_ERR && compl_num_items > 0) {
        first = *(char**)array_item(results.strings, 0);

        yed_start_undo_record(frame, buff);
        for (i = word_len; i < cpl; i += 1) {
            yed_insert_into_line(buff, frame->cursor_line, frame->cursor_col, G(first[i]));
            yed_move_cursor_within_frame(frame, 0, 1);
        }
        yed_end_undo_record(frame, buff);

        if (compl_num_items > 1) {
LOG_CMD_ENTER("completer");
            yed_cprint("");
            array_traverse(results.strings, it) {
                yed_cprint("\n%s", *it);
            }
LOG_EXIT();
        }

        free_string_array(results.strings);
    }

    free(word);

    return 1;
}

static int completer_no_auto_popup(void) {
    yed_frame               *frame;
    yed_buffer              *buff;
    char                    *word;
    int                      word_len;
    int                      compl_status;
    yed_completion_results   results;
    int                      compl_num_items;
    int                      cpl;
    char                    *first;
    int                      i;

    frame = ys->active_frame;
    if (frame == NULL) { return 0; }

    buff = frame->buffer;
    if (buff == NULL) { return 0; }

    word = completer_get_current_word();
    if (word == NULL) { return 0; }

    word_len = strlen(word);

    compl_status    = yed_complete("word", word, &results);
    compl_num_items = array_len(results.strings);
    cpl             = results.common_prefix_len;

    if (compl_status == COMPL_ERR_NO_ERR && compl_num_items > 0) {
        first = *(char**)array_item(results.strings, 0);

        yed_start_undo_record(frame, buff);
        for (i = word_len; i < cpl; i += 1) {
            yed_insert_into_line(buff, frame->cursor_line, frame->cursor_col, G(first[i]));
            yed_move_cursor_within_frame(frame, 0, 1);
        }
        yed_end_undo_record(frame, buff);

        if (compl_num_items > 1) {
            completer_start_popup(frame, cpl, results.strings);
        }

        free_string_array(results.strings);
    }

    free(word);

    return 1;
}

static int completer_auto_popup(void) {
    yed_frame               *frame;
    yed_buffer              *buff;
    char                    *word;
    int                      word_len;
    int                      compl_status;
    yed_completion_results   results;
    int                      compl_num_items;

    frame = ys->active_frame;
    if (frame == NULL) { return 0; }

    buff = frame->buffer;
    if (buff == NULL) { return 0; }

    word = completer_get_current_word();
    if (word == NULL) { return 0; }

    word_len = strlen(word);

    compl_status    = yed_complete("word", word, &results);
    compl_num_items = array_len(results.strings);

    if (compl_status == COMPL_ERR_NO_ERR && compl_num_items > 0) {
        completer_start_popup(frame, word_len, results.strings);
        free_string_array(results.strings);
    }

    free(word);

    return 1;
}

static void completer_next_selection(void) {
    yed_frame *frame;
    char      *sel_string;
    int        i;

    frame = popup.frame;

    yed_start_undo_record(frame, frame->buffer);
    if (popup.selection > -1) {
        sel_string = *(char**)array_item(popup.strings, popup.selection);
        for (i = 0; i < strlen(sel_string) - popup.start_len; i += 1) {
            yed_move_cursor_within_frame(frame, 0, -1);
            yed_delete_from_line(frame->buffer, frame->cursor_line, frame->cursor_col);
        }
    }

    popup.selection += 1;
    if (popup.selection == array_len(popup.strings)) {
        popup.selection = -1;
    }

    if (popup.selection > -1) {
        sel_string = *(char**)array_item(popup.strings, popup.selection);
        for (i = popup.start_len; i < strlen(sel_string); i += 1) {
            yed_insert_into_line(frame->buffer, frame->cursor_line, frame->cursor_col, G(sel_string[i]));
            yed_move_cursor_within_frame(frame, 0, 1);
        }
    }

    yed_end_undo_record(frame, frame->buffer);

    completer_draw_popup();
}

static void completer_prev_selection(void) {
    yed_frame *frame;
    char      *sel_string;
    int        i;

    frame = popup.frame;

    yed_start_undo_record(frame, frame->buffer);
    if (popup.selection > -1) {
        sel_string = *(char**)array_item(popup.strings, popup.selection);
        for (i = 0; i < strlen(sel_string) - popup.start_len; i += 1) {
            yed_move_cursor_within_frame(frame, 0, -1);
            yed_delete_from_line(frame->buffer, frame->cursor_line, frame->cursor_col);
        }
    }

    popup.selection -= 1;
    if (popup.selection == -2) {
        popup.selection = array_len(popup.strings) - 1;
    }

    if (popup.selection > -1) {
        sel_string = *(char**)array_item(popup.strings, popup.selection);
        for (i = popup.start_len; i < strlen(sel_string); i += 1) {
            yed_insert_into_line(frame->buffer, frame->cursor_line, frame->cursor_col, G(sel_string[i]));
            yed_move_cursor_within_frame(frame, 0, 1);
        }
    }

    yed_end_undo_record(frame, frame->buffer);

    completer_draw_popup();
}

static int in_pre_insert_handler;
static int in_post_insert_handler;

void completer_buff_pre_insert_handler(yed_event *event) {
    int did_compl;

    if (in_pre_insert_handler || in_post_insert_handler) { return; }

    if (event->key == ENTER) {
        if (popup.is_up && popup.selection > -1) {
            completer_kill_popup();
            event->cancel = 1;
            return;
        }
    }

    if (event->key != TAB && event->key != SHIFT_TAB) { return; }

    in_pre_insert_handler = 1;

    did_compl = 0;

    if (yed_var_is_truthy("completer-popup")) {
        if (popup.is_up) {
            if (event->key == TAB) {
                completer_next_selection();
            } else if (event->key == SHIFT_TAB) {
                completer_prev_selection();
            }
            did_compl = 1;
        } else if (yed_var_is_truthy("completer-auto")) {
            did_compl = completer_auto_popup();
        } else {
            did_compl = completer_no_auto_popup();
        }
    } else {
        did_compl = completer_cmd_line();
    }

    if (did_compl) {
        event->cancel = 1;
    }

    in_pre_insert_handler = 0;
}

void completer_buff_post_insert_handler(yed_event *event) {
    if (!yed_var_is_truthy("completer-auto")
    ||  !yed_var_is_truthy("completer-popup"))           { return; }
    if (in_pre_insert_handler || in_post_insert_handler) { return; }

    in_post_insert_handler = 1;

    completer_kill_popup();
    completer_auto_popup();

    in_post_insert_handler = 0;
}

void completer_buff_post_delete_back_handler(yed_event *event) {
    if (!yed_var_is_truthy("completer-auto")
    ||  !yed_var_is_truthy("completer-popup"))           { return; }
    if (in_pre_insert_handler || in_post_insert_handler) { return; }

    in_post_insert_handler = 1;

    completer_kill_popup();
    completer_auto_popup();

    in_post_insert_handler = 0;
}

void completer_cursor_post_move_handler(yed_event *event) {
    if (in_pre_insert_handler || in_post_insert_handler) { return; }

    if (popup.is_up && event->frame == popup.frame) {
        completer_kill_popup();
    }
}

void completer_frame_activated_handler(yed_event *event) {
    if (in_pre_insert_handler || in_post_insert_handler) { return; }

    if (popup.is_up) {
        completer_kill_popup();
    }
}
