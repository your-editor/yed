#include <yed/plugin.h>

#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define color00 (ALT(RGB_32_hex(eeeeee), 255))
#define color00_but_darker \
                (ALT(RGB_32_hex(dadada), 253))
#define color01 (ALT(RGB_32_hex(af0000), 124))
#define color02 (ALT(RGB_32_hex(008700), 28))
#define color03 (ALT(RGB_32_hex(5f8700), 64))
#define color04 (ALT(RGB_32_hex(0087af), 31))
#define color05 (ALT(RGB_32_hex(878787), 102))
#define color06 (ALT(RGB_32_hex(005f87), 24))
#define color07 (ALT(RGB_32_hex(444444), 238))
#define color08 (ALT(RGB_32_hex(bcbcbc), 250))
#define color09 (ALT(RGB_32_hex(d70000), 160))
#define color10 (ALT(RGB_32_hex(d70087), 162))
#define color11 (ALT(RGB_32_hex(8700af), 91))
#define color12 (ALT(RGB_32_hex(d75f00), 166))
#define color13 (ALT(RGB_32_hex(d75f00), 166))
#define color14 (ALT(RGB_32_hex(005faf), 25))
#define color15 (ALT(RGB_32_hex(005f87), 24))
#define color16 (ALT(RGB_32_hex(0087af), 31))
#define color17 (ALT(RGB_32_hex(008700), 28))
#define cursorline \
                (ALT(RGB_32_hex(c6c6c6), 251))
#define searchfg color07
#define searchbg \
                (ALT(RGB_32_hex(ffff5f), 227))
#define selectfg color00
#define selectbg color04
#define statusfg \
                (ALT(RGB_32_hex(e4e4e4), 254))
#define statusbg \
                (ALT(RGB_32_hex(005f87), 24))

PACKABLE_STYLE(papercolor) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = color07;
    s.active.bg           = color00;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = color07;
    s.inactive.bg         = color00_but_darker;

    s.active_border       = s.active;
    s.active_border.fg    = color10;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = cursorline;

    s.search.flags        = attr_kind | ATTR_BOLD;
    s.search.fg           = searchfg;
    s.search.bg           = searchbg;

    s.search_cursor       = s.search;
    s.search_cursor.flags |= ATTR_BOLD;

    s.selection           = s.cursor_line;
/*     s.selection.flags     = attr_kind; */
/*     s.selection.fg        = selectfg; */
/*     s.selection.bg        = selectbg; */

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = color01;

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.bg        = color00_but_darker;

    s.command_line        = s.active;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = statusfg;
    s.status_line.bg      = statusbg;


    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind;
    s.code_comment.fg     = color05;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = color14;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = color11;

    s.code_fn_call.flags  = attr_kind | ATTR_BOLD;
    s.code_fn_call.fg     = color10;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = color13;

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = color09;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = color03;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "papercolor", &s);

    return 0;
}
