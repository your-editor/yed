#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))
#define ALT(rgb, _256)     (tc ? (rgb) : (_256))

#define nord0              ALT(RGB_32_hex(2E3440), 234)
#define nord1              ALT(RGB_32_hex(3B4252), 236)
#define nord2              ALT(RGB_32_hex(434C5E), 237)
#define nord3              MAYBE_CONVERT(RGB_32_hex(4C566A))
#define nord3_but_brighter MAYBE_CONVERT(RGB_32_hex(6C768A))
#define nord4              MAYBE_CONVERT(RGB_32_hex(D8DEE9))
#define nord5              MAYBE_CONVERT(RGB_32_hex(E5E9F0))
#define nord6              MAYBE_CONVERT(RGB_32_hex(ECEFF4))
#define nord7              MAYBE_CONVERT(RGB_32_hex(8FBCBB))
#define nord8              MAYBE_CONVERT(RGB_32_hex(88C0D0))
#define nord9              MAYBE_CONVERT(RGB_32_hex(81A1C1))
#define nord10             MAYBE_CONVERT(RGB_32_hex(5E81AC))
#define nord11             MAYBE_CONVERT(RGB_32_hex(BF616A))
#define nord12             MAYBE_CONVERT(RGB_32_hex(D08770))
#define nord13             MAYBE_CONVERT(RGB_32_hex(EBCB8B))
#define nord14             MAYBE_CONVERT(RGB_32_hex(A3BE8C))
#define nord15             MAYBE_CONVERT(RGB_32_hex(B48EAD))

PACKABLE_STYLE(nord) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = nord5;
    s.active.bg           = nord0;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = nord4;
    s.inactive.bg         = nord1;

    s.active_border       = s.active;
    s.active_border.fg    = nord12;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = nord6;
    s.cursor_line.bg      = nord2;

    s.search.flags        = attr_kind | ATTR_BOLD;
    s.search.fg           = nord6;
    s.search.bg           = nord10;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = nord10;
    s.search_cursor.bg    = nord13;

    s.selection.flags     = attr_kind;
    s.selection.fg        = nord5;
    s.selection.bg        = nord2;

    s.attention.flags     = attr_kind;
    s.attention.fg        = nord11;

    s.associate.flags     = attr_kind;
    s.associate.fg        = nord12;

    s.command_line        = s.inactive;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = s.active.fg;
    s.status_line.bg      = nord3;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind;
    s.code_comment.fg     = nord3_but_brighter;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = nord9;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = nord8;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = nord15;

    s.code_constant.flags = attr_kind;
    s.code_constant.fg    = nord15;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = nord14;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "nord", &s);

    return 0;
}
