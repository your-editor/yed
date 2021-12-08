#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#if 0
let s:palette.bg        = "#111a1f"
let s:palette.comment   = "#545759"
let s:palette.markup    = "#8D7856"
let s:palette.constant  = "#738C9C"
let s:palette.operator  = "#738C9C"
let s:palette.tag       = "#8D7856"
let s:palette.regexp    = "#6998B3"
let s:palette.string    = "#798362"
let s:palette.function  = "#8D7856"
let s:palette.special   = "#9B9257"
let s:palette.keyword   = "#8D7856"
let s:palette.error     = "#810002"
let s:palette.accent    = "#9B9257"
let s:palette.panel     = "#14191F"
let s:palette.guide     = "#2D3640"
let s:palette.line      = "#151A1E"
let s:palette.selection = "#253340"
let s:palette.fg        = "#ACB3B5"
let s:palette.fg_idle   = "#3E4B59"
let s:palette.diffg     = "#012800"
let s:palette.diffr     = "#340001"
let s:palette.cdiffg    = "#037500"
let s:palette.cdiffy    = "#817e00"
let s:palette.cdiffr    = "#810002"
#endif

#define color_fg            MAYBE_CONVERT(RGB_32_hex(ebdbb2))
#define color_bg            MAYBE_CONVERT(RGB_32_hex(3d5663))
#define color_bg_2          MAYBE_CONVERT(RGB_32_hex(253340))
#define color_search        MAYBE_CONVERT(RGB_32(73, 212, 92))
#define color_search_cursor MAYBE_CONVERT(RGB_32(212, 73, 92))
#define color_attent        MAYBE_CONVERT(RGB_32_hex(dd0000))
#define color_assoc         color_bg_2
#define color_comment       MAYBE_CONVERT(RGB_32_hex(6f7f6f))
#define color_keyword       MAYBE_CONVERT(RGB_32_hex(eeeeee))
#define color_pp            MAYBE_CONVERT(RGB_32_hex(A3937F))
#define color_fn            MAYBE_CONVERT(RGB_32_hex(5CA0BD))
#define color_num           MAYBE_CONVERT(RGB_32_hex(C78283))
#define color_str           MAYBE_CONVERT(RGB_32_hex(00AFB9))
#define color_ty            MAYBE_CONVERT(RGB_32_hex(C78283))
#define color_cf            color_keyword

PACKABLE_STYLE(river) {
    yed_style s;
    int       tc,
              attr_kind;

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = color_fg;
    s.active.bg           = color_bg;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = color_fg;
    s.inactive.bg         = color_bg;

    s.active_border       = s.active;
    s.active_border.fg    = color_keyword;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = color_bg_2;

    s.selection           = s.cursor_line;

/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = s.active.bg; */
/*     s.selection.bg        = s.active.fg; */

    s.search.flags        = attr_kind;
    s.search.fg           = color_bg;
    s.search.bg           = color_search;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = color_bg;
    s.search_cursor.bg    = color_search_cursor;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = color_attent;

    s.associate.flags     = attr_kind | ATTR_INVERSE;
    s.associate.bg        = color_assoc;
    s.associate.bg        = color_assoc;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = color_bg_2;
    s.status_line.bg      = color_fg;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = color_comment;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = color_keyword;

    s.code_control_flow.fg = color_cf;
    s.code_typename.fg     = color_ty;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = color_pp;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = color_fn;

    s.code_number.flags  = attr_kind;
    s.code_number.fg     = color_num;

    s.code_constant      = s.code_preprocessor;
    s.code_string.fg     = color_str;
    s.code_character.fg  = color_str;

    yed_plugin_set_style(self, "river", &s);

    return 0;
}
