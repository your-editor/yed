#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define white           MAYBE_CONVERT(0xffffff)
#define slate_grey      MAYBE_CONVERT(0x506066)
#define very_light_blue MAYBE_CONVERT(0xCCF3FF)
#define dark_blue       MAYBE_CONVERT(0x2F394D)
#define light_tan       MAYBE_CONVERT(0xEEE1B3)
#define powerful_pink   MAYBE_CONVERT(0xEF476F)
#define sky_blue        MAYBE_CONVERT(0x78C0E0)
#define light_red       MAYBE_CONVERT(0xFF7878)
#define light_green     MAYBE_CONVERT(0x17B890)
#define orange          MAYBE_CONVERT(0xFFB31C)
#define yellow          MAYBE_CONVERT(0xFFF275)

#define background      slate_grey
#define foreground      light_tan
#define srch            light_tan
#define srch_cursor     powerful_pink
#define attn            powerful_pink
#define assoc_bg        MAYBE_CONVERT(background + 0x111122)
#define sel_color       dark_blue
#define comment         very_light_blue
#define keyword         light_red
#define pp_keyword      white
#define call            sky_blue
#define constant        orange
#define number          light_green
#define string          yellow
#define character       yellow

PACKABLE_STYLE(doug) {
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
    s.status_line.fg          = background;
    s.status_line.bg          = foreground;

    s.active_gutter           = s.active;
    s.inactive_gutter         = s.inactive;

    s.code_comment.flags      = attr_kind | ATTR_BOLD;
    s.code_comment.fg         = comment;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;
    s.code_keyword.fg         = keyword;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg    = pp_keyword;

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

    yed_plugin_set_style(self, "doug", &s);

    return 0;
}
