#include <yed/plugin.h>

array_t dds;

void ekey(yed_event *event) {
    char *mouse_buttons[] = {
        "LEFT",
        "MIDDLE",
        "RIGHT",
        "WHEEL UP",
        "WHEEL DOWN",
    };

    char *mouse_actions[] = {
        "PRESS",
        "RELEASE",
        "DRAG",
    };

    yed_direct_draw_t  *dd;
    yed_direct_draw_t **dit;

    if (IS_MOUSE(event->key)) {
        LOG_FN_ENTER();
        yed_log("MOUSE: %s %s %d %d",
                mouse_buttons[MOUSE_BUTTON(event->key)],
                mouse_actions[MOUSE_KIND(event->key)],
                MOUSE_ROW(event->key),
                MOUSE_COL(event->key));
        LOG_EXIT();

        switch (MOUSE_BUTTON(event->key)) {
            case MOUSE_BUTTON_LEFT:
                if (MOUSE_KIND(event->key) == MOUSE_PRESS
                ||  MOUSE_KIND(event->key) == MOUSE_DRAG) {
                    dd = yed_direct_draw(MOUSE_ROW(event->key),
                                         MOUSE_COL(event->key),
                                         yed_parse_attrs("bg ff00ff"),
                                         " ");
                    array_push(dds, dd);
                }
                break;
            case MOUSE_BUTTON_RIGHT:
                array_traverse(dds, dit) {
                    dd = *dit;
                    yed_kill_direct_draw(dd);
                }
                array_clear(dds);
                break;
            case MOUSE_WHEEL_DOWN:
                yed_frame_scroll_buffer(ys->active_frame, 1);
                break;
            case MOUSE_WHEEL_UP:
                yed_frame_scroll_buffer(ys->active_frame, -1);
                break;
        }
    }
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler key;

    YED_PLUG_VERSION_CHECK();

    dds = array_make(yed_direct_draw_t*);

    yed_plugin_request_mouse_reporting(self);

    key.kind = EVENT_KEY_PRESSED;
    key.fn   = ekey;

    yed_plugin_add_event_handler(self, key);

    return 0;
}
