#include <yed/plugin.h>

PACKABLE_STYLE(vt_light) {
    yed_style s;

    YED_PLUG_VERSION_CHECK();

    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_16 | ATTR_16_LIGHT_BG;
    s.active.fg           = ATTR_16_BLACK;
    s.active.bg           = ATTR_16_GREY;
    s.active_border = s.active;

    s.inactive.flags        = ATTR_16 | ATTR_16_LIGHT_BG;
    s.inactive.fg           = ATTR_16_BLACK;
    s.inactive.bg           = ATTR_16_GREY;
    s.inactive_border = s.inactive;

    s.cursor_line.flags     = ATTR_16;
    s.cursor_line.fg        = s.active.fg;
    s.cursor_line.bg        = ATTR_16_GREY;

    s.selection.flags     = ATTR_INVERSE;

    s.search.flags        = ATTR_16 | ATTR_INVERSE;
    s.search.fg           = ATTR_16_YELLOW;

    s.search_cursor.flags = ATTR_16 | ATTR_INVERSE;
    s.search_cursor.fg    = ATTR_16_MAGENTA;

    s.attention.flags     = ATTR_16;
    s.attention.fg        = ATTR_16_RED;

    s.status_line = s.active;
    s.status_line.flags   |= ATTR_INVERSE;

    s.command_line = s.active;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = ATTR_16;
    s.code_comment.fg     = ATTR_16_RED;

    s.code_keyword.flags  = ATTR_16;
    s.code_keyword.fg     = ATTR_16_BLUE;

    s.code_control_flow   =
    s.code_typename       = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_number.flags   = ATTR_16;
    s.code_number.fg      = ATTR_16_MAGENTA;

    s.code_constant       = s.code_number;
    s.code_string         = s.code_number;
    s.code_character      = s.code_number;

    yed_plugin_set_style(self, "vt-light", &s);

    return 0;
}
