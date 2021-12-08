#include <yed/plugin.h>
#include <yed/syntax.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

#ifdef __APPLE__
#define WB "[[:>:]]"
#else
#define WB "\\b"
#endif

typedef struct {
    char       *name;
    yed_syntax *syn;
    u32         start_row;
    u32         end_row;
} man_section_t;

static array_t     sections;
static yed_syntax *fallback_section_highlighter;

void man(int n_args, char **args);
void man_word(int n_args, char **args);
void man_line_handler(yed_event *event);
void unload(yed_plugin *self);

void estyle(yed_event *event) {
    man_section_t *it;

    array_traverse(sections, it) {
        if (it->syn != NULL) {
            yed_syntax_style_event(it->syn, event);
        }
    }

    if (fallback_section_highlighter != NULL) {
        yed_syntax_style_event(fallback_section_highlighter, event);
    }
}

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

    h.kind = EVENT_STYLE_CHANGE;
    h.fn   = estyle;
    yed_plugin_add_event_handler(self, h);

    return 0;
}

static void clear_sections(void) {
    man_section_t *it;

    array_traverse(sections, it) {
        free(it->name);
        if (it->syn != NULL) {
            yed_syntax_free(it->syn);
            free(it->syn);
        }
    }

    array_clear(sections);

    if (fallback_section_highlighter != NULL) {
        yed_syntax_free(fallback_section_highlighter);
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

static void add_section_highlighter(const char *section_name, yed_syntax *syn) {
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
        it->syn = syn;
    } else {
        yed_syntax_free(syn);
        free(syn);
    }
}

static void set_fallback_section_highlighter(yed_syntax *syn) {
    if (fallback_section_highlighter != NULL) {
        yed_syntax_free(fallback_section_highlighter);
        free(fallback_section_highlighter);
    }
    fallback_section_highlighter = syn;
}

static void setup_highlighting(int man_section) {
    yed_syntax    *syn;
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
            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+)"WB, 2);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            set_fallback_section_highlighter(syn);

            break;

        case 2:
        case 3:
            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-constant");
                        yed_syntax_kwd(syn, "NULL");
                        yed_syntax_kwd(syn, "stdin");
                        yed_syntax_kwd(syn, "stdout");
                        yed_syntax_kwd(syn, "stderr");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-fn-call");
                        yed_syntax_regex_sub(syn, "([[:alpha:]_][[:alnum:]_]*)\\(", 1);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            set_fallback_section_highlighter(syn);

            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-comment");
                        yed_syntax_range_start(syn, "/\\*");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "\\*/");
                        yed_syntax_range_start(syn, "//");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "$");
                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*if[[:space:]]+0"WB);
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,"^[[:space:]]*#[[:space:]]*(else|endif|elif|elifdef)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-string");
                        yed_syntax_regex(syn, "'(\\\\.|[^'\\\\])'");

                        yed_syntax_range_start(syn, "\""); yed_syntax_range_one_line(syn); yed_syntax_range_skip(syn, "\\\\\"");
                            yed_syntax_attr_push(syn, "&code-escape");
                                yed_syntax_regex(syn, "\\\\.");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "\"");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-constant");
                        yed_syntax_kwd(syn, "NULL");
                        yed_syntax_kwd(syn, "stdin");
                        yed_syntax_kwd(syn, "stdout");
                        yed_syntax_kwd(syn, "stderr");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-keyword");
                        ARRAY_LOOP(c_kwds)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-control-flow");
                        ARRAY_LOOP(c_control_flow)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-typename");
                        ARRAY_LOOP(c_typenames)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-preprocessor");
                        yed_syntax_kwd(syn, "__VA_ARGS__");
                        yed_syntax_kwd(syn, "__FILE__");
                        yed_syntax_kwd(syn, "__func__");
                        yed_syntax_kwd(syn, "__FUNCTION__");
                        yed_syntax_kwd(syn, "__LINE__");
                        yed_syntax_kwd(syn, "__DATE__");
                        yed_syntax_kwd(syn, "__TIME__");
                        yed_syntax_kwd(syn, "__STDC__");
                        yed_syntax_kwd(syn, "__STDC_VERSION__");
                        yed_syntax_kwd(syn, "__STDC_HOSTED__");
                        yed_syntax_kwd(syn, "__cplusplus");
                        yed_syntax_kwd(syn, "__OBJC__");
                        yed_syntax_kwd(syn, "__ASSEMBLER__");

                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*include"); yed_syntax_range_one_line(syn);
                            yed_syntax_attr_push(syn, "&code-string");
                                yed_syntax_regex(syn, "[<\"].*");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "$");

                        yed_syntax_regex(syn, "^[[:space:]]*#[[:space:]]*(syn, define|elif|else|endif|error|if|ifdef|ifndef|line|message|pragma|undef|warning)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-fn-call");
                        yed_syntax_regex_sub(syn, "([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            add_section_highlighter("SYNOPSIS", syn);

            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-comment");
                        yed_syntax_range_start(syn, "/\\*");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "\\*/");
                        yed_syntax_range_start(syn, "//");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "$");
                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*if[[:space:]]+0"WB);
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,"^[[:space:]]*#[[:space:]]*(else|endif|elif|elifdef)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-string");
                        yed_syntax_regex(syn, "'(\\\\.|[^'\\\\])'");

                        yed_syntax_range_start(syn, "\""); yed_syntax_range_one_line(syn); yed_syntax_range_skip(syn, "\\\\\"");
                            yed_syntax_attr_push(syn, "&code-escape");
                                yed_syntax_regex(syn, "\\\\.");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "\"");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-constant");
                        yed_syntax_kwd(syn, "NULL");
                        yed_syntax_kwd(syn, "stdin");
                        yed_syntax_kwd(syn, "stdout");
                        yed_syntax_kwd(syn, "stderr");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-keyword");
                        ARRAY_LOOP(c_kwds)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-control-flow");
                        ARRAY_LOOP(c_control_flow)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-typename");
                        ARRAY_LOOP(c_typenames)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-preprocessor");
                        yed_syntax_kwd(syn, "__VA_ARGS__");
                        yed_syntax_kwd(syn, "__FILE__");
                        yed_syntax_kwd(syn, "__func__");
                        yed_syntax_kwd(syn, "__FUNCTION__");
                        yed_syntax_kwd(syn, "__LINE__");
                        yed_syntax_kwd(syn, "__DATE__");
                        yed_syntax_kwd(syn, "__TIME__");
                        yed_syntax_kwd(syn, "__STDC__");
                        yed_syntax_kwd(syn, "__STDC_VERSION__");
                        yed_syntax_kwd(syn, "__STDC_HOSTED__");
                        yed_syntax_kwd(syn, "__cplusplus");
                        yed_syntax_kwd(syn, "__OBJC__");
                        yed_syntax_kwd(syn, "__ASSEMBLER__");

                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*include"); yed_syntax_range_one_line(syn);
                            yed_syntax_attr_push(syn, "&code-string");
                                yed_syntax_regex(syn, "[<\"].*");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "$");

                        yed_syntax_regex(syn, "^[[:space:]]*#[[:space:]]*(syn, define|elif|else|endif|error|if|ifdef|ifndef|line|message|pragma|undef|warning)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-fn-call");
                        yed_syntax_regex_sub(syn, "([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            add_section_highlighter("EXAMPLE", syn);

            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-comment");
                        yed_syntax_range_start(syn, "/\\*");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "\\*/");
                        yed_syntax_range_start(syn, "//");
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,  "$");
                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*if[[:space:]]+0"WB);
                            yed_syntax_range_one_line(syn);
                        yed_syntax_range_end(syn,"^[[:space:]]*#[[:space:]]*(else|endif|elif|elifdef)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-string");
                        yed_syntax_regex(syn, "'(\\\\.|[^'\\\\])'");

                        yed_syntax_range_start(syn, "\""); yed_syntax_range_one_line(syn); yed_syntax_range_skip(syn, "\\\\\"");
                            yed_syntax_attr_push(syn, "&code-escape");
                                yed_syntax_regex(syn, "\\\\.");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "\"");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?[fFlL]?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?))?)"WB, 2);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-constant");
                        yed_syntax_kwd(syn, "NULL");
                        yed_syntax_kwd(syn, "stdin");
                        yed_syntax_kwd(syn, "stdout");
                        yed_syntax_kwd(syn, "stderr");
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-keyword");
                        ARRAY_LOOP(c_kwds)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-control-flow");
                        ARRAY_LOOP(c_control_flow)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-typename");
                        ARRAY_LOOP(c_typenames)
                            yed_syntax_kwd(syn, *it);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-preprocessor");
                        yed_syntax_kwd(syn, "__VA_ARGS__");
                        yed_syntax_kwd(syn, "__FILE__");
                        yed_syntax_kwd(syn, "__func__");
                        yed_syntax_kwd(syn, "__FUNCTION__");
                        yed_syntax_kwd(syn, "__LINE__");
                        yed_syntax_kwd(syn, "__DATE__");
                        yed_syntax_kwd(syn, "__TIME__");
                        yed_syntax_kwd(syn, "__STDC__");
                        yed_syntax_kwd(syn, "__STDC_VERSION__");
                        yed_syntax_kwd(syn, "__STDC_HOSTED__");
                        yed_syntax_kwd(syn, "__cplusplus");
                        yed_syntax_kwd(syn, "__OBJC__");
                        yed_syntax_kwd(syn, "__ASSEMBLER__");

                        yed_syntax_range_start(syn, "^[[:space:]]*#[[:space:]]*include"); yed_syntax_range_one_line(syn);
                            yed_syntax_attr_push(syn, "&code-string");
                                yed_syntax_regex(syn, "[<\"].*");
                            yed_syntax_attr_pop(syn);
                        yed_syntax_range_end(syn, "$");

                        yed_syntax_regex(syn, "^[[:space:]]*#[[:space:]]*(syn, define|elif|else|endif|error|if|ifdef|ifndef|line|message|pragma|undef|warning)"WB);
                    yed_syntax_attr_pop(syn);

                    yed_syntax_attr_push(syn, "&code-fn-call");
                        yed_syntax_regex_sub(syn, "([[:alpha:]_][[:alnum:]_]*)[[:space:]]*\\(", 1);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            add_section_highlighter("EXAMPLES", syn);

            break;

        default:
            syn = malloc(sizeof(*syn));
                yed_syntax_start(syn);
                    yed_syntax_attr_push(syn, "&code-number");
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?([[:digit:]]+\\.[[:digit:]]*)|(([[:digit:]]*\\.[[:digit:]]+))(e\\+[[:digit:]]+)?)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(-?[[:digit:]]+)"WB, 2);
                        yed_syntax_regex_sub(syn, "(^|[^[:alnum:]_])(0[xX][0-9a-fA-F]+)"WB, 2);
                    yed_syntax_attr_pop(syn);
                yed_syntax_end(syn);
            set_fallback_section_highlighter(syn);

            break;
    }
}

void man(int n_args, char **args) {
    int        width;
    char       pre_cmd_buff[1024];
    char       cmd_buff[1024];
    char       err_buff[1024];
    int        i;
    char      *output;
    int        status;
    int        man_section;
    yed_line  *line;
    yed_glyph *git;
    int        row;

    pre_cmd_buff[0] = 0;
    cmd_buff[0]     = 0;
    err_buff[0]     = 0;

    strcat(pre_cmd_buff, "man");
    strcat(err_buff,     "man");

    for (i = 0; i < n_args; i += 1) {
        strcat(pre_cmd_buff, " ");
        strcat(pre_cmd_buff, args[i]);
        strcat(err_buff, " ");
        strcat(err_buff, args[i]);
    }

    strcat(pre_cmd_buff, "");

    snprintf(cmd_buff, sizeof(cmd_buff), "bash -c '");
    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, "'");

    output = yed_run_subproc(cmd_buff, NULL, &status);

    if (output != NULL) { free(output); }

    if (status != 0) {
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
    strcat(cmd_buff, " | col -bx; exit ${PIPESTATUS[0]}'");

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

static yed_syntax *get_section_highlighter(int row) {
    man_section_t *it;

    array_traverse(sections, it) {
        if (row > it->start_row && row < it->end_row) {
            if (it->syn != NULL) {
                return it->syn;
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
    yed_syntax *syn;

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

    if ((syn = get_section_highlighter(event->row))) {
        yed_syntax_line_event(syn, event);
    }
}
