#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define white       MAYBE_CONVERT(RGB_32_hex(d9d4bd))
#define black       MAYBE_CONVERT(RGB_32_hex(222215))
#define grey        ALT(          RGB_32_hex(37352d), 237)
#define grey2       ALT(          RGB_32_hex(302e27), 238)
#define blue        MAYBE_CONVERT(RGB_32_hex(36a8aa))
#define green       MAYBE_CONVERT(RGB_32_hex(9ab863))
#define yellow      MAYBE_CONVERT(RGB_32_hex(daae4e))
#define red         ALT(          RGB_32_hex(c13f2e), 1)
#define orange      MAYBE_CONVERT(RGB_32_hex(ff731c))
#define attn        MAYBE_CONVERT(RGB_32_hex(ff0000))
#define sel_color   MAYBE_CONVERT(RGB_32_hex(444444))

PACKABLE_STYLE(hat) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = white;
    s.active.bg           = grey;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = white;
    s.inactive.bg         = grey2;

    s.active_border       = s.active;
    s.active_border.fg    = red;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = white;
    s.cursor_line.bg      = sel_color;

    s.search.flags        = attr_kind;
    s.search.fg           = black;
    s.search.bg           = blue;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = red;
    s.search_cursor.bg    = blue;

    s.selection.flags     = attr_kind;
    s.selection.fg        = white;
    s.selection.bg        = sel_color;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = attn;

    s.associate.flags     = attr_kind | ATTR_BOLD;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = black;
    s.status_line.bg      = red;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = orange;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = red;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = yellow;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = yellow;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = yellow;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = green;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "hat", &s);

    return 0;
}
