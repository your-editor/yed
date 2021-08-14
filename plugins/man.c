#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

typedef struct {
    char           *name;
    highlight_info *hinfo;
    u32             start_row;
    u32             end_row;
} man_section_t;

static array_t         sections;
static highlight_info *fallback_section_highlighter;

void man(int n_args, char **args);
void man_word(int n_args, char **args);
void man_line_handler(yed_event *event);
void unload(yed_plugin *self);

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*man-page");

    if (buff == NULL) {
        buff = yed_create_buffer("*man-page");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler h;

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    sections = array_make(man_section_t);

    yed_plugin_set_command(self, "man", man);
    yed_plugin_set_command(self, "man-word", man_word);

    h.kind = EVENT_LINE_PRE_DRAW;
    h.fn   = man_line_handler;
    yed_plugin_add_event_handler(self, h);

    return 0;
}

static void clear_sections(void) {
    man_section_t *it;

    array_traverse(sections, it) {
        free(it->name);
        if (it->hinfo != NULL) {
            highlight_info_free(it->hinfo);
            free(it->hinfo);
        }
    }

    array_clear(sections);

    if (fallback_section_highlighter != NULL) {
        highlight_info_free(fallback_section_highlighter);
        free(fallback_section_highlighter);
        fallback_section_highlighter = NULL;
    }
}

static void add_section(const char *section_name, int row) {
    man_section_t *last;
    man_section_t  new;

    last = array_last(sections);

    if (last != NULL) { last->end_row = row; }

    memset(&new, 0, sizeof(new));
    new.name      = strdup(section_name);
    new.start_row = row;

    array_push(sections, new);
}

static void finish_adding_sections(int row) {
    man_section_t *last;

    last = array_last(sections);

    if (last == NULL) { return; }

    last->end_row = row;
}

void unload(yed_plugin *self) {
    clear_sections();
    array_free(sections);
}

void man_word(int n_args, char **args) {
    char *word;

    word = yed_word_under_cursor();

    if (!word) {
        yed_cerr("cursor is not on a word");
        return;
    }

    YEXE("man", word);

    free(word);
}

static void add_section_highlighter(const char *section_name, highlight_info *hinfo) {
    man_section_t *it;
    int            found;

    found = 0;

    array_traverse(sections, it) {
        if (strcmp(section_name, it->name) == 0) {
            found = 1;
            break;
        }
    }

    if (found) {
        it->hinfo = hinfo;
    } else {
        highlight_info_free(hinfo);
        free(hinfo);
    }
}

static void set_fallback_section_highlighter(highlight_info *hinfo) {
    if (fallback_section_highlighter != NULL) {
        highlight_info_free(fallback_section_highlighter);
        free(fallback_section_highlighter);
    }
    fallback_section_highlighter = hinfo;
}

static void setup_highlighting(int man_section) {
    highlight_info    *hinfo;
    char              *c_kwds[] = {
        "__asm__",  "asm",
        "const",
        "enum",     "extern",
        "inline",
        "restrict",
        "sizeof", "static", "struct",
        "typedef",
        "union",
        "volatile",
    };
    char              *c_pp_kwds[] = {
        "define",
        "elif", "else", "endif", "error",
        "if", "ifdef", "ifndef", "include",
        "message",
        "pragma",
        "undef",
        "warning",
    };
    char              *c_control_flow[] = {
        "break", "case", "continue", "default", "do", "else", "for",
        "goto", "if", "return", "switch",
        "while",
    };
    char              *c_typenames[] = {
        "bool", "char",
        "double",
        "float",
        "long", "int",
        "short", "size_t", "ssize_t", "unsigned", "void",
    };

    switch (man_section) {
        case 1:
            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                highlight_numbers(hinfo);
            set_fallback_section_highlighter(hinfo);

            break;

        case 2:
        case 3:
            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                highlight_add_kwd(hinfo, "NULL", HL_CON);
                highlight_add_kwd(hinfo, "stdin", HL_CON);
                highlight_add_kwd(hinfo, "stdout", HL_CON);
                highlight_add_kwd(hinfo, "stderr", HL_CON);
                highlight_suffixed_words(hinfo, '(', HL_CALL);
                highlight_numbers(hinfo);
            set_fallback_section_highlighter(hinfo);

            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                ARRAY_LOOP(c_kwds)
                    highlight_add_kwd(hinfo, *it, HL_KEY);
                ARRAY_LOOP(c_pp_kwds)
                    highlight_add_prefixed_kwd(hinfo, '#', *it, HL_PP);
                ARRAY_LOOP(c_typenames)
                    highlight_add_kwd(hinfo, *it, HL_TYPE);
                highlight_add_kwd(hinfo, "__VA_ARGS__", HL_PP);
                highlight_add_kwd(hinfo, "__FILE__", HL_PP);
                highlight_add_kwd(hinfo, "__func__", HL_PP);
                highlight_add_kwd(hinfo, "__FUNCTION__", HL_PP);
                highlight_add_kwd(hinfo, "__LINE__", HL_PP);
                highlight_add_kwd(hinfo, "__DATE__", HL_PP);
                highlight_add_kwd(hinfo, "__TIME__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_VERSION__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_HOSTED__", HL_PP);
                highlight_add_kwd(hinfo, "_plusplus", HL_PP);
                highlight_add_kwd(hinfo, "__OBJC__", HL_PP);
                highlight_add_kwd(hinfo, "__ASSEMBLER__", HL_PP);
                highlight_add_kwd(hinfo, "NULL", HL_CON);
                highlight_add_kwd(hinfo, "stdin", HL_CON);
                highlight_add_kwd(hinfo, "stdout", HL_CON);
                highlight_add_kwd(hinfo, "stderr", HL_CON);
                highlight_suffixed_words(hinfo, '(', HL_CALL);
                highlight_numbers(hinfo);
                highlight_within(hinfo, "\"", "\"", '\\', -1, HL_STR);
                highlight_within(hinfo, "'", "'", '\\', 1, HL_CHAR);
                highlight_within(hinfo, "/*", "*/", 0, -1, HL_COMMENT);
                highlight_to_eol_from(hinfo, "//", HL_COMMENT);
            add_section_highlighter("SYNOPSIS", hinfo);

            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                ARRAY_LOOP(c_kwds)
                    highlight_add_kwd(hinfo, *it, HL_KEY);
                ARRAY_LOOP(c_pp_kwds)
                    highlight_add_prefixed_kwd(hinfo, '#', *it, HL_PP);
                ARRAY_LOOP(c_control_flow)
                    highlight_add_kwd(hinfo, *it, HL_CF);
                ARRAY_LOOP(c_typenames)
                    highlight_add_kwd(hinfo, *it, HL_TYPE);
                highlight_add_kwd(hinfo, "__VA_ARGS__", HL_PP);
                highlight_add_kwd(hinfo, "__FILE__", HL_PP);
                highlight_add_kwd(hinfo, "__func__", HL_PP);
                highlight_add_kwd(hinfo, "__FUNCTION__", HL_PP);
                highlight_add_kwd(hinfo, "__LINE__", HL_PP);
                highlight_add_kwd(hinfo, "__DATE__", HL_PP);
                highlight_add_kwd(hinfo, "__TIME__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_VERSION__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_HOSTED__", HL_PP);
                highlight_add_kwd(hinfo, "_plusplus", HL_PP);
                highlight_add_kwd(hinfo, "__OBJC__", HL_PP);
                highlight_add_kwd(hinfo, "__ASSEMBLER__", HL_PP);
                highlight_add_kwd(hinfo, "NULL", HL_CON);
                highlight_add_kwd(hinfo, "stdin", HL_CON);
                highlight_add_kwd(hinfo, "stdout", HL_CON);
                highlight_add_kwd(hinfo, "stderr", HL_CON);
                highlight_suffixed_words(hinfo, '(', HL_CALL);
                highlight_numbers(hinfo);
                highlight_within(hinfo, "\"", "\"", '\\', -1, HL_STR);
                highlight_within(hinfo, "'", "'", '\\', 1, HL_CHAR);
                highlight_within(hinfo, "/*", "*/", 0, -1, HL_COMMENT);
                highlight_to_eol_from(hinfo, "//", HL_COMMENT);
            add_section_highlighter("EXAMPLE", hinfo);

            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                ARRAY_LOOP(c_kwds)
                    highlight_add_kwd(hinfo, *it, HL_KEY);
                ARRAY_LOOP(c_pp_kwds)
                    highlight_add_prefixed_kwd(hinfo, '#', *it, HL_PP);
                ARRAY_LOOP(c_control_flow)
                    highlight_add_kwd(hinfo, *it, HL_CF);
                ARRAY_LOOP(c_typenames)
                    highlight_add_kwd(hinfo, *it, HL_TYPE);
                highlight_add_kwd(hinfo, "__VA_ARGS__", HL_PP);
                highlight_add_kwd(hinfo, "__FILE__", HL_PP);
                highlight_add_kwd(hinfo, "__func__", HL_PP);
                highlight_add_kwd(hinfo, "__FUNCTION__", HL_PP);
                highlight_add_kwd(hinfo, "__LINE__", HL_PP);
                highlight_add_kwd(hinfo, "__DATE__", HL_PP);
                highlight_add_kwd(hinfo, "__TIME__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_VERSION__", HL_PP);
                highlight_add_kwd(hinfo, "__STDC_HOSTED__", HL_PP);
                highlight_add_kwd(hinfo, "_plusplus", HL_PP);
                highlight_add_kwd(hinfo, "__OBJC__", HL_PP);
                highlight_add_kwd(hinfo, "__ASSEMBLER__", HL_PP);
                highlight_add_kwd(hinfo, "NULL", HL_CON);
                highlight_add_kwd(hinfo, "stdin", HL_CON);
                highlight_add_kwd(hinfo, "stdout", HL_CON);
                highlight_add_kwd(hinfo, "stderr", HL_CON);
                highlight_suffixed_words(hinfo, '(', HL_CALL);
                highlight_numbers(hinfo);
                highlight_within(hinfo, "\"", "\"", '\\', -1, HL_STR);
                highlight_within(hinfo, "'", "'", '\\', 1, HL_CHAR);
                highlight_within(hinfo, "/*", "*/", 0, -1, HL_COMMENT);
                highlight_to_eol_from(hinfo, "//", HL_COMMENT);
            add_section_highlighter("EXAMPLES", hinfo);

            break;

        default:
            hinfo = malloc(sizeof(*hinfo));
            highlight_info_make(hinfo);
                highlight_numbers(hinfo);
            set_fallback_section_highlighter(hinfo);

            break;
    }
}

void man(int n_args, char **args) {
    int        width;
    char       pre_cmd_buff[1024];
    char       cmd_buff[1024];
    char       err_buff[1024];
    int        i;
    int        status;
    FILE      *stream;
    int        man_section;
    yed_line  *line;
    yed_glyph *git;
    int        row;

    pre_cmd_buff[0] = 0;
    cmd_buff[0]     = 0;
    err_buff[0]     = 0;

#ifdef __APPLE__
    strcat(pre_cmd_buff, "man");
#else
    strcat(pre_cmd_buff, "man --ascii");
#endif

    strcat(err_buff,     "man");

    for (i = 0; i < n_args; i += 1) {
        strcat(pre_cmd_buff, " ");
        strcat(pre_cmd_buff, args[i]);
        strcat(err_buff, " ");
        strcat(err_buff, args[i]);
    }

#ifdef __APPLE__
    strcat(pre_cmd_buff, " | col -bx; exit ${PIPESTATUS[0]} 2>/dev/null");
#else
    strcat(pre_cmd_buff, " 2>/dev/null");
#endif

    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, " >/dev/null");

    if ((stream = popen(cmd_buff, "r")) == NULL) {
        yed_cerr("failed to invoke '%s'", cmd_buff);
        return;
    }
    status = pclose(stream);
    if (status) {
        yed_cerr("command '%s' failed", err_buff);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*man-page");

    if (ys->active_frame != NULL) {
        width = ys->active_frame->width;
    } else {
        width = 80;
    }

    snprintf(cmd_buff, sizeof(cmd_buff),
             "bash -c 'MANWIDTH=%d ", width);
    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, "'");

    get_or_make_buff()->flags &= ~BUFF_RD_ONLY;

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
        get_or_make_buff()->flags |= BUFF_RD_ONLY;
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        yed_cerr("failed to invoke '%s'", cmd_buff);
        return;
    }
    if (status != 0) {
        get_or_make_buff()->flags |= BUFF_RD_ONLY;
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        yed_cerr("command '%s' failed", err_buff);
        return;
    }

    get_or_make_buff()->flags |= BUFF_RD_ONLY;

    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*man-page");

    clear_sections();
again:;
    row = 1;
    bucket_array_traverse(ys->active_frame->buffer->lines, line) {
        if (row == 1 && line->visual_width == 0) {
            get_or_make_buff()->flags &= ~BUFF_RD_ONLY;
            yed_buff_delete_line_no_undo(get_or_make_buff(), row);
            get_or_make_buff()->flags |= BUFF_RD_ONLY;
            goto again;
        }

        if (row == 1) {
            yed_line_glyph_traverse(*line, git) {
                if (isdigit(git->c)) {
                    man_section = git->c - '0';
                    break;
                }
            }
        } else if (line->visual_width >= 1) {
            git = yed_line_col_to_glyph(line, 1);

            if (git->c != ' ') {
                if (row > 1) {
                    array_zero_term(line->chars);
                    add_section((char*)array_data(line->chars), row);
                }
            }
        }
        row += 1;
    }
    finish_adding_sections(row);

    setup_highlighting(man_section);
}

static highlight_info *get_section_highlighter(int row) {
    man_section_t *it;

    array_traverse(sections, it) {
        if (row > it->start_row && row < it->end_row) {
            if (it->hinfo != NULL) {
                return it->hinfo;
            }
        }
    }

    if (fallback_section_highlighter != NULL) {
        return fallback_section_highlighter;
    }

    return NULL;
}

static int row_is_in_name_section(int row) {
    man_section_t *it;

    array_traverse(sections, it) {
        if (strcmp(it->name, "NAME") == 0) {
            return (row > it->start_row && row < it->end_row);
        }
    }

    return 0;
}

void man_line_handler(yed_event *event) {
    yed_frame      *frame;
    yed_line       *line;
    yed_glyph      *git;
    yed_attrs       attn;
    int             i;
    yed_attrs      *attrs;
    highlight_info *hinfo;

    frame = event->frame;

    if (!frame || frame->buffer != get_or_make_buff()) { return; }

    line = yed_buff_get_line(frame->buffer, event->row);
    if (line != NULL
    &&  line->visual_width >= 1) {

        git = yed_line_col_to_glyph(line, 1);

        if (git->c != ' ') {
            attn = yed_active_style_get_attention();
            for (i = 1; i <= line->visual_width; i += 1) {
                attrs = array_item(event->line_attrs, i - 1);
                yed_combine_attrs(attrs, &attn);
            }
            return;
        }
    }

    if (row_is_in_name_section(event->row)) {
        attn = yed_active_style_get_attention();
        for (i = 1; i <= line->visual_width; i += 1) {
            attrs = array_item(event->line_attrs, i);
            yed_combine_attrs(attrs, &attn);
        }
    }

    if ((hinfo = get_section_highlighter(event->row))) {
        highlight_line(hinfo, event);
    }
}
