#include "status_line.h"

static void write_status_bar(void) {
#if 0
    int         sav_x, sav_y;
    char       *path;
    char       *status_line_var;
    char        right_side_buff[256];
    char       *ft_name;
    int         i;
    yed_frame **fit;

    sav_x = ys->cur_x;
    sav_y = ys->cur_y;

    yed_set_cursor(1, ys->term_rows - 1);
    if (ys->active_style) {
        yed_set_attr(yed_active_style_get_status_line());
    } else {
        append_to_output_buff(TERM_INVERSE);
    }
    append_n_to_output_buff(ys->_4096_spaces, ys->term_cols);

    right_side_buff[0] = 0;

    if (ys->active_frame) {
        yed_set_cursor(1, ys->term_rows - 1);
        append_n_to_output_buff(" ", 1);

        i = 0;
        array_traverse(ys->frames, fit) {
            if (*fit == ys->active_frame) {
                append_n_to_output_buff("[", 1);
                append_int_to_output_buff(i);
                append_n_to_output_buff("]", 2);
            } else {
                append_n_to_output_buff(" ", 1);
                append_int_to_output_buff(i);
                append_n_to_output_buff(" ", 1);
            }
            i += 1;
        }
        append_n_to_output_buff(" ", 1);

        ft_name = "";
        if (ys->active_frame->buffer) {
            path     = ys->active_frame->buffer->name;
            append_to_output_buff(path);

            if (ys->active_frame->buffer->flags & BUFF_SPECIAL) {
                ft_name = "<special>";
            } else {
                ft_name = yed_get_ft_name(ys->active_frame->buffer->ft);
                if (ft_name == NULL) {
                    ft_name = "<unknown file type>";
                }
            }
        }

        snprintf(right_side_buff, MIN(ys->term_cols, sizeof(right_side_buff)),
                 "%s  %7d :: %-3d",
                 ft_name, ys->active_frame->cursor_line, ys->active_frame->cursor_col);
    }

    yed_set_cursor(ys->term_cols - strlen(right_side_buff) - 2, ys->term_rows - 1);
    append_to_output_buff(right_side_buff);


    if ((status_line_var = yed_get_var("status-line-var"))) {
        status_line_var = yed_get_var(status_line_var);
        if (status_line_var) {
            yed_set_small_message(status_line_var);
        } else {
            yed_set_small_message(NULL);
        }
    } else {
        yed_set_small_message(NULL);
    }

    if (ys->small_message) {
        yed_set_cursor((ys->term_cols / 2) - (strlen(ys->small_message) / 2), ys->term_rows - 1);
        if (ys->active_style) {
            yed_set_attr(yed_active_style_get_status_line());
        } else {
            append_to_output_buff(TERM_INVERSE);
        }
        append_to_output_buff(ys->small_message);
    }


    append_to_output_buff(TERM_RESET);
    append_to_output_buff(TERM_CURSOR_HIDE);
    yed_set_cursor(sav_x, sav_y);
#endif
}

static char *get_expanded(char *s) {
    switch (*s) {
        case 'a': return strdup("abc");
        case 'b': return strdup("barbarbar");
        case 'c': return strdup("catcatcatcatcat");
    }

    return NULL;
}

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

    while ((&(git->c) + sizeof(yed_glyph) - 1) < end) {
        len = yed_get_glyph_len(*git);

        if (len == 1) {
            if (last_was_perc) {
                expanded = get_expanded(&git->c);
                if (expanded != NULL) {
                    width += yed_get_string_width(expanded);
                    free(expanded);
                }
                last_was_perc = 0;
            } else if (git->c == '%') {
                last_was_perc = 1;
            } else {
                width += yed_get_glyph_width(*git);
            }
        } else {
            width += yed_get_glyph_width(*git);
        }

        git += len;
    }

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

    col           = start_col;
    git           = (yed_glyph*)s;
    end           = s + strlen(s);
    last_was_perc = 0;

    while ((&(git->c) + sizeof(yed_glyph) - 1) < end) {
        width = 0;
        len   = yed_get_glyph_len(*git);

        if (len == 1) {
            if (last_was_perc) {
                expanded = get_expanded(&git->c);
                if (expanded != NULL) {
                    width = yed_get_string_width(expanded);
                    if (col + width <= ys->term_cols) {
                        append_to_output_buff(expanded);
                        free(expanded);
                    } else {
                        free(expanded);
                        break;
                    }
                }
                last_was_perc = 0;
            } else if (git->c == '%') {
                last_was_perc = 1;
            } else {
                width = yed_get_glyph_width(*git);
                if (col + width > ys->term_cols) { break; }
                append_n_to_output_buff(&git->c, len);
            }
        } else {
            width = yed_get_glyph_width(*git);
            if (col + width > ys->term_cols) { break; }
            append_n_to_output_buff(&git->c, len);
        }

        col += width;
        git += len;
    }
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
    col   = MAX(1, (ys->term_cols / 2) - (width / 2));

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
