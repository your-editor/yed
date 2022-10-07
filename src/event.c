void yed_init_events(void) {
    int i;

    for (i = 0; i < N_EVENTS; i += 1) {
        ys->event_handlers[i] = array_make(yed_event_handler);
    }

    yed_reload_default_event_handlers();
}

void yed_reload_default_event_handlers(void) {
    int               i;
    yed_event_handler h;

    for (i = 0; i < N_EVENTS; i += 1) {
        array_clear(ys->event_handlers[i]);
    }

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = yed_search_line_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_BUFFER_POST_MOD;
    h.fn   = yed_log_buff_mod_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_KEY_POST_BIND;
    h.fn = yed_key_bind_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_KEY_POST_UNBIND;
    h.fn = yed_key_bind_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_BUFFER_FOCUSED;
    h.fn   = yed_bindings_buffer_focus_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_VAR_POST_SET;
    h.fn = yed_var_change_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_VAR_POST_UNSET;
    h.fn = yed_var_change_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_BUFFER_FOCUSED;
    h.fn   = yed_var_buffer_focus_handler;
    yed_add_event_handler(h);

    h.kind = EVENT_CURSOR_PRE_MOVE;
    h.fn   = yed_replace_cursor_handler;
    yed_add_event_handler(h);
}

void yed_add_event_handler(yed_event_handler handler) {
    array_push(ys->event_handlers[handler.kind], handler);
}

void yed_delete_event_handler(yed_event_handler handler) {
    yed_event_handler *handler_it;
    int                i;

    i = 0;
    array_traverse(ys->event_handlers[handler.kind], handler_it) {
        if (handler_it->fn == handler.fn) {
            array_delete(ys->event_handlers[handler.kind], i);
            break;
        }
        i += 1;
    }
}

void yed_trigger_event(yed_event *event) {
    int                i;
    yed_event_handler *handler_it;
    yed_event_handler  handler;
    int                len_before;
    int                len_after;
    int                j;

    event->cancel = 0;

    i = array_len(ys->event_handlers[event->kind]) - 1;
    while (i >= 0) {
        handler_it = array_item(ys->event_handlers[event->kind], i);
        handler    = *handler_it;

        ASSERT(handler.kind == event->kind, "event/handler kind mismatch");

        len_before = array_len(ys->event_handlers[event->kind]);

        handler.fn(event);

        if (event->cancel) { break; }

        len_after = array_len(ys->event_handlers[event->kind]);

        if (len_after < len_before) {
            for (j = 0; j < len_after; j += 1) {
                handler_it = array_item(ys->event_handlers[event->kind], j);
                if (handler_it->fn == handler.fn) { i = j - 1; break; }
            }
        } else {
            i -= 1;
        }
    }
}
