#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define green       MAYBE_CONVERT(RGB_32_hex(92d7a5))
#define purple      MAYBE_CONVERT(RGB_32_hex(B8B8D1))
#define blue        MAYBE_CONVERT(RGB_32_hex(4980a3))
#define magenta     MAYBE_CONVERT(RGB_32_hex(aa4465))
#define light_blue  MAYBE_CONVERT(RGB_32_hex(b9d6f2))
#define yellow      MAYBE_CONVERT(RGB_32_hex(f9e784))
#define orange      MAYBE_CONVERT(RGB_32_hex(e58f65))
#define attn        MAYBE_CONVERT(RGB_32_hex(ff0000))
#define sel_color   MAYBE_CONVERT(RGB_32_hex(17233f))
#define background  MAYBE_CONVERT(RGB_32_hex(3A506B))
#define foreground  MAYBE_CONVERT(RGB_32_hex(FAF3DD))

PACKABLE_STYLE(sammy) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = foreground;
    s.active.bg           = background;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = foreground;
    s.inactive.bg         = background;

    s.active_border       = s.active;
    s.active_border.fg    = magenta;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = foreground;
    s.cursor_line.bg      = sel_color;

    s.search.flags        = attr_kind;
    s.search.fg           = background;
    s.search.bg           = foreground;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = magenta;
    s.search_cursor.bg    = foreground;

    s.selection.flags     = attr_kind;
    s.selection.fg        = foreground;
    s.selection.bg        = sel_color;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = attn;

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = blue;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = foreground;
    s.status_line.bg      = magenta;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = yellow;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = green;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = magenta;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = light_blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = orange;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = orange;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = purple;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "sammy", &s);

    return 0;
}
