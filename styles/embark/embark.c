#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define deep_space  MAYBE_CONVERT(0x0E0B1C)
#define space       MAYBE_CONVERT(0x1e1c31)
#define eclipse     MAYBE_CONVERT(0x585273)
#define stardust    MAYBE_CONVERT(0xcbe3e7)
#define cosmos      MAYBE_CONVERT(0xa6b3cc)
#define red         MAYBE_CONVERT(0xF48FB1)
#define dark_red    MAYBE_CONVERT(0xF02E6E)
#define green       MAYBE_CONVERT(0xA1EFD3)
#define dark_green  MAYBE_CONVERT(0x62d196)
#define yellow      MAYBE_CONVERT(0xffe6b3)
#define dark_yellow MAYBE_CONVERT(0xF2B482)
#define blue        MAYBE_CONVERT(0x91ddff)
#define dark_blue   MAYBE_CONVERT(0x65b2ff)
#define purple      MAYBE_CONVERT(0xd4bfff)
#define dark_purple MAYBE_CONVERT(0xa37acc)
#define cyan        MAYBE_CONVERT(0x87DFEB)
#define dark_cyan   MAYBE_CONVERT(0x63f2f1)
#define medium_gray MAYBE_CONVERT(0x767676)

#define background      space
#define foreground      stardust
#define srch            dark_cyan
#define srch_cursor     dark_green
#define attn            dark_red
#define assoc_bg        MAYBE_CONVERT(background + 0x111122)
#define status_bg       deep_space
#define sel_color       deep_space
#define comment         medium_gray
#define keyword         red
#define pp_keyword      dark_purple
#define control_flow    yellow
#define typename        purple
#define call            blue
#define constant        dark_yellow
#define number          dark_yellow
#define string          green
#define character       green

PACKABLE_STYLE(embark) {
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
    s.active_border.fg        = status_bg;
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

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags      = attr_kind;
    s.code_comment.fg         = comment;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;
    s.code_keyword.fg         = keyword;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = pp_keyword;

    s.code_control_flow.flags = attr_kind | ATTR_BOLD;
    s.code_control_flow.fg    = control_flow;

    s.code_typename.flags     = attr_kind | ATTR_BOLD;
    s.code_typename.fg        = typename;

    s.code_fn_call.flags      = attr_kind;
    s.code_fn_call.fg         = call;

    s.code_number.flags       = attr_kind;
    s.code_number.fg          = number;

    s.code_constant.flags     = attr_kind;
    s.code_constant.fg        = constant;

    s.code_string.flags       = attr_kind;
    s.code_string.fg          = string;

    s.code_character.flags    = attr_kind;
    s.code_character.fg       = character;

    yed_plugin_set_style(self, "embark", &s);

    return 0;
}
