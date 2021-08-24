#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define color_bg            MAYBE_CONVERT(RGB_32_hex(0F141E))
#define color_bg_2          MAYBE_CONVERT(RGB_32_hex(1F242E))
#define color_cursor_line   MAYBE_CONVERT(RGB_32_hex(2f343e))
#define color_fg            MAYBE_CONVERT(RGB_32_hex(D4BBB1))
#define color_status_bar_fg MAYBE_CONVERT(RGB_32_hex(0F141E))
#define color_status_bar_bg MAYBE_CONVERT(RGB_32_hex(AB595D))
#define color_search        MAYBE_CONVERT(RGB_32_hex(49d45c))
#define color_search_cursor MAYBE_CONVERT(RGB_32_hex(d4495c))
#define color_attent        MAYBE_CONVERT(RGB_32_hex(ef0808))
#define color_assoc_fg      MAYBE_CONVERT(RGB_32_hex(1F242E))
#define color_assoc_bg      MAYBE_CONVERT(RGB_32_hex(6461A0))
#define color_comment       MAYBE_CONVERT(RGB_32_hex(6f6f7f))
#define color_keyword       MAYBE_CONVERT(RGB_32_hex(AB4E37))
#define color_control_flow  MAYBE_CONVERT(RGB_32_hex(AB4E37))
#define color_typename      MAYBE_CONVERT(RGB_32_hex(AB4E37))
#define color_pp            MAYBE_CONVERT(RGB_32_hex(efefef))
#define color_fn            MAYBE_CONVERT(RGB_32_hex(9Fa4aE))
#define color_string        MAYBE_CONVERT(RGB_32_hex(7B9E87))
#define color_character     MAYBE_CONVERT(RGB_32_hex(7B9E87))
#define color_num           MAYBE_CONVERT(RGB_32_hex(7B9E87))
#define color_constant      MAYBE_CONVERT(RGB_32_hex(efefef))


PACKABLE_STYLE(drift) {
    yed_style s;
    int       tc;
    int       attr_kind;

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = color_fg;
    s.active.bg           = color_bg;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = color_fg;
    s.inactive.bg         = color_bg_2;

    s.active_border       = s.active;
    s.active_border.fg    = color_fg;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = color_cursor_line;

    s.selection           = s.cursor_line;

    s.search.flags        = attr_kind;
    s.search.fg           = color_bg;
    s.search.bg           = color_search;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = color_bg;
    s.search_cursor.bg    = color_search_cursor;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = color_attent;

    s.associate.flags     = attr_kind;
    s.associate.fg        = color_assoc_fg;
    s.associate.bg        = color_assoc_bg;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = color_status_bar_fg;
    s.status_line.bg      = color_status_bar_bg;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = color_comment;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = color_keyword;

    s.code_control_flow.flags  = attr_kind | ATTR_BOLD;
    s.code_control_flow.fg     = color_control_flow;

    s.code_typename.flags  = attr_kind | ATTR_BOLD;
    s.code_typename.fg     = color_typename;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = color_pp;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = color_fn;

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = color_num;

    s.code_string.flags  = attr_kind;
    s.code_string.fg     = color_string;

    s.code_character.flags  = attr_kind;
    s.code_character.fg     = color_character;

    s.code_constant.flags  = attr_kind;
    s.code_constant.fg     = color_constant;

    yed_plugin_set_style(self, "drift", &s);

    return 0;
}
