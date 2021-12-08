#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

PACKABLE_STYLE(first) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = MAYBE_CONVERT(RGB_32(246, 241, 209));
    s.active.bg           = MAYBE_CONVERT(RGB_32(10, 20, 30));

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = MAYBE_CONVERT(RGB_32(246, 241, 209));
    s.inactive.bg         = MAYBE_CONVERT(RGB_32(0, 10, 20));

    s.active_border       = s.active;

    s.inactive_border     = s.inactive;
    s.inactive_border.fg  = MAYBE_CONVERT(RGB_32(64, 121, 140));

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = MAYBE_CONVERT(RGB_32(246, 241, 209));
    s.cursor_line.bg      = MAYBE_CONVERT(RGB_32(21, 42, 49));

    s.selection           = s.cursor_line;
/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = MAYBE_CONVERT(RGB_32(10, 20, 30)); */
/*     s.selection.bg        = MAYBE_CONVERT(RGB_32(246, 241, 209)); */

    s.search.flags        = attr_kind | ATTR_BOLD;
    s.search.fg           = MAYBE_CONVERT(RGB_32(0, 0, 255));
    s.search.bg           = MAYBE_CONVERT(RGB_32(255, 255, 0));

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = MAYBE_CONVERT(RGB_32(0, 0, 255));
    s.search_cursor.bg    = MAYBE_CONVERT(RGB_32(255, 150, 0));

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = MAYBE_CONVERT(RGB_32(255, 0, 0));

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = MAYBE_CONVERT(s.selection.bg + RGB_32(0, 0, 60));

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = MAYBE_CONVERT(RGB_32(246, 241, 209));
    s.status_line.bg      = MAYBE_CONVERT(RGB_32(64, 121, 140));

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = MAYBE_CONVERT(RGB_32(72, 180, 235));

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = MAYBE_CONVERT(RGB_32(216, 30, 91));

    s.code_control_flow   = s.code_keyword;

    s.code_typename.flags = attr_kind | ATTR_BOLD;
    s.code_typename.fg    = MAYBE_CONVERT(RGB_32_hex(87B38D));

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = MAYBE_CONVERT(RGB_32(64, 121, 140));

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = MAYBE_CONVERT(RGB_32(147, 97, 129));

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = MAYBE_CONVERT(RGB_32(252, 163, 17));

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = MAYBE_CONVERT(RGB_32(83, 170, 111));

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "first-dark", &s);


    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = MAYBE_CONVERT(RGB_32(11, 32, 39));
    s.active.bg           = MAYBE_CONVERT(RGB_32(246, 241, 209));

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = MAYBE_CONVERT(RGB_32(11, 32, 39));
    s.inactive.bg         = MAYBE_CONVERT(RGB_32(226, 221, 189));

    s.active_border       = s.active;
    s.active_border.fg    = MAYBE_CONVERT(RGB_32(216, 30, 91));

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = MAYBE_CONVERT(RGB_32(11, 32, 39));
    s.cursor_line.bg      = MAYBE_CONVERT(RGB_32(207, 215, 199));

    s.selection           = s.cursor_line;
/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = MAYBE_CONVERT(RGB_32(246, 241, 209)); */
/*     s.selection.bg        = MAYBE_CONVERT(RGB_32(11, 32, 39)); */

    s.search.flags        = attr_kind | ATTR_BOLD;
    s.search.fg           = MAYBE_CONVERT(RGB_32(0, 0, 255));
    s.search.bg           = MAYBE_CONVERT(RGB_32(255, 255, 0));

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = MAYBE_CONVERT(RGB_32(0, 0, 255));
    s.search_cursor.bg    = MAYBE_CONVERT(RGB_32(255, 150, 0));

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = MAYBE_CONVERT(RGB_32(255, 0, 0));

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = MAYBE_CONVERT(s.selection.bg - RGB_32(0, 0, 60));

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = MAYBE_CONVERT(RGB_32(246, 241, 209));
    s.status_line.bg      = MAYBE_CONVERT(RGB_32(64, 121, 140));

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = MAYBE_CONVERT(RGB_32(72, 180, 235));

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = MAYBE_CONVERT(RGB_32(216, 30, 91));

    s.code_control_flow   = s.code_keyword;

    s.code_typename.flags = attr_kind | ATTR_BOLD;
    s.code_typename.fg    = MAYBE_CONVERT(RGB_32_hex(67936D));

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = MAYBE_CONVERT(RGB_32(64, 121, 140));

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = MAYBE_CONVERT(RGB_32(147, 97, 129));

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = MAYBE_CONVERT(RGB_32(252, 163, 17));

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = MAYBE_CONVERT(RGB_32(83, 170, 111));

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "first-light", &s);

    return 0;
}
