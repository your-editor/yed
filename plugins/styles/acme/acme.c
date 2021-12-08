#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define _acme_bg      (0xFFFFE8)
#define _acme_bg_dark (0xE5E5D0)

#define acme_bg            MAYBE_CONVERT(_acme_bg)
#define acme_bg_alt2       MAYBE_CONVERT(0xeFeFd8)
#define acme_bg_dark       MAYBE_CONVERT(_acme_bg_dark)
#define acme_fg            MAYBE_CONVERT(0x444444)
#define acme_fg_alt        MAYBE_CONVERT(0xB8B09A)
#define acme_fg_alt_dark   MAYBE_CONVERT(0x988d6d)
#define acme_fg_light      MAYBE_CONVERT(0xCCCCB7)
#define acme_highlight     MAYBE_CONVERT(0xe8eb98)
#define acme_highlight_alt MAYBE_CONVERT(0xe8ebc8)
#define acme_cyan          MAYBE_CONVERT(0x007777)
#define acme_cyan_light    MAYBE_CONVERT(0xa8efeb)
#define acme_red           MAYBE_CONVERT(0x880000)
#define acme_red_light     MAYBE_CONVERT(0xf8e8e8)
#define acme_yellow        MAYBE_CONVERT(0x888838)
#define acme_yellow_light  MAYBE_CONVERT(0xf8fce8)
#define acme_green         MAYBE_CONVERT(0x005500)
#define acme_green_alt     MAYBE_CONVERT(0x006600)
#define acme_green_light   MAYBE_CONVERT(0xe8fce8)
#define acme_blue          MAYBE_CONVERT(0x1054af)
#define acme_blue_light    MAYBE_CONVERT(0xc1dadf)
#define acme_purple        MAYBE_CONVERT(0x555599)
#define acme_purple_light  MAYBE_CONVERT(0xffeaff)

#define background      acme_bg
#define foreground      acme_fg
#define srch            acme_fg_alt_dark
#define srch_cursor     acme_green
#define attn            acme_red
#define assoc_bg        MAYBE_CONVERT(_acme_bg - 0x222211)
#define status_bg       MAYBE_CONVERT(_acme_bg_dark - 0x303017)
#define sel_color       acme_blue_light
#define comment         acme_yellow
#define keyword         acme_fg
#define pp_keyword      acme_fg
#define call            acme_blue
#define constant        acme_purple
#define number          acme_cyan
#define string          acme_red
#define character       acme_red

PACKABLE_STYLE(acme) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags            = attr_kind;
    s.active.fg               = foreground;
    s.active.bg               = background;

    s.inactive.flags          = attr_kind;
    s.inactive.fg             = foreground;
    s.inactive.bg             = background;

    s.active_border           = s.active;
    s.inactive_border         = s.inactive;

    s.cursor_line.flags       = attr_kind;
    s.cursor_line.fg          = foreground;
    s.cursor_line.bg          = sel_color;

    s.search.flags            = attr_kind | ATTR_INVERSE;
    s.search.fg               = srch;

    s.search_cursor.flags     = attr_kind | ATTR_INVERSE;
    s.search_cursor.fg        = srch_cursor;

    s.selection.flags         = attr_kind;
    s.selection.fg            = foreground;
    s.selection.bg            = sel_color;

    s.attention.flags         = attr_kind | ATTR_BOLD;
    s.attention.fg            = attn;

    s.associate.flags         = attr_kind | ATTR_BOLD;
    s.associate.bg            = assoc_bg;

    s.command_line            = s.active;

    s.status_line.flags       = attr_kind | ATTR_BOLD;
    s.status_line.fg          = foreground;
    s.status_line.bg          = status_bg;

    s.active_gutter           = s.active;
    s.inactive_gutter         = s.inactive;

    s.code_comment.flags      = attr_kind;
    s.code_comment.fg         = comment;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;
    s.code_keyword.fg         = keyword;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = pp_keyword;

    s.code_fn_call.flags      = attr_kind | ATTR_BOLD;
    s.code_fn_call.fg         = call;

    s.code_number.flags       = attr_kind;
    s.code_number.fg          = number;

    s.code_constant.flags     = attr_kind;
    s.code_constant.fg        = constant;

    s.code_string.flags       = attr_kind;
    s.code_string.fg          = string;

    s.code_character.flags    = attr_kind;
    s.code_character.fg       = character;

    yed_plugin_set_style(self, "acme", &s);

    return 0;
}
