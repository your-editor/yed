#include <yed/plugin.h>

void update(yed_event *event) {
    struct tm *tm;
    time_t     t;
    char       buff[256];

    t  = time(NULL);
    tm = localtime(&t);
    strftime(buff, sizeof(buff), "%I:%M:%S", tm);

    yed_set_var("status-bar-clock-time", buff);
}

int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    h.kind = EVENT_PRE_PUMP;
    h.fn   = update;

    yed_plugin_add_event_handler(self, h);

    yed_set_var("status-line-var", "status-bar-clock-time");

    return 0;
}
