#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))



#define both_00        MAYBE_CONVERT(RGB_32_hex(fcfcfc))
#define both_01        MAYBE_CONVERT(RGB_32_hex(eeede7))
#define both_02        MAYBE_CONVERT(RGB_32_hex(d6d6d3))
#define both_03        MAYBE_CONVERT(RGB_32_hex(c0c0bd))
#define both_04        MAYBE_CONVERT(RGB_32_hex(7a7b75))
#define both_07        MAYBE_CONVERT(RGB_32_hex(070708))
#define both_09        MAYBE_CONVERT(RGB_32_hex(ff9505))
#define dark_grey      MAYBE_CONVERT(RGB_32_hex(2f3337))
#define light_grey     MAYBE_CONVERT(RGB_32_hex(eeede7))
#define dark_red       MAYBE_CONVERT(RGB_32_hex(f7143a))
#define light_red      MAYBE_CONVERT(RGB_32_hex(b0151a))
#define dark_orange    MAYBE_CONVERT(RGB_32_hex(ffb627))
#define light_orange   MAYBE_CONVERT(RGB_32_hex(fb6107))
#define dark_green     MAYBE_CONVERT(RGB_32_hex(02d849))
#define light_green    MAYBE_CONVERT(RGB_32_hex(22a54e))
#define dark_cyan      MAYBE_CONVERT(RGB_32_hex(0ed1d1))
#define light_cyan     MAYBE_CONVERT(RGB_32_hex(007784))
#define dark_blue      MAYBE_CONVERT(RGB_32_hex(4bc1fc))
#define light_blue     MAYBE_CONVERT(RGB_32_hex(006fd7))
#define dark_magenta   MAYBE_CONVERT(RGB_32_hex(ec44eb))
#define light_magenta  MAYBE_CONVERT(RGB_32_hex(811cac))
#define dark_brown     MAYBE_CONVERT(RGB_32_hex(b27701))
#define light_brown    MAYBE_CONVERT(RGB_32_hex(7f5501))
#define dark_BG        MAYBE_CONVERT(RGB_32_hex(232629))
#define dark_FG        MAYBE_CONVERT(RGB_32_hex(f8f8f2))
#define light_BG       MAYBE_CONVERT(RGB_32_hex(f8f8f2))
#define light_FG       MAYBE_CONVERT(RGB_32_hex(232629))
#define dark_assoc     MAYBE_CONVERT(RGB_32_hex(484e54))
#define light_assoc    MAYBE_CONVERT(RGB_32_hex(d6d6d3))

PACKABLE_STYLE(humanoid) {
    yed_style s;
    int       tc,
              attr_kind;

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = dark_FG;
    s.active.bg           = dark_BG;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = dark_FG;
    s.inactive.bg         = dark_BG;

    s.active_border       = s.active;
    s.active_border.fg    = dark_magenta;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = dark_grey;

    s.selection           = s.cursor_line;

/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = s.active.bg; */
/*     s.selection.bg        = s.active.fg; */

    s.search.flags        = attr_kind;
    s.search.fg           = dark_grey;
    s.search.bg           = dark_orange;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = s.search.fg;
    s.search_cursor.bg    = s.search.bg;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = dark_red;

    s.associate.flags     = attr_kind;
    s.associate.bg        = dark_assoc;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = dark_FG;
    s.status_line.bg      = dark_grey;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = both_03;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = dark_magenta;

    s.code_control_flow.fg = dark_magenta;
    s.code_typename.fg     = dark_orange;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = dark_orange;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = dark_blue;

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = both_09;

    s.code_constant      = s.code_number;
    s.code_string.fg     = dark_green;
    s.code_character.fg  = dark_green;

    yed_plugin_set_style(self, "humanoid-dark", &s);


    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = light_FG;
    s.active.bg           = light_BG;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = light_FG;
    s.inactive.bg         = light_BG;

    s.active_border       = s.active;
    s.active_border.fg    = light_magenta;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = light_grey;

    s.selection           = s.cursor_line;

/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = s.active.bg; */
/*     s.selection.bg        = s.active.fg; */

    s.search.flags        = attr_kind;
    s.search.fg           = light_grey;
    s.search.bg           = light_orange;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = s.search.fg;
    s.search_cursor.bg    = s.search.bg;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = light_red;

    s.associate.flags     = attr_kind;
    s.associate.bg        = light_assoc;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = light_FG;
    s.status_line.bg      = light_grey;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = both_03;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = light_magenta;

    s.code_control_flow.fg = light_magenta;
    s.code_typename.fg     = light_orange;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = light_orange;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = light_blue;

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = both_09;

    s.code_constant      = s.code_number;
    s.code_string.fg     = light_green;
    s.code_character.fg  = light_green;

    yed_plugin_set_style(self, "humanoid-light", &s);

    return 0;
}
