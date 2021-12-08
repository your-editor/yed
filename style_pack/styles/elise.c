#include <yed/plugin.h>

PACKABLE_STYLE(elise) {
    yed_style s;

    YED_PLUG_VERSION_CHECK();

    memset(&s, 0, sizeof(s));

    s.active.flags = ATTR_RGB;
    s.active.bg    = RGB_32(48, 82, 82);

    /* s.inactive set to default 0 */

    s.cursor_line = s.active;
    s.cursor_line.flags = ATTR_RGB;
    s.cursor_line.bg    = RGB_32(57, 96, 96);

    s.selection.flags = ATTR_INVERSE;

    s.search.flags = ATTR_RGB;
    s.search.fg    = RGB_32(48, 82, 82);
    s.search.bg    = RGB_32(166, 212, 159);

    s.search_cursor.flags = ATTR_RGB;
    s.search_cursor.fg    = RGB_32(48, 82, 82);
    s.search_cursor.bg    = RGB_32(126, 172, 119);

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags = ATTR_RGB | ATTR_BOLD;
    s.code_comment.fg    = RGB_32(232, 208, 58);

    s.code_keyword.flags = ATTR_RGB | ATTR_BOLD;
    s.code_keyword.fg    = RGB_32(175, 89, 122);

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor = s.code_keyword;

    s.code_number.flags = ATTR_RGB;
    s.code_number.fg    = RGB_32(129, 165, 61);

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = RGB_32(141, 144, 233);

    s.code_string.flags = ATTR_RGB;
    s.code_string.fg    = RGB_32(26, 174, 192);

    s.code_character = s.code_string;

    yed_plugin_set_style(self, "elise", &s);
    return 0;
}
