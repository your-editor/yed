#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define white       MAYBE_CONVERT(RGB_32_hex(ffffff))
#define black       MAYBE_CONVERT(RGB_32_hex(111111))
#define grey        MAYBE_CONVERT(RGB_32_hex(123456))
#define dark_blue   MAYBE_CONVERT(RGB_32_hex(091729))
#define medium_blue MAYBE_CONVERT(RGB_32_hex(58a4b0))
#define light_blue  MAYBE_CONVERT(RGB_32_hex(b0d0d3))
#define yellow      MAYBE_CONVERT(RGB_32_hex(ddcc73))
#define green       MAYBE_CONVERT(RGB_32_hex(b6cb9e))
#define red         MAYBE_CONVERT(RGB_32_hex(ff0000))
#define orange      MAYBE_CONVERT(RGB_32_hex(D08770))
#define rust        MAYBE_CONVERT(RGB_32_hex(795c5f))

PACKABLE_STYLE(cadet) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = white;
    s.active.bg           = dark_blue;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = white;
    s.inactive.bg         = black;

    s.active_border       = s.active;
    s.active_border.fg    = medium_blue;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = white;
    s.cursor_line.bg      = grey;

    s.search.flags        = attr_kind;
    s.search.fg           = dark_blue;
    s.search.bg           = green;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = dark_blue;
    s.search_cursor.bg    = yellow;

    s.selection.flags     = attr_kind;
    s.selection.fg        = white;
    s.selection.bg        = grey;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = red;

    s.associate.flags     = attr_kind;
    s.associate.fg        = orange;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = black;
    s.status_line.bg      = medium_blue;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = rust;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = yellow;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = light_blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = medium_blue;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = medium_blue;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = green;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "cadet", &s);

    return 0;
}
