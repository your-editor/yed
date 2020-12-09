#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define dark_green   MAYBE_CONVERT(RGB_32_hex(294037))
#define green        MAYBE_CONVERT(RGB_32_hex(87D6A3))
#define tan          MAYBE_CONVERT(RGB_32_hex(EBE0C0))
#define blue         MAYBE_CONVERT(RGB_32_hex(9CCFFF))
#define salmon       MAYBE_CONVERT(RGB_32_hex(DEA178))
#define yellow       MAYBE_CONVERT(RGB_32_hex(F0E989))
#define orange       MAYBE_CONVERT(RGB_32_hex(f78e2f))
#define cyan         MAYBE_CONVERT(RGB_32_hex(30F2F2))
#define pink         MAYBE_CONVERT(RGB_32_hex(e08bd6))
#define white        MAYBE_CONVERT(RGB_32_hex(FFFFFF))
#define grey         MAYBE_CONVERT(RGB_32_hex(002222))

#define background   dark_green
#define foreground   tan
#define attn         MAYBE_CONVERT(RGB_32_hex(ff0000))
#define assoc_bg     MAYBE_CONVERT(background + RGB_32_hex(111122))
#define sel_color    grey

PACKABLE_STYLE(olive) {
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
    s.active_border.fg     = foreground;

    s.inactive_border      = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = foreground;
    s.cursor_line.bg      = sel_color;

    s.search.flags        = attr_kind;
    s.search.fg           = sel_color;
    s.search.bg           = pink;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = sel_color;
    s.search_cursor.bg    = green;

    s.selection.flags     = attr_kind;
    s.selection.fg        = foreground;
    s.selection.bg        = sel_color;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = attn;

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = assoc_bg;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = background;
    s.status_line.bg      = foreground;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = yellow;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = green;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = white;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = blue;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = cyan;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = orange;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = salmon;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "olive", &s);

    return 0;
}
