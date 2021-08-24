#ifndef __EVENT_H__
#define __EVENT_H__


typedef enum {
    EVENT_FRAME_PRE_UPDATE,
    EVENT_FRAME_PRE_BUFF_DRAW,
    EVENT_FRAME_ACTIVATED,
    EVENT_FRAME_PRE_DELETE,
    EVENT_FRAME_PRE_SET_BUFFER,
    EVENT_FRAME_POST_SET_BUFFER,
    EVENT_ROW_PRE_CLEAR,
    EVENT_LINE_PRE_DRAW,
    EVENT_BUFFER_PRE_LOAD,
    EVENT_BUFFER_POST_LOAD,
    EVENT_BUFFER_PRE_DELETE,
    EVENT_BUFFER_PRE_SET_FT,
    EVENT_BUFFER_POST_SET_FT,
    EVENT_BUFFER_PRE_INSERT,
    EVENT_BUFFER_POST_INSERT,
    EVENT_BUFFER_PRE_DELETE_BACK,
    EVENT_BUFFER_POST_DELETE_BACK,
    EVENT_BUFFER_PRE_MOD,
    EVENT_BUFFER_POST_MOD,
    EVENT_BUFFER_PRE_WRITE,
    EVENT_BUFFER_POST_WRITE,
    EVENT_CURSOR_PRE_MOVE,
    EVENT_CURSOR_POST_MOVE,
    EVENT_KEY_PRESSED,
    EVENT_TERMINAL_RESIZED,
    EVENT_PRE_PUMP,
    EVENT_POST_PUMP,
    EVENT_STYLE_CHANGE,
    EVENT_PRE_QUIT,
    EVENT_PLUGIN_PRE_LOAD,
    EVENT_PLUGIN_POST_LOAD,
    EVENT_PLUGIN_PRE_UNLOAD,
    EVENT_PLUGIN_POST_UNLOAD,

    N_EVENTS,
} yed_event_kind_t;

typedef enum {
    BUFF_MOD_APPEND_TO_LINE,
    BUFF_MOD_POP_FROM_LINE,
    BUFF_MOD_CLEAR_LINE,
    BUFF_MOD_ADD_LINE,
    BUFF_MOD_SET_LINE,
    BUFF_MOD_INSERT_LINE,
    BUFF_MOD_DELETE_LINE,
    BUFF_MOD_INSERT_INTO_LINE,
    BUFF_MOD_DELETE_FROM_LINE,
    BUFF_MOD_CLEAR,

    N_BUFF_MOD_EVENTS,
} yed_buff_mod_event;

typedef struct {
    yed_event_kind_t  kind;
    yed_frame        *frame;
    yed_buffer       *buffer;
    union { int       row, new_row; };
    union { int       col, new_col; };
    yed_attrs         row_base_attr;
    array_t           line_attrs;
    array_t           gutter_glyphs;
    array_t           gutter_attrs;
    int               key;
    char             *glyph;
    int               cancel;
    char             *path;
    int               buffer_is_new_file;
    int               buff_mod_event;
    const char       *plugin_name;
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
