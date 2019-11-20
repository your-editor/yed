#include "internal.h"

yed_undo_record yed_new_undo_record(void) {
    yed_undo_record ur;

    ur.actions = array_make(yed_undo_action);

    return ur;
}

yed_undo_history yed_new_undo_history(void) {
    yed_undo_history uh;

    uh.undo = array_make(yed_undo_record);
    uh.redo = array_make(yed_undo_record);

    return uh;
}

void yed_free_undo_action(yed_undo_action *action) {
    /* TODO */
}

void yed_free_undo_record(yed_undo_record *record) {
    yed_undo_action *action;

    array_traverse(record->actions, action) {
        yed_free_undo_action(action);
    }

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

int yed_num_undo_records(yed_buffer *buffer) {
    return array_len(buffer->undo_history.undo);
}

void yed_start_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record   record;

    history = &buffer->undo_history;

    record                  = yed_new_undo_record();
    record.start_cursor_row = frame->cursor_line;
    record.start_cursor_col = frame->cursor_col;

    array_push(history->undo, record);
}

void yed_end_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;

    history = &buffer->undo_history;

    record                 = array_last(history->undo);
    record->end_cursor_row = frame->cursor_line;
    record->end_cursor_col = frame->cursor_col;

    /*
     * We must clear the redo history here.
     * For now, we'll just clear the array, but
     * eventually there will be stuff that needs
     * to be freed and such.
     */
     array_clear(history->redo);
}

void yed_cancel_undo_record(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;

    history = &buffer->undo_history;
    record = array_last(history->undo);
    yed_free_undo_record(record);
    array_pop(history->undo);
}

int yed_push_undo_action(yed_buffer *buffer, yed_undo_action *action) {
    yed_undo_history *history;
    yed_undo_record  *record;

    history = &buffer->undo_history;
    record  = array_last(history->undo);

    if (!record)    { return 0; }

    array_push(record->actions, *action);

    return 1;
}

void yed_undo_single_action(yed_frame *frame, yed_buffer *buffer, yed_undo_action *action) {
    switch (action->kind) {
        case UNDO_CHAR_ADD:
            yed_delete_from_line_no_undo(buffer, action->row, action->col);
            break;

        case UNDO_CHAR_PUSH:
            yed_pop_from_line_no_undo(buffer, action->row);
            break;

        case UNDO_CHAR_DEL:
            yed_insert_into_line_no_undo(buffer, action->row, action->col, action->c);
            break;

        case UNDO_CHAR_POP:
            yed_append_to_line_no_undo(buffer, action->row, action->c);
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
        case UNDO_CHAR_ADD:
            yed_insert_into_line_no_undo(buffer, action->row, action->col, action->c);
            break;

        case UNDO_CHAR_PUSH:
            yed_append_to_line_no_undo(buffer, action->row, action->c);
            break;

        case UNDO_CHAR_DEL:
            yed_delete_from_line_no_undo(buffer, action->row, action->col);
            break;

        case UNDO_CHAR_POP:
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

    history = &buffer->undo_history;
    record  = array_last(history->undo);

    if (!record)    { return 0; }

    array_rtraverse(record->actions, action) {
        yed_undo_single_action(frame, buffer, action);
    }

    yed_set_cursor_within_frame(frame, record->start_cursor_col, record->start_cursor_row);

    array_push(history->redo, *record);
    array_pop(history->undo);

    frame->dirty = 1;

    return 1;
}

int yed_redo(yed_frame *frame, yed_buffer *buffer) {
    yed_undo_history *history;
    yed_undo_record  *record;
    yed_undo_action  *action;

    history = &buffer->undo_history;
    record  = array_last(history->redo);

    if (!record)    { return 0; }

    array_traverse(record->actions, action) {
        yed_redo_single_action(frame, buffer, action);
    }

    yed_set_cursor_within_frame(frame, record->end_cursor_col, record->end_cursor_row);

    array_push(history->undo, *record);
    array_pop(history->redo);

    frame->dirty = 1;

    return 1;
}
