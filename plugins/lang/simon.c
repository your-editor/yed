#include <yed/plugin.h>

void unload(yed_plugin *self);
void maybe_change_ft(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler buff_post_load_handler;
    yed_event_handler buff_pre_write_handler;

LOG_FN_ENTER();
    yed_plugin_set_unload_fn(self, unload);

    if (yed_make_ft("Simon") == FT_ERR_TAKEN) {
        yed_cerr("lang/simon: unable to create file type name");
        LOG_EXIT();
        return 1;
    }

    buff_post_load_handler.kind = EVENT_BUFFER_POST_LOAD;
    buff_post_load_handler.fn   = maybe_change_ft;
    buff_pre_write_handler.kind = EVENT_BUFFER_PRE_WRITE;
    buff_pre_write_handler.fn   = maybe_change_ft;

    yed_plugin_add_event_handler(self, buff_post_load_handler);
    yed_plugin_add_event_handler(self, buff_pre_write_handler);

    YEXE("plugin-load", "lang/syntax/simon");

LOG_EXIT();
    return 0;
}

void unload(yed_plugin *self) {
    YEXE("plugin-unload", "lang/syntax/simon");
    yed_delete_ft("Simon");
}

void maybe_change_ft(yed_event *event) {
    char *ext;

    if (event->buffer->path == NULL) {
        return;
    }
    if ((ext = get_path_ext(event->buffer->path)) == NULL) {
        return;
    }

    if (strcmp(ext, "si") == 0) {
        yed_buffer_set_ft(event->buffer, yed_get_ft("Simon"));
    }
}
