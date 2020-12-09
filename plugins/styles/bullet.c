#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))


#define white      MAYBE_CONVERT(0xFFFFFF)
#define black      MAYBE_CONVERT(0x010101)
#define grey        MAYBE_CONVERT(0x505550)
#define light_grey MAYBE_CONVERT(0xD0D0D0)
#define blue       MAYBE_CONVERT(0x3891A6)
#define dark_blue   MAYBE_CONVERT(0x083D77)
#define yellow     MAYBE_CONVERT(0xFDE74C)
#define red        MAYBE_CONVERT(0xDB5461)
#define orange     MAYBE_CONVERT(0xE76F51)
#define green       MAYBE_CONVERT(0x659157)
#define dark_purple MAYBE_CONVERT(0x45425A)
#define purple      MAYBE_CONVERT(0x9C528B)
#define cyan        MAYBE_CONVERT(0x44FFD2)

#define background      black
#define foreground      light_grey
#define srch            cyan
#define srch_cursor     white
#define attn            MAYBE_CONVERT(0xFF0000)
#define assoc_bg        MAYBE_CONVERT(background + 0x111122)
#define status_bg       green
#define sel_color       grey
#define comment         dark_purple
#define keyword         purple
#define pp_keyword      dark_blue
#define control_flow    red
#define typename        green
#define call            blue
#define constant        orange
#define number          orange
#define string          yellow
#define character       yellow

PACKABLE_STYLE(bullet) {
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

    s.active_gutter           = s.active;
    s.inactive_gutter         = s.inactive;

    s.code_comment.flags      = attr_kind | ATTR_BOLD;
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

    yed_plugin_set_style(self, "bullet", &s);

    return 0;
}
