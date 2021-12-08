#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define black_1      MAYBE_CONVERT(0x0C0A0F)
#define black_2      MAYBE_CONVERT(0x161419)
#define black_3      MAYBE_CONVERT(0x14191F)
#define black_4      MAYBE_CONVERT(0x2D3640)
#define black_5      MAYBE_CONVERT(0x253340)
#define white        MAYBE_CONVERT(0xEEEEEE)
#define grey_1       MAYBE_CONVERT(0x3E4B59)
#define grey_2       MAYBE_CONVERT(0x5C6773)
#define light_red    MAYBE_CONVERT(0xF07178)
#define light_yellow MAYBE_CONVERT(0xFFEE99)
#define yellow       MAYBE_CONVERT(0xE7C547)
#define light_blue   MAYBE_CONVERT(0x36A3D9)
#define light_green  MAYBE_CONVERT(0x95E6CB)
#define green        MAYBE_CONVERT(0xB8CC52)
#define orange       MAYBE_CONVERT(0xFFB454)
#define light_orange MAYBE_CONVERT(0xE6B673)
#define dark_orange  MAYBE_CONVERT(0xFF7733)
#define red          MAYBE_CONVERT(0xFF3333)


#define background      black_1
#define foreground      white
#define srch            yellow
#define srch_cursor     light_red
#define attn            red
#define assoc_bg        MAYBE_CONVERT(background + 0x111122)
#define status_bg       light_blue
#define sel_color       black_5
#define comment         grey_1
#define keyword         light_blue
#define pp_keyword      dark_orange
#define control_flow    light_green
#define call            orange
#define constant        light_yellow
#define number          light_yellow
#define string          green
#define character       green

PACKABLE_STYLE(dalton) {
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

    s.code_comment.flags      = attr_kind;
    s.code_comment.fg         = comment;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;
    s.code_keyword.fg         = keyword;

    s.code_control_flow.flags = attr_kind | ATTR_BOLD;
    s.code_control_flow.fg    = control_flow;

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

    yed_plugin_set_style(self, "dalton", &s);

    return 0;
}
