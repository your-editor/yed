#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define black        MAYBE_CONVERT(RGB_32_hex(090a18))
#define peach        MAYBE_CONVERT(RGB_32_hex(ff778a))
#define light_green  MAYBE_CONVERT(RGB_32_hex(6ab539))
#define yellow       MAYBE_CONVERT(RGB_32_hex(bfa01a))
#define light_blue   MAYBE_CONVERT(RGB_32_hex(4aaed3))
#define salmon       MAYBE_CONVERT(RGB_32_hex(e58a82))
#define light_cyan   MAYBE_CONVERT(RGB_32_hex(29b3bb))
#define lavender     MAYBE_CONVERT(RGB_32_hex(a59ebd))
#define maroon       MAYBE_CONVERT(RGB_32_hex(260e22))
#define orange       MAYBE_CONVERT(RGB_32_hex(f78e2f))
#define green        MAYBE_CONVERT(RGB_32_hex(60ba80))
#define light_orange MAYBE_CONVERT(RGB_32_hex(de9b1d))
#define purple       MAYBE_CONVERT(RGB_32_hex(8ba7ea))
#define pink         MAYBE_CONVERT(RGB_32_hex(e08bd6))
#define cyan         MAYBE_CONVERT(RGB_32_hex(2cbab6))
#define grey         MAYBE_CONVERT(RGB_32_hex(b4abac))

#define background   black
#define foreground   grey
#define attn         MAYBE_CONVERT(RGB_32_hex(ff0000))
#define assoc_bg     MAYBE_CONVERT(background + RGB_32_hex(111122))
#define sel_color    maroon

PACKABLE_STYLE(tempus_future) {
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

    s.active_border        = s.active;
    s.active_border.fg     = maroon;

    s.inactive_border      = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = foreground;
    s.cursor_line.bg      = sel_color;

    s.search.flags        = attr_kind;
    s.search.fg           = background;
    s.search.bg           = light_orange;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = maroon;
    s.search_cursor.bg    = light_orange;

    s.selection.flags     = attr_kind;
    s.selection.fg        = foreground;
    s.selection.bg        = sel_color;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = attn;

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = assoc_bg;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = foreground;
    s.status_line.bg      = maroon;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = lavender;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = green;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = yellow;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = orange;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = pink;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = light_blue;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = purple;

    s.code_character.flags   = attr_kind;
    s.code_character.fg      = peach;

    yed_plugin_set_style(self, "tempus-future", &s);

    return 0;
}
