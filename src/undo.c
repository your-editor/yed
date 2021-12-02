yed_undo_record yed_new_undo_record(void) {
    yed_undo_record ur;

    ur.actions = array_make(yed_undo_action);

    return ur;
}

yed_undo_history yed_new_undo_history(void) {
    yed_undo_history uh;

    uh.undo = array_make(yed_undo_record);
    uh.redo = array_make(yed_undo_record);

    uh.current_record    = NULL;

    return uh;
}

void yed_free_undo_record(yed_undo_record *record) {
    array_free(record->actions);
}

void yed_free_undo_history(yed_undo_history *history) {
    yed_undo_record *record;

    array_traverse(history->undo, record) {
        yed_free_undo_record(record);
    }

    array_free(history->undo);

    array_traverse(history->redo, record) {
        yed_free_undo_record(record);
    }

    array_free(history->redo);
}

void yed_reset_undo_history(yed_undo_history *history) {
    yed_free_undo_history(history);
    *history = yed_new_undo_history();
}

int yed_num_undo_records(yed_buffer *buffer) {
    return array_len(buffer->undo_history.undo);
}

void yed_force_end_undo_record(yed_undo_history *history) {
    yed_undo_record *record;

    record = history->current_record;

    ASSERT(record, "no current undo record");

    record->end_cursor_row = record->start_cursor_row;
    record->end_cursor_col = record->start_cursor_col;

    /*
     * We must clear the redo history here.
     * For now, we'll just clear the array, but
     * eventually there will be stuff that needs
     * to be freed and such.
     */
    array_clear(history->redo);

    history->current_record = NULL;
}

void yed_start_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record   record;
    int               merge;

    if (buffer->kind == BUFF_KIND_YANK)    { return; }

    history = &buffer->undo_history;
    merge   = 0;

    if (history->current_record) {
        /*
         * The current record has been interrupted.
         * Possibly, someone forgot to end their undo record.
         * What we'll do is force the current one to end here.
         * This will clump all of the actions together whether they
         * were meant to be or not, but it will help us re-sync the
         * undo history.
         */

         /*
          * UPDATE: Seems like the best thing to do is to merge them.
          */

        yed_force_end_undo_record(history);
        merge = 1;
    }

    record = yed_new_undo_record();

    if (frame) {
        record.start_cursor_row = frame->cursor_line;
        record.start_cursor_col = frame->cursor_col;
    } else {
        record.start_cursor_row = 1;
        record.start_cursor_col = 1;
    }

    history->current_record = array_push(history->undo, record);

    if (merge)    { yed_merge_undo_records(buffer); }
}

void yed_end_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;

    if (buffer->kind == BUFF_KIND_YANK)    { return; }

    history = &buffer->undo_history;
    record  = history->current_record;

    ASSERT(record, "no current undo record");

    if (frame) {
        record->end_cursor_row = frame->cursor_line;
        record->end_cursor_col = frame->cursor_col;
    } else {
        record->end_cursor_row = 1;
        record->end_cursor_col = 1;
    }

    /*
     * We must clear the redo history here.
     * For now, we'll just clear the array, but
     * eventually there will be stuff that needs
     * to be freed and such.
     */
    array_clear(history->redo);

    history->current_record    = NULL;
}

void yed_cancel_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;

    if (buffer->kind == BUFF_KIND_YANK)    { return; }

    history = &buffer->undo_history;
    record  = history->current_record;

    ASSERT(record, "no current undo record");

    yed_free_undo_record(record);
    array_pop(history->undo);

    history->current_record = NULL;
}

int yed_get_undo_num_records(struct yed_buffer_t *buffer) {
    yed_undo_history *history;

    if (buffer->kind == BUFF_KIND_YANK)    { return -1; }

    history = &buffer->undo_history;

    return array_len(history->undo);
}

void yed_merge_undo_records(struct yed_buffer_t *buffer) {
    yed_undo_history *history;
    yed_undo_record  *last_record,
                     *new_last_record;

    if (buffer->kind == BUFF_KIND_YANK)    { return; }

    history = &buffer->undo_history;

    if (array_len(history->undo) < 2)    { return; }

    last_record     = array_last(history->undo);
    new_last_record = array_item(history->undo, array_len(history->undo) - 2);

    array_push_n(new_last_record->actions,
                 array_data(last_record->actions),
                 array_len(last_record->actions));
/*     array_traverse(last_record->actions, action) { */
/*         array_push(new_last_record->actions, *action); */
/*     } */

    new_last_record->end_cursor_row = last_record->end_cursor_row;
    new_last_record->end_cursor_col = last_record->end_cursor_col;

    yed_free_undo_record(last_record);
    array_pop(history->undo);

    new_last_record = array_last(history->undo);

    if (history->current_record) {
        ASSERT(history->current_record == last_record, "undo history messed up");
        history->current_record = new_last_record;
    }
}

int yed_push_undo_action(yed_buffer *buffer, yed_undo_action *action) {
    yed_undo_history *history;
    yed_undo_record  *record;

    if (buffer->kind == BUFF_KIND_YANK)    { return 0; }

    history = &buffer->undo_history;
    record  = array_last(history->undo);

    if (!record) {
        yed_start_undo_record(NULL, buffer);
        record = array_last(history->undo);
    }

    array_push(record->actions, *action);

    return 1;
}

void yed_undo_single_action(yed_frame *frame, yed_buffer *buffer, yed_undo_action *action) {
    switch (action->kind) {
        case UNDO_GLYPH_ADD:
            yed_delete_from_line_no_undo(buffer, action->row, action->col);
            break;

        case UNDO_GLYPH_PUSH:
            yed_pop_from_line_no_undo(buffer, action->row);
            break;

        case UNDO_GLYPH_DEL:
            yed_insert_into_line_no_undo(buffer, action->row, action->col, action->g);
            break;

        case UNDO_GLYPH_POP:
            yed_append_to_line_no_undo(buffer, action->row, action->g);
            break;

        case UNDO_LINE_ADD:
            yed_buff_delete_line_no_undo(buffer, action->row);
            break;

        case UNDO_LINE_DEL:
            yed_buff_insert_line_no_undo(buffer, action->row);
            break;

        default:
            ASSERT(0, "unhandled undo action kind");
    }
}

void yed_redo_single_action(yed_frame *frame, yed_buffer *buffer, yed_undo_action *action) {
    switch (action->kind) {
        case UNDO_GLYPH_ADD:
            yed_insert_into_line_no_undo(buffer, action->row, action->col, action->g);
            break;

        case UNDO_GLYPH_PUSH:
            yed_append_to_line_no_undo(buffer, action->row, action->g);
            break;

        case UNDO_GLYPH_DEL:
            yed_delete_from_line_no_undo(buffer, action->row, action->col);
            break;

        case UNDO_GLYPH_POP:
            yed_pop_from_line_no_undo(buffer, action->row);
            break;

        case UNDO_LINE_ADD:
            yed_buff_insert_line_no_undo(buffer, action->row);
            break;

        case UNDO_LINE_DEL:
            yed_buff_delete_line_no_undo(buffer, action->row);
            break;

        default:
            ASSERT(0, "unhandled undo action kind");
    }
}

int yed_undo(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;
    yed_undo_action  *action;

    if (buffer->kind == BUFF_KIND_YANK)    { return 0; }

    history = &buffer->undo_history;
    record  = array_last(history->undo);

    if (!record)    { return 0; }

    array_rtraverse(record->actions, action) {
        yed_undo_single_action(frame, buffer, action);
    }

    yed_set_cursor_within_frame(frame, record->start_cursor_row, record->start_cursor_col);

    array_push(history->redo, *record);
    array_pop(history->undo);

    frame->dirty = 1;

    return 1;
}

int yed_redo(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;
    yed_undo_action  *action;

    if (buffer->kind == BUFF_KIND_YANK)    { return 0; }

    history = &buffer->undo_history;
    record  = array_last(history->redo);

    if (!record)    { return 0; }

    array_traverse(record->actions, action) {
        yed_redo_single_action(frame, buffer, action);
    }

    yed_set_cursor_within_frame(frame, record->end_cursor_row, record->end_cursor_col);

    array_push(history->undo, *record);
    array_pop(history->redo);

    frame->dirty = 1;

    return 1;
}
