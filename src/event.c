#include "internal.h"

void yed_init_events(void) {
    int i;

    for (i = 0; i < N_EVENTS; i += 1) {
        ys->event_handlers[i] = array_make(yed_event_handler);
    }
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
    yed_event_handler *handler_it;

    array_traverse(ys->event_handlers[event->kind], handler_it) {
        ASSERT(handler_it->kind == event->kind, "event/handler kind mismatch");

        handler_it->fn(event);
    }
}
