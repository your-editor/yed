#include <yed/plugin.h>

void style_picker(int n_args, char **args);
void style_picker_cleanup(void);
void style_picker_cleanup_no_frame_del(void);
void style_picker_make_frame(void);
void style_picker_make_buffer(void);
void style_picker_select(void);
void style_picker_commit(void);
void style_picker_abort(void);

void style_picker_key_pressed_handler(yed_event *event);
void style_picker_cursor_moved_handler(yed_event *event);
void style_picker_del_handler(yed_event *event);

static yed_frame  *frame;
static yed_buffer *buff;
static char       *save;

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler key, move, del;

    YED_PLUG_VERSION_CHECK();

    key.kind  = EVENT_KEY_PRESSED;
    key.fn    = style_picker_key_pressed_handler;

    move.kind = EVENT_CURSOR_MOVED;
    move.fn   = style_picker_cursor_moved_handler;

    del.kind  = EVENT_FRAME_PRE_DELETE;
    del.fn    = style_picker_del_handler;

    yed_plugin_add_event_handler(self, key);
    yed_plugin_add_event_handler(self, move);
    yed_plugin_add_event_handler(self, del);
    yed_plugin_set_command(self, "style-picker", style_picker);

    return 0;
}

void style_picker(int n_args, char **args) {
    if (ys->active_style) {
        save = strdup(ys->active_style->_name);
    }
    style_picker_make_frame();
    style_picker_make_buffer();
    yed_frame_set_buff(frame, buff);
}

void style_picker_cleanup(void) {
    yed_frame  *the_frame;
    yed_buffer *the_buff;

    /* Do this so that our handlers don't do anything. */
    the_frame = frame;
    the_buff  = buff;
    frame     = NULL;
    buff      = NULL;

    yed_frame_set_buff(the_frame, NULL);
    yed_free_buffer(the_buff);
    yed_delete_frame(the_frame);

    if (save) {
        free(save);
    }
}

void style_picker_cleanup_no_frame_del(void) {
    yed_frame  *the_frame;
    yed_buffer *the_buff;

    /* Do this so that our handlers don't do anything. */
    the_frame = frame;
    the_buff  = buff;
    frame     = NULL;
    buff      = NULL;

    yed_frame_set_buff(the_frame, NULL);
    yed_free_buffer(the_buff);

    if (save) {
        free(save);
    }
}

void style_picker_make_buffer(void) {
    tree_it(yed_style_name_t, yed_style_ptr_t)  it;
    int                                         i, row;
    char                                       *style;

    buff = yed_get_buffer("*style-picker-list");

    if (!buff) {
        buff = yed_create_buffer("*style-picker-list");
        buff->flags |= BUFF_RD_ONLY;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    ASSERT(buff, "did not create '*style-picker-list' buffer");

    tree_traverse(ys->styles, it) {
        row   = yed_buff_n_lines(buff);
        style = tree_it_key(it);
        for (i = 0; i < strlen(style); i += 1) {
            yed_append_to_line_no_undo(buff, row, G(style[i]));
        }
        yed_buffer_add_line_no_undo(buff);
    }

    yed_buff_delete_line_no_undo(buff, yed_buff_n_lines(buff));

}

void style_picker_make_frame(void) {
    frame = yed_add_new_frame(0.0, 0.15, 0.15, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);
}

void style_picker_select(void) {
    yed_line *line;
    char     *style;

    line = yed_buff_get_line(buff, frame->cursor_line);
    array_zero_term(line->chars);
    style = strdup(array_data(line->chars));

    if (ys->active_style && strcmp(style, ys->active_style->_name)) {
        yed_activate_style(style);
    }

    free(style);
}

void style_picker_commit(void) {
    style_picker_cleanup();

    if (!ys->active_frame) {
        YEXE("frame-new");
    }
}

void style_picker_abort(void) {
    yed_activate_style(save);

    style_picker_cleanup_no_frame_del();

    if (!ys->active_frame) {
        YEXE("frame-new");
    }
}

void style_picker_key_pressed_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (event->key != ENTER                                   /* not the key we want */
    ||  ys->interactive_command                               /* enter in command line */
    ||  !eframe                                               /* no frame */
    ||  eframe != frame                                       /* not our frame */
    ||  !eframe->buffer                                       /* no buffer */
    ||  strcmp(eframe->buffer->name, "*style-picker-list")) { /* not our buffer */
        return;
    }

    style_picker_commit();
}

void style_picker_cursor_moved_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (!eframe                                               /* no frame */
    ||  eframe != frame                                       /* not our frame */
    ||  !eframe->buffer                                       /* no buffer */
    ||  strcmp(eframe->buffer->name, "*style-picker-list")) { /* not our buffer */
        return;
    }

    style_picker_select();
}

void style_picker_del_handler(yed_event *event) {
    yed_frame *eframe;

    eframe = ys->active_frame;

    if (!eframe                                               /* no frame */
    ||  eframe != frame                                       /* not our frame */
    ||  !eframe->buffer                                       /* no buffer */
    ||  strcmp(eframe->buffer->name, "*style-picker-list")) { /* not our buffer */
        return;
    }

    style_picker_abort();
}
