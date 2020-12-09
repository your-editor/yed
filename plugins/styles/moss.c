#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define white        MAYBE_CONVERT(RGB_32_hex(bdbea9))
#define black        MAYBE_CONVERT(RGB_32_hex(111111))
#define grey         MAYBE_CONVERT(RGB_32_hex(30352f))
#define dark_green   MAYBE_CONVERT(RGB_32_hex(151c18))
#define medium_green MAYBE_CONVERT(RGB_32_hex(628761))
#define light_green  MAYBE_CONVERT(RGB_32_hex(49a078))
#define red          MAYBE_CONVERT(RGB_32_hex(ff0000))
#define dark_blue    MAYBE_CONVERT(RGB_32_hex(495867))
#define rust         MAYBE_CONVERT(RGB_32_hex(a55f4a))

PACKABLE_STYLE(moss) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = white;
    s.active.bg           = dark_green;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = white;
    s.inactive.bg         = black;

    s.active_border       = s.active;
    s.active_border.fg    = medium_green;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = white;
    s.cursor_line.bg      = grey;

    s.search.flags        = attr_kind;
    s.search.fg           = dark_green;
    s.search.bg           = light_green;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = dark_green;
    s.search_cursor.bg    = medium_green;

    s.selection.flags     = attr_kind;
    s.selection.fg        = white;
    s.selection.bg        = grey;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = red;

    s.associate.flags     = attr_kind | ATTR_BOLD;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = black;
    s.status_line.bg      = medium_green;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = light_green;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = medium_green;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = dark_blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = light_green;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = medium_green;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = rust;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "moss", &s);

    return 0;
}
