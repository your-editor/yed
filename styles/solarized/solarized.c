#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define base03    MAYBE_CONVERT(RGB_32_hex(002b36))
#define base02    MAYBE_CONVERT(RGB_32_hex(073642))
#define base01    MAYBE_CONVERT(RGB_32_hex(586e75))
#define base00    MAYBE_CONVERT(RGB_32_hex(657b83))
#define base0     MAYBE_CONVERT(RGB_32_hex(839496))
#define base1     MAYBE_CONVERT(RGB_32_hex(93a1a1))
#define base2     MAYBE_CONVERT(RGB_32_hex(eee8d5))
#define base3     MAYBE_CONVERT(RGB_32_hex(fdf6e3))
#define yellow    MAYBE_CONVERT(RGB_32_hex(b58900))
#define orange    MAYBE_CONVERT(RGB_32_hex(cb4b16))
#define red       MAYBE_CONVERT(RGB_32_hex(dc322f))
#define magenta   MAYBE_CONVERT(RGB_32_hex(d33682))
#define violet    MAYBE_CONVERT(RGB_32_hex(6c71c4))
#define blue      MAYBE_CONVERT(RGB_32_hex(268bd2))
#define cyan      MAYBE_CONVERT(RGB_32_hex(2aa198))
#define green     MAYBE_CONVERT(RGB_32_hex(859900))

#define attn        MAYBE_CONVERT(RGB_32_hex(ff0000))

PACKABLE_STYLE(solarized) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = base1;
    s.active.bg           = base02;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = base1;
    s.inactive.bg         = base03;

    s.active_border       = s.active;
    s.active_border.fg    = red;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = base1;
    s.cursor_line.bg      = base03;

    s.search.flags        = attr_kind;
    s.search.fg           = base02;
    s.search.bg           = blue;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = red;
    s.search_cursor.bg    = blue;

    s.selection.flags     = attr_kind;
    s.selection.fg        = base1;
    s.selection.bg        = base03;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = attn;

    s.associate.flags     = attr_kind | ATTR_BOLD;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = base02;
    s.status_line.bg      = red;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = base01;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = yellow;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = red;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = red;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = green;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = cyan;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "solarized-dark", &s);

    s.active.fg           = base01;
    s.active.bg           = base3;

    s.inactive.fg         = base01;
    s.inactive.bg         = base2;

    s.active_border       = s.active;
    s.active_border.fg    = blue;

    s.inactive_border     = s.inactive;

    s.cursor_line.fg      = base01;
    s.cursor_line.bg      = base2;

    s.selection.fg        = base01;
    s.selection.bg        = base2;

    s.command_line        = s.active;

    s.status_line.fg      = base3;
    s.status_line.bg      = blue;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.fg     = base1;

    yed_plugin_set_style(self, "solarized-light", &s);

    return 0;
}
