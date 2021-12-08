#include <yed/plugin.h>

#define black        (16)
#define white        (15)
#define orange       (130)
#define red          (124)
#define blue         (20)
#define green        (34)
#define light_yellow (187)
#define cyan         (51)
#define bright_red   (9)
#define dark_grey    (235)
#define grey         (244)

#define background   black
#define foreground   white
#define srch         light_yellow
#define srch_cursor  cyan
#define attn         bright_red
#define assoc_bg     grey
#define status_bg    white
#define status_fg    black
#define sel_color    dark_grey
#define comment      blue
#define keyword      orange
#define pp_keyword   orange
#define control_flow orange
#define typename     green
#define call         white
#define constant     red
#define number       red
#define string       red
#define character    red

PACKABLE_STYLE(mrjantz) {
    yed_style s;
    int       attr_kind;

    YED_PLUG_VERSION_CHECK();

    attr_kind = ATTR_256;

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
    s.status_line.fg          = status_fg;
    s.status_line.bg          = status_bg;

    s.active_gutter       = s.active;
    s.inactive_gutter     = s.inactive;

    s.code_comment.flags      = attr_kind;
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

    yed_plugin_set_style(self, "mrjantz", &s);

    return 0;
}
