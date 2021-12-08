#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define white      MAYBE_CONVERT(RGB_32_hex(F8F8F2))
#define dark_grey  MAYBE_CONVERT(RGB_32_hex(282A36))
#define sel_fg     MAYBE_CONVERT(RGB_32_hex(282A36))
#define sel_bg     MAYBE_CONVERT(RGB_32_hex(807b96))
#define black      MAYBE_CONVERT(RGB_32_hex(181A26))
#define grey       MAYBE_CONVERT(RGB_32_hex(4D4D4D))
#define red        MAYBE_CONVERT(RGB_32_hex(FF5555))
#define green      MAYBE_CONVERT(RGB_32_hex(50FA7B))
#define yellow     MAYBE_CONVERT(RGB_32_hex(F1FA8C))
#define purple     MAYBE_CONVERT(RGB_32_hex(BD93F9))
#define pink       MAYBE_CONVERT(RGB_32_hex(FF79C6))
#define blue       MAYBE_CONVERT(RGB_32_hex(8BE9FD))
#define light_grey MAYBE_CONVERT(RGB_32_hex(BFBFBF))

PACKABLE_STYLE(dracula) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = white;
    s.active.bg           = dark_grey;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = white;
    s.inactive.bg         = black;

    s.active_border       = s.active;
    s.active_border.fg    = purple;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = white;
    s.cursor_line.bg      = black;

    s.search.flags        = attr_kind;
    s.search.fg           = black;
    s.search.bg           = light_grey;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = red;
    s.search_cursor.bg    = light_grey;

    s.selection.flags     = attr_kind;
    s.selection.fg        = sel_fg;
    s.selection.bg        = sel_bg;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = red;

    s.associate.flags     = attr_kind | ATTR_BOLD;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = dark_grey;
    s.status_line.bg      = purple;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = grey;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = pink;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = pink;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = green;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = purple;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = purple;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = yellow;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "dracula", &s);

    return 0;
}
