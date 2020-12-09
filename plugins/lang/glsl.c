#include <yed/plugin.h>

void unload(yed_plugin *self);
void maybe_change_ft(yed_buffer *buff);
void maybe_change_ft_event(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t) bit;
    yed_event_handler                            buff_post_load_handler;
    yed_event_handler                            buff_pre_write_handler;

    YED_PLUG_VERSION_CHECK();

LOG_FN_ENTER();
    yed_plugin_set_unload_fn(self, unload);

    if (yed_plugin_make_ft(self, "GLSL") == FT_ERR_TAKEN) {
        yed_cerr("lang/glsl: unable to create file type name");
        LOG_EXIT();
        return 1;
    }

    buff_post_load_handler.kind = EVENT_BUFFER_POST_LOAD;
    buff_post_load_handler.fn   = maybe_change_ft_event;
    buff_pre_write_handler.kind = EVENT_BUFFER_PRE_WRITE;
    buff_pre_write_handler.fn   = maybe_change_ft_event;

    yed_plugin_add_event_handler(self, buff_post_load_handler);
    yed_plugin_add_event_handler(self, buff_pre_write_handler);

    tree_traverse(ys->buffers, bit) {
        maybe_change_ft(tree_it_val(bit));
    }

    YEXE("plugin-load", "lang/syntax/glsl");

LOG_EXIT();
    return 0;
}

void unload(yed_plugin *self) {
    YEXE("plugin-unload", "lang/syntax/glsl");
}

void maybe_change_ft(yed_buffer *buff) {
    char *ext;

    if (buff->ft != FT_UNKNOWN) {
        return;
    }
    if (buff->path == NULL) {
        return;
    }
    if ((ext = get_path_ext(buff->path)) == NULL) {
        return;
    }

    if (strcmp(ext, "glsl") == 0) {
        yed_buffer_set_ft(buff, yed_get_ft("GLSL"));
    }
}

void maybe_change_ft_event(yed_event *event) {
    if (event->buffer) {
        maybe_change_ft(event->buffer);
    }
}
