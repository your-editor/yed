#include <yed/plugin.h>

#define MAYBE_CONVERT(rgb) (tc ? (rgb) : rgb_to_256(rgb))


#define color_white         MAYBE_CONVERT(RGB_32(255, 255, 255))
#define color_bg            MAYBE_CONVERT(RGB_32(40, 40, 40))
#define color_bg_1          MAYBE_CONVERT(RGB_32(30, 30, 30))
#define color_bg_2          MAYBE_CONVERT(RGB_32(30, 30, 30) + RGB_32(20, 25, 35))
#define color_fg            MAYBE_CONVERT(RGB_32_hex(d5c4a1))
#define color_search        MAYBE_CONVERT(RGB_32(73, 212, 92))
#define color_search_cursor MAYBE_CONVERT(RGB_32(212, 73, 92))
#define color_attent        MAYBE_CONVERT(RGB_32(244, 91, 83))
#define color_assoc         MAYBE_CONVERT(RGB_32(30, 30, 30) + RGB_32(20, 25, 35) + RGB_32(40, 20, 80))
#define color_comment       MAYBE_CONVERT(RGB_32_hex(7c6f64))
#define color_keyword       MAYBE_CONVERT(RGB_32(185, 150, 84))
#define color_pp            MAYBE_CONVERT(RGB_32(70, 120, 130))
#define color_fn            MAYBE_CONVERT(RGB_32(150, 90, 80))
#define color_num           MAYBE_CONVERT(RGB_32(110, 150, 100))
#define color_esc           MAYBE_CONVERT(RGB_32(50, 110, 150))
#define color_field         MAYBE_CONVERT(RGB_32_hex(d5c4a1) - RGB_32(80, 50, 20));


PACKABLE_STYLE(casey) {
    yed_style s;
    int       tc,
              attr_kind;

    YED_PLUG_VERSION_CHECK();

    tc        = !!yed_get_var("truecolor");
    attr_kind = tc ? ATTR_KIND_RGB : ATTR_KIND_256;

    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.active.fg           = color_fg;
    s.active.bg           = color_bg_1;

    s.inactive.flags      = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.inactive.fg         = color_fg;
    s.inactive.bg         = color_bg;

    s.active_border       = s.active;
    s.active_border.fg    = color_pp;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = color_bg_2;

    s.selection           = s.cursor_line;

    s.search.flags        = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.search.fg           = color_bg;
    s.search.bg           = color_search;

    s.search_cursor.flags = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.search_cursor.fg    = color_bg;
    s.search_cursor.bg    = color_search_cursor;

    s.attention.flags     = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.attention.fg        = color_attent;

    s.associate.flags     = ATTR_BG_KIND_BITS(attr_kind);
    s.associate.bg        = color_assoc;

    s.command_line        = s.active;

    s.status_line.flags   = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.status_line.fg      = color_bg_1;
    s.status_line.bg      = color_pp;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_comment.fg     = color_comment;

    s.code_keyword.flags  = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_keyword.fg     = color_keyword;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_preprocessor.fg    = color_pp;

    s.code_fn_call.flags  = ATTR_FG_KIND_BITS(attr_kind);
    s.code_fn_call.fg     = color_fn;

    s.code_number.flags  = ATTR_FG_KIND_BITS(attr_kind);
    s.code_number.fg     = color_num;

    s.code_constant      = s.code_preprocessor;
    s.code_string        = s.code_number;
    s.code_character     = s.code_number;
    
    s.code_escape        = s.code_string;
    s.code_escape.fg     = color_esc;

    s.code_field.flags   = attr_kind;
    s.code_field.fg      = color_field;

    yed_plugin_set_style(self, "casey", &s);


#undef color_bg
#undef color_bg_1
#undef color_bg_2
#undef color_fg
#undef color_search
#undef color_search_cursor
#undef color_attent
#undef color_assoc
#undef color_comment
#undef color_keyword
#undef color_pp
#undef color_fn
#undef color_num
#undef color_esc
#undef color_field
#define color_bg            MAYBE_CONVERT(RGB_32_hex(d5c4a1) - RGB_32(10, 10, 10))
#define color_bg_1          MAYBE_CONVERT(RGB_32_hex(d5c4a1))
#define color_bg_2          MAYBE_CONVERT(RGB_32_hex(c5b491) - RGB_32(20, 25, 35))
#define color_fg            MAYBE_CONVERT(RGB_32(30, 30, 30))
#define color_cursor_line   MAYBE_CONVERT(RGB_32_hex(c5b491) - RGB_32(20, 25, 35) + RGB_32(0, 0, 50) - RGB_32(30, 0, 0))
#define color_search        MAYBE_CONVERT(RGB_32(73, 212, 92) - RGB_32_hex(303030))
#define color_search_cursor MAYBE_CONVERT(RGB_32(212, 73, 92) - RGB_32_hex(303030))
#define color_attent        MAYBE_CONVERT(RGB_32(244, 91, 83) - RGB_32_hex(303030))
#define color_assoc         MAYBE_CONVERT(RGB_32(30, 30, 30) + RGB_32(20, 25, 35) + RGB_32(40, 20, 80))
#define color_comment       MAYBE_CONVERT(RGB_32_hex(7c6f64))
#define color_keyword       MAYBE_CONVERT(RGB_32(185, 150, 84) - RGB_32_hex(303030))
#define color_pp            MAYBE_CONVERT(RGB_32(70, 120, 130) - RGB_32_hex(171717))
#define color_fn            MAYBE_CONVERT(RGB_32(150, 90, 80) - RGB_32_hex(303030))
#define color_num           MAYBE_CONVERT(RGB_32(110, 150, 100) - RGB_32_hex(303030))
#define color_esc           MAYBE_CONVERT(RGB_32(50, 110, 150) - RGB_32_hex(303030))
#define color_field         MAYBE_CONVERT(RGB_32(30, 30 ,30) + RGB_32(0, 30, 50));
    memset(&s, 0, sizeof(s));

    s.active.flags        = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.active.fg           = color_fg;
    s.active.bg           = color_bg_1;

    s.inactive.flags      = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.inactive.fg         = color_fg;
    s.inactive.bg         = color_bg;

    s.active_border       = s.active;
    s.active_border.fg    = color_pp;

    s.inactive_border     = s.inactive;

    s.cursor_line.flags   = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.cursor_line.fg      = s.active.fg;
    s.cursor_line.bg      = color_cursor_line;

    s.selection           = s.cursor_line;

    s.search.flags        = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.search.fg           = color_bg;
    s.search.bg           = color_search;

    s.search_cursor.flags = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.search_cursor.fg    = color_bg;
    s.search_cursor.bg    = color_search_cursor;

    s.attention.flags     = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.attention.fg        = color_attent;

    s.associate.flags     = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind);
    s.associate.fg        = color_bg_2;
    s.associate.bg        = color_assoc;

    s.command_line        = s.active;

    s.status_line.flags   = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.status_line.fg      = color_bg_1;
    s.status_line.bg      = color_pp;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags  = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_comment.fg     = color_comment;

    s.code_keyword.flags  = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_keyword.fg     = color_keyword;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags = ATTR_FG_KIND_BITS(attr_kind) | ATTR_BOLD;
    s.code_preprocessor.fg    = color_pp;

    s.code_fn_call.flags  = ATTR_FG_KIND_BITS(attr_kind);
    s.code_fn_call.fg     = color_fn;

    s.code_number.flags  = ATTR_FG_KIND_BITS(attr_kind);
    s.code_number.fg     = color_num;

    s.code_constant      = s.code_preprocessor;
    s.code_string        = s.code_number;
    s.code_character     = s.code_number;
    
    s.code_escape        = s.code_string;
    s.code_escape.fg     = color_esc;

    s.code_field.flags   = attr_kind;
    s.code_field.fg      = color_field;

    yed_plugin_set_style(self, "casey-light", &s);

    return 0;
}
