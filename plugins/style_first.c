#include "plugin.h"

int yed_plugin_boot(yed_plugin *self) {
    yed_style s;

    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_RGB;
    s.active.fg           = RGB_32(246, 241, 209);
    s.active.bg           = RGB_32(10, 20, 30);

    s.inactive.flags      = ATTR_RGB;
    s.inactive.fg         = RGB_32(246, 241, 209);
    s.inactive.bg         = 0;

    s.cursor_line.flags   = ATTR_RGB;
    s.cursor_line.fg      = RGB_32(246, 241, 209);
    s.cursor_line.bg      = RGB_32(11, 32, 39);

    s.selection.flags     = ATTR_RGB;
    s.selection.fg        = RGB_32(10, 20, 30);
    s.selection.bg        = RGB_32(246, 241, 209);

    s.search.flags        = ATTR_RGB | ATTR_BOLD;
    s.search.fg           = RGB_32(0, 0, 255);
    s.search.bg           = RGB_32(255, 255, 0);

    s.search_cursor.flags = ATTR_RGB | ATTR_BOLD;
    s.search_cursor.fg    = RGB_32(0, 0, 255);
    s.search_cursor.bg    = RGB_32(255, 150, 0);

    s.code_comment.flags  = ATTR_RGB;
    s.code_comment.fg     = RGB_32(72, 180, 235);

    s.code_keyword.flags  = ATTR_RGB | ATTR_BOLD;
    s.code_keyword.fg     = RGB_32(216, 30, 91);

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = ATTR_RGB;
    s.code_fn_call.fg     = RGB_32(64, 121, 140);

    s.code_number.flags  = ATTR_RGB;
    s.code_number.fg     = RGB_32(147, 97, 129);

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = RGB_32(252, 163, 17);

    s.code_string.flags   = ATTR_RGB;
    s.code_string.fg      = RGB_32(83, 170, 111);

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "first-dark", &s);


    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_RGB;
    s.active.fg           = RGB_32(11, 32, 39);
    s.active.bg           = RGB_32(246, 241, 209);

    s.inactive.flags      = ATTR_RGB;
    s.inactive.fg         = RGB_32(11, 32, 39);
    s.inactive.bg         = RGB_32(226, 221, 189);

    s.cursor_line.flags   = ATTR_RGB;
    s.cursor_line.fg      = RGB_32(11, 32, 39);
    s.cursor_line.bg      = RGB_32(207, 215, 199);

    s.selection.flags     = ATTR_RGB;
    s.selection.fg        = RGB_32(246, 241, 209);
    s.selection.bg        = RGB_32(11, 32, 39);

    s.search.flags        = ATTR_RGB | ATTR_BOLD;
    s.search.fg           = RGB_32(0, 0, 255);
    s.search.bg           = RGB_32(255, 255, 0);

    s.search_cursor.flags = ATTR_RGB | ATTR_BOLD;
    s.search_cursor.fg    = RGB_32(0, 0, 255);
    s.search_cursor.bg    = RGB_32(255, 150, 0);

    s.code_comment.flags  = ATTR_RGB;
    s.code_comment.fg     = RGB_32(72, 180, 235);

    s.code_keyword.flags  = ATTR_RGB | ATTR_BOLD;
    s.code_keyword.fg     = RGB_32(216, 30, 91);

    s.code_preprocessor   = s.code_keyword;

    s.code_fn_call.flags  = ATTR_RGB;
    s.code_fn_call.fg     = RGB_32(64, 121, 140);

    s.code_number.flags  = ATTR_RGB;
    s.code_number.fg     = RGB_32(147, 97, 129);

    s.code_constant.flags = ATTR_RGB;
    s.code_constant.fg    = RGB_32(252, 163, 17);

    s.code_string.flags   = ATTR_RGB;
    s.code_string.fg      = RGB_32(83, 170, 111);

    s.code_character      = s.code_string;

    yed_plugin_set_style(self, "first-light", &s);

    return 0;
}
