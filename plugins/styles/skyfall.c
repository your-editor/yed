#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))

#define colorfg (MAYBE_CONVERT(RGB_32_hex(a2a4b0)))
#define colorbg (MAYBE_CONVERT(RGB_32_hex(191919)))
#define colorbg_but_lighter\
                (MAYBE_CONVERT(RGB_32_hex(292929)))
#define colorbg_but_lighter_but_lighter\
                (MAYBE_CONVERT(RGB_32_hex(393939)))
#define color00 (MAYBE_CONVERT(RGB_32_hex(32323d)))
#define color01 (MAYBE_CONVERT(RGB_32_hex(f48fb1)))
#define color02 (MAYBE_CONVERT(RGB_32_hex(1d9c80)))
#define color03 (MAYBE_CONVERT(RGB_32_hex(a1efd3)))
#define color04 (MAYBE_CONVERT(RGB_32_hex(92b6f4)))
#define color05 (MAYBE_CONVERT(RGB_32_hex(985eff)))
#define color06 (MAYBE_CONVERT(RGB_32_hex(6dc7b5)))
#define color07 (MAYBE_CONVERT(RGB_32_hex(4d4f5c)))
#define color08 (MAYBE_CONVERT(RGB_32_hex(40424f)))
#define color09 (MAYBE_CONVERT(RGB_32_hex(ec4890)))
#define color0A (MAYBE_CONVERT(RGB_32_hex(43cb8c)))
#define color0B (MAYBE_CONVERT(RGB_32_hex(fffdaf)))
#define color0C (MAYBE_CONVERT(RGB_32_hex(87dfeb)))
#define color0D (MAYBE_CONVERT(RGB_32_hex(bd99ff)))
#define color0E (MAYBE_CONVERT(RGB_32_hex(51e2c2)))
#define color0F (MAYBE_CONVERT(RGB_32_hex(c6c6c6)))

PACKABLE_STYLE(skyfall) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_RGB : ATTR_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = attr_kind;
    s.active.fg           = color0F;
    s.active.bg           = colorbg;

    s.inactive.flags      = attr_kind;
    s.inactive.fg         = color0F;
    s.inactive.bg         = colorbg_but_lighter;

    s.active_border       = s.active;
    s.active_border.fg    = color02;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = attr_kind;
    s.cursor_line.fg      = color0F;
    s.cursor_line.bg      = colorbg_but_lighter_but_lighter;

    s.search.flags        = attr_kind | ATTR_INVERSE;
    s.search.fg           = color0F;
    s.search.bg           = colorbg;

    s.search_cursor.flags = attr_kind | ATTR_INVERSE | ATTR_BOLD;
    s.search_cursor.fg    = color0F;
    s.search_cursor.bg    = colorbg;

    s.selection           = s.cursor_line;
/*     s.selection.flags     = attr_kind | ATTR_INVERSE; */
/*     s.selection.fg        = color0F; */
/*     s.selection.bg        = colorbg; */

    s.attention.flags     = attr_kind | ATTR_BOLD;
    s.attention.fg        = color0F;

    s.associate.flags     = attr_kind | ATTR_BOLD;
    s.associate.fg        = color0F;

    s.command_line        = s.inactive;

    s.status_line.flags   = attr_kind | ATTR_BOLD;
    s.status_line.fg      = s.active.fg;
    s.status_line.bg      = color02;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = attr_kind | ATTR_BOLD;
    s.code_comment.fg     = color03;

    s.code_keyword.flags  = attr_kind | ATTR_BOLD;
    s.code_keyword.fg     = color0B;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags  = attr_kind | ATTR_BOLD;
    s.code_preprocessor.fg     = color0B;

    s.code_fn_call.flags  = attr_kind;
    s.code_fn_call.fg     = color0C;

    s.code_number.flags   = attr_kind;
    s.code_number.fg      = color0D;

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = color0E;

    s.code_string.flags   = attr_kind;
    s.code_string.fg      = color09;

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "skyfall", &s);

    return 0;
}
