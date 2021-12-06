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
    EVENT_BUFFER_FOCUSED,
    EVENT_CURSOR_PRE_MOVE,
    EVENT_CURSOR_POST_MOVE,
    EVENT_KEY_PRESSED,
    EVENT_KEY_PRE_BIND,
    EVENT_KEY_POST_BIND,
    EVENT_KEY_PRE_UNBIND,
    EVENT_KEY_POST_UNBIND,
    EVENT_TERMINAL_RESIZED,
    EVENT_PRE_PUMP,
    EVENT_POST_PUMP,
    EVENT_STYLE_CHANGE,
    EVENT_PRE_QUIT,
    EVENT_PLUGIN_PRE_LOAD,
    EVENT_PLUGIN_POST_LOAD,
    EVENT_PLUGIN_PRE_UNLOAD,
    EVENT_PLUGIN_POST_UNLOAD,
    EVENT_CMD_PRE_RUN,
    EVENT_CMD_POST_RUN,
    EVENT_VAR_PRE_SET,
    EVENT_VAR_POST_SET,
    EVENT_VAR_PRE_UNSET,
    EVENT_VAR_POST_UNSET,
    EVENT_STATUS_LINE_PRE_UPDATE,
    EVENT_PLUGIN_MESSAGE,
    _EVENT_RESERVED_0,
    _EVENT_RESERVED_1,
    _EVENT_RESERVED_2,
    _EVENT_RESERVED_3,
    _EVENT_RESERVED_4,
    _EVENT_RESERVED_5,
    _EVENT_RESERVED_6,
    _EVENT_RESERVED_7,

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
    yed_event_kind_t            kind;
    yed_frame                  *frame;
    yed_buffer                 *buffer;
    union { int                 row;
            int                 new_row; };
    union { int                 col;
            int                 new_col; };
    yed_attrs                   row_base_attr;
    array_t                     line_attrs;
    array_t                     gutter_glyphs;
    array_t                     gutter_attrs;
    int                         key;
    char                       *glyph;
    int                         cancel;
    char                       *path;
    int                         buffer_is_new_file;
    int                         buff_mod_event;
    union { const char         *plugin_name;
            const char         *cmd_name;
            const char         *var_name; };
    int                         n_args;
    union { const char * const *args;
            const char         *var_val; };
    struct {
        const char             *message_id;
        const char             *plugin_id;
        union { const char     *string_data;
                void           *v_data; };
    } plugin_message;
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
