#include "status_line.h"

static char *get_expanded(char *s) {
    array_t     chars;
    int         i;
    char        c;
    yed_frame **fit;
    char        ibuff[32];
    char       *istr;
    char       *str;
    struct tm  *tm;
    time_t      t;
    char        tbuff[256];

    switch (*s) {
        case 'b':
            if (ys->active_frame && ys->active_frame->buffer) {
                return strdup(ys->active_frame->buffer->name);
            }
            break;
        case 'B':
            if (ys->active_frame && ys->active_frame->buffer && ys->active_frame->buffer->path) {
                return strdup(ys->active_frame->buffer->path);
            }
            break;
        case 'c':
            if (ys->active_frame) {
                istr = itoa(ibuff, ys->active_frame->cursor_col);
                return strdup(istr);
            } else {
                return strdup("-");
            }
            break;
        case 'f':
            chars = array_make(char);
            i = 0;
            array_traverse(ys->frames, fit) {
                if (*fit == ys->active_frame) {
                    c = '['; array_push(chars, c);
                    istr = itoa(ibuff, i);
                    array_push_n(chars, istr, strlen(istr));
                    c = ']'; array_push(chars, c);
                } else {
                    c = ' ';
                    array_push(chars, c);
                    istr = itoa(ibuff, i);
                    array_push_n(chars, istr, strlen(istr));
                    array_push(chars, c);
                }
                i += 1;
            }
            array_zero_term(chars);
            str = strdup(array_data(chars));
            array_free(chars);
            return str;
            break;
        case 'l':
            if (ys->active_frame) {
                istr = itoa(ibuff, ys->active_frame->cursor_line);
                return strdup(istr);
            } else {
                return strdup("-");
            }
            break;
        case 'p':
            if (ys->active_frame && ys->active_frame->buffer) {
                istr = itoa(ibuff,
                              (100.0 * ys->active_frame->cursor_line)
                            / (yed_buff_n_lines(ys->active_frame->buffer)));
                return strdup(istr);
            } else {
                return strdup("-");
            }
            break;
        case 't':
            if (ys->active_frame && ys->active_frame->buffer) {
                str = yed_get_ft_name(ys->active_frame->buffer->ft);
                if (str == NULL) {
                    return strdup("?");
                }
                return strdup(str);
            }
            break;
        case 'T':
            t  = time(NULL);
            tm = localtime(&t);
            strftime(tbuff, sizeof(tbuff), "%I:%M:%S", tm);
            return strdup(tbuff);
            break;
        case '(':
            chars = array_make(char);
            s += 1;
            while (*s && *s != ')') { array_push(chars, *s); s += 1; }
            if (*s) {
                array_zero_term(chars);
                str = (char*)array_data(chars);
                str = yed_get_var(str);
                if (str != NULL) {
                    str = strdup(str);
                    for (s = str; *s; s += 1) { if (*s == '\n') { *s = 0; break; } }
                    array_free(chars);
                    return str;
                }
            }
            array_free(chars);
            break;
        case '%':
            return strdup("%");
            break;
    }

    return NULL;
}

static void parse_and_put_attrs(char *s) {
    array_t    chars;
    char      *str;
    yed_attrs  base;
    yed_attrs  attrs;

    chars = array_make(char);

    s += 1;
    while (*s && *s != ']') { array_push(chars, *s); s += 1; }

    base = yed_active_style_get_status_line();

    if (*s) {
        array_zero_term(chars);

        str   = (char*)array_data(chars);
        attrs = yed_parse_attrs(str);
        yed_combine_attrs(&base, &attrs);
    }

    yed_set_attr(base);

    array_free(chars);
}

#define GBUMP(_g, _l) ((yed_glyph*)((&((_g)->c)) + (_l)))

static int get_status_line_string_width(char *s) {
    int         width;
    yed_glyph  *git;
    const char *end;
    int         last_was_perc;
    int         len;
    char       *expanded;

    width         = 0;
    git           = (yed_glyph*)s;
    end           = s + strlen(s);
    last_was_perc = 0;

    while ((&(git->c) < end)) {
        len = yed_get_glyph_len(*git);

        if (len == 1) {
            if (last_was_perc) {
                expanded = get_expanded(&git->c);
                if (expanded != NULL) {
                    width += yed_get_string_width(expanded);
                    free(expanded);
                }
                if (git->c == '(') {
                    while (git->c != ')') {
                        git = GBUMP(git, len);
                        if ((&(git->c) >= end)) { goto out; }
                        len = yed_get_glyph_len(*git);
                    }
                    goto skip;
                } else if (git->c == '[') {
                    while (git->c != ']') {
                        git = GBUMP(git, len);
                        if ((&(git->c) >= end)) { goto out; }
                        len = yed_get_glyph_len(*git);
                    }
                    goto skip;
                }
                last_was_perc = 0;
            } else if (git->c == '\n') {
                goto out;
            } else if (git->c == '%') {
                last_was_perc = 1;
            } else {
                width += yed_get_glyph_width(*git);
            }
        } else {
            width += yed_get_glyph_width(*git);
        }

        git = GBUMP(git, len);
skip:;
    }

out:;
    return width;
}

static void put_status_line_string(char *s, int start_col) {
    int         col;
    yed_glyph  *git;
    const char *end;
    int         last_was_perc;
    char       *expanded;
    int         width;
    int         len;

    yed_set_cursor(ys->term_rows - 1, start_col);

    if (ys->active_style) {
        yed_set_attr(yed_active_style_get_status_line());
    } else {
        append_to_output_buff(TERM_INVERSE);
    }

    col           = start_col;
    git           = (yed_glyph*)s;
    end           = s + strlen(s);
    last_was_perc = 0;

    while ((&(git->c) < end)) {
        width = 0;
        len   = yed_get_glyph_len(*git);

        if (len == 1) {
            if (last_was_perc) {
                expanded = get_expanded(&git->c);
                if (expanded != NULL) {
                    width = yed_get_string_width(expanded);
                    if (col + (width - 1) <= ys->term_cols) {
                        append_to_output_buff(expanded);
                        free(expanded);
                    } else {
                        free(expanded);
                        break;
                    }
                }

                if (git->c == '(') {
                    while (git->c != ')') {
                        git = GBUMP(git, len);
                        if ((&(git->c) >= end)) { goto out; }
                        len = yed_get_glyph_len(*git);
                    }
                    goto skip;
                } else if (git->c == '[') {
                    parse_and_put_attrs((char*)&git->c);
                    while (git->c != ']') {
                        git = GBUMP(git, len);
                        if ((&(git->c) >= end)) { goto out; }
                        len = yed_get_glyph_len(*git);
                    }
                    goto skip;
                }

                last_was_perc = 0;
            } else if (git->c == '\n') {
                goto out;
            } else if (git->c == '%') {
                last_was_perc = 1;
            } else {
                width = yed_get_glyph_width(*git);
                if (col + (width - 1) > ys->term_cols) { break; }
                append_n_to_output_buff(&git->c, len);
            }
        } else {
            width = yed_get_glyph_width(*git);
            if (col + (width - 1) > ys->term_cols) { break; }
            append_n_to_output_buff(&git->c, len);
        }

        col += width;
        git = GBUMP(git, len);
skip:;
    }
out:;
}

static void write_status_line_left(void) {
    char *var;

    var = yed_get_var("status-line-left");
    if (var == NULL) { return; }

    put_status_line_string(var, 1);
}

static void write_status_line_center(void) {
    char *var;
    int   width;
    int   col;

    var = yed_get_var("status-line-center");
    if (var == NULL) { return; }

    width = get_status_line_string_width(var);
    col   = MAX(1, 1 + (ys->term_cols / 2) - (width / 2));

    put_status_line_string(var, col);
}

static void write_status_line_right(void) {
    char *var;
    int   width;
    int   col;

    var = yed_get_var("status-line-right");
    if (var == NULL) { return; }

    width = get_status_line_string_width(var);
    col   = MAX(1, ys->term_cols - width + 1);

    put_status_line_string(var, col);
}

void yed_write_status_line(void) {
/*     int        sav_x; */
/*     int        sav_y; */
/*     int        width; */
/*     int        n_bytes; */
/*     char      *buff; */
/*     int        str_width; */
/*     yed_glyph *git; */
/*     int        col; */

/*     sav_x = ys->cur_x; */
/*     sav_y = ys->cur_y; */
/*     width = ys->term_cols; */

    yed_set_cursor(ys->term_rows - 1, 1);

    if (ys->active_style) {
        yed_set_attr(yed_active_style_get_status_line());
    } else {
        append_to_output_buff(TERM_INVERSE);
    }
    append_n_to_output_buff(ys->_4096_spaces, ys->term_cols);

    write_status_line_left();
    write_status_line_center();
    write_status_line_right();
}
