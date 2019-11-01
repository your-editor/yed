#ifndef __EVENT_H__
#define __EVENT_H__

#include "internal.h"

typedef enum {
    EVENT_FRAME_PRE_UPDATE,
    EVENT_FRAME_PRE_BUFF_DRAW,
    EVENT_LINE_PRE_DRAW,
    EVENT_BUFFER_PRE_INSERT,
    EVENT_BUFFER_POST_INSERT,
    EVENT_BUFFER_PRE_DELETE_BACK,
    EVENT_BUFFER_POST_DELETE_BACK,
    EVENT_BUFFER_PRE_MOD,
    EVENT_BUFFER_POST_MOD,
    EVENT_BUFFER_PRE_WRITE,
    EVENT_FRAME_ACTIVATED,

    N_EVENTS,
} yed_event_kind_t;

typedef struct {
    yed_event_kind_t  kind;
    yed_frame        *frame;
    yed_buffer       *buffer;
    int               row, col;
    array_t           line_attrs;
    int               key;
} yed_event;

typedef void (*yed_event_handler_fn_t)(yed_event*);

typedef struct {
    yed_event_kind_t       kind;
    yed_event_handler_fn_t fn;
} yed_event_handler;

void yed_init_events(void);
void yed_reload_default_event_handlers(void);
void yed_add_event_handler(yed_event_handler handler);
void yed_delete_event_handler(yed_event_handler handler);

void yed_trigger_event(yed_event *event);

#endif
