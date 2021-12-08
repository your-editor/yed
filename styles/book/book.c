#include <yed/plugin.h>

PACKABLE_STYLE(book) {
    yed_style s;

    YED_PLUG_VERSION_CHECK();

    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG;
    s.active.fg           = ATTR_16_BLACK;
    s.active.bg           = ATTR_16_GREY;

    s.inactive.flags      = ATTR_16 | ATTR_16_LIGHT_BG;
    s.inactive.fg         = ATTR_16_BLACK;
    s.inactive.bg         = ATTR_16_BLACK;

    s.active_border       = s.active;
    s.active_border.fg    = ATTR_16_BLUE;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG;
    s.cursor_line.fg      = ATTR_16_GREY;
    s.cursor_line.bg      = ATTR_16_BLACK;

/*     s.selection.flags     = ATTR_INVERSE; */
    s.selection     = s.inactive;

    s.search.flags        = ATTR_16 | ATTR_16_LIGHT_FG;
    s.search.fg           = ATTR_16_BLUE;
    s.search.bg           = ATTR_16_YELLOW;

    s.search_cursor.flags = ATTR_16 | ATTR_16_LIGHT_FG;
    s.search_cursor.fg    = ATTR_16_BLUE;
    s.search_cursor.bg    = ATTR_16_MAGENTA;

    s.attention.flags     = ATTR_16 | ATTR_16_LIGHT_FG;
    s.attention.fg        = ATTR_16_RED;

    s.associate.flags     = ATTR_16 | ATTR_BOLD;

    s.command_line        = s.active;

    s.status_line.flags   = ATTR_16 | ATTR_BOLD | ATTR_16_LIGHT_FG | ATTR_16_LIGHT_BG;
    s.status_line.fg      = ATTR_16_GREY;
    s.status_line.bg      = ATTR_16_BLUE;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = ATTR_16 | ATTR_16_LIGHT_FG | ATTR_BOLD;
    s.code_comment.fg     = ATTR_16_BLUE;

    s.code_keyword.flags  = ATTR_16 | ATTR_16_LIGHT_FG ;
    s.code_keyword.fg     = ATTR_16_MAGENTA;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = ATTR_16;
    s.code_fn_call.fg     = ATTR_16_BLACK;

    s.code_number.flags   = ATTR_16;
    s.code_number.fg      = ATTR_16_BLUE;

    s.code_constant.flags = ATTR_16 | ATTR_16_LIGHT_FG ;
    s.code_constant.fg    = ATTR_16_MAGENTA;

    s.code_string.flags   = ATTR_16 | ATTR_16_LIGHT_FG ;
    s.code_string.fg      = ATTR_16_RED;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "book", &s);

    return 0;
}
