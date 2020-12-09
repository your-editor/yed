#include <yed/plugin.h>

PACKABLE_STYLE(bold) {
    yed_style s;
    int       attr_kind;

    YED_PLUG_VERSION_CHECK();

    attr_kind = ATTR_256;

    memset(&s, 0, sizeof(s));

#define background  (233)
#define forground   (15)
#define sel         (240)
#define accent      (107)
#define assoc       (25)
#define attn        (9)
#define comm        (102)

    s.active.flags            = attr_kind;
    s.active.fg               = forground;
    s.active.bg               = background;

    s.inactive                = s.active;

    s.active_border           = s.active;
    s.active_border.fg        = accent;

    s.inactive_border         = s.inactive;

    s.cursor_line.flags       = attr_kind;
    s.cursor_line.fg          = s.active.fg;
    s.cursor_line.bg          = sel;

    s.search.flags            = attr_kind;
    s.search.fg               = s.active.fg;
    s.search.bg               = accent;

    s.search_cursor.flags     = attr_kind | ATTR_BOLD;
    s.search_cursor.fg        = s.active.bg;
    s.search_cursor.bg        = accent;

    s.selection.flags         = attr_kind;
    s.selection.fg            = s.active.fg;
    s.selection.bg            = sel;

    s.attention.flags         = attr_kind | ATTR_BOLD;
    s.attention.fg            = attn;

    s.associate.flags         = attr_kind;
    s.associate.bg            = assoc;

    s.command_line            = s.active;

    s.status_line.flags       = attr_kind | ATTR_BOLD;
    s.status_line.fg          = s.active.bg;
    s.status_line.bg          = accent;

    s.active_gutter           = s.active;
    s.inactive_gutter         = s.inactive;

    s.code_comment.flags      = attr_kind;
    s.code_comment.fg         = comm;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;

    s.code_control_flow       =
    s.code_typename           = s.code_keyword;

    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;

    s.code_number.flags       = attr_kind;
    s.code_number.fg          = accent;

    s.code_constant.flags     = attr_kind;
    s.code_constant.fg        = accent;

    s.code_string.flags       = attr_kind;
    s.code_string.fg          = accent;

    s.code_character.flags    = attr_kind;
    s.code_character.fg       = accent;

    yed_plugin_set_style(self, "bold-dark", &s);



    memset(&s, 0, sizeof(s));

#undef background
#undef forground
#undef sel
#undef accent
#undef assoc
#undef attn
#undef comm

#define background (15)
#define forground  (233)
#define sel        (248)
#define accent     (23)
#define assoc      (111)
#define attn       (167)
#define comm       (102)


    s.active.flags            = attr_kind;
    s.active.fg               = forground;
    s.active.bg               = background;

    s.inactive                = s.active;

    s.active_border           = s.active;
    s.active_border.fg        = accent;

    s.inactive_border         = s.inactive;

    s.cursor_line.flags       = attr_kind;
    s.cursor_line.fg          = s.active.fg;
    s.cursor_line.bg          = sel;

    s.search.flags            = attr_kind;
    s.search.fg               = s.active.fg;
    s.search.bg               = accent;

    s.search_cursor.flags     = attr_kind | ATTR_BOLD;
    s.search_cursor.fg        = s.active.bg;
    s.search_cursor.bg        = accent;

    s.selection.flags         = attr_kind;
    s.selection.fg            = s.active.fg;
    s.selection.bg            = sel;

    s.attention.flags         = attr_kind | ATTR_BOLD;
    s.attention.fg            = attn;

    s.associate.flags         = attr_kind;
    s.associate.bg            = assoc;

    s.command_line            = s.active;

    s.status_line.flags       = attr_kind | ATTR_BOLD;
    s.status_line.fg          = s.active.bg;
    s.status_line.bg          = accent;

    s.active_gutter           = s.active;
    s.inactive_gutter         = s.inactive;

    s.code_comment.flags      = attr_kind;
    s.code_comment.fg         = comm;

    s.code_keyword.flags      = attr_kind | ATTR_BOLD;
    s.code_preprocessor.flags = attr_kind | ATTR_BOLD;

    s.code_number.flags       = attr_kind;
    s.code_number.fg          = accent;

    s.code_constant.flags     = attr_kind;
    s.code_constant.fg        = accent;

    s.code_string.flags       = attr_kind;
    s.code_string.fg          = accent;

    s.code_character.flags    = attr_kind;
    s.code_character.fg       = accent;

    yed_plugin_set_style(self, "bold-light", &s);

    return 0;
}
