#include "plugin.h"

#define ALT(rgb, _256) (tc ? (rgb) : (_256))

#define base00 (ALT(RGB_32_hex(1d2021), 234))
#define base00_but_lighter \
               (ALT(RGB_32_hex(282828), 235))
#define base01 (ALT(RGB_32_hex(3c3836), 237))
#define base02 (ALT(RGB_32_hex(504945), 239))
#define base03 (ALT(RGB_32_hex(665c54), 241))
#define base04 (ALT(RGB_32_hex(bdae93), 248))
#define base05 (ALT(RGB_32_hex(d5c4a1), 250))
#define base06 (ALT(RGB_32_hex(ebdbb2), 223))
#define base07 (ALT(RGB_32_hex(fbf1c7), 229))
#define base08 (ALT(RGB_32_hex(fb4934), 167))
#define base09 (ALT(RGB_32_hex(fe8019), 208))
#define base0A (ALT(RGB_32_hex(fabd2f), 214))
#define base0B (ALT(RGB_32_hex(b8bb26), 142))
#define base0C (ALT(RGB_32_hex(8ec07c), 108))
#define base0D (ALT(RGB_32_hex(83a598), 109))
#define base0E (ALT(RGB_32_hex(d3869b), 175))
#define base0F (ALT(RGB_32_hex(d65d0e), 166))

int yed_plugin_boot(yed_plugin *self) {
    yed_style s;
    int       tc,
              attr_kind;

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = base05;
    s.active.bg           = base00;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = base05;
    s.inactive.bg         = base00_but_lighter;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = base05;
    s.cursor_line.bg      = base01;

    s.search.flags        = attr_kind | ATTR_BOLD;
    s.search.fg           = base01;
    s.search.bg           = base0A;

    s.search_cursor.flags = attr_kind | ATTR_BOLD;
    s.search_cursor.fg    = base01;
    s.search_cursor.bg    = base09;


    s.selection.flags     = attr_kind;
    s.selection.fg        = base05;
    s.selection.bg        = base02;

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = base08;

    s.code_comment.flags  = attr_kind;
    s.code_comment.fg     = base03;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = base08;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = base09;

    s.code_fn_call.flags  = attr_kind | ATTR_BOLD;
    s.code_fn_call.fg     = base05;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = base0E;

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = base09;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = base0B;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "gruvbox", &s);

    return 0;
}
