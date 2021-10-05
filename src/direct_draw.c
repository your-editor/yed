#include "direct_draw.h"

void yed_init_direct_draw(void) {
    ys->direct_draws = array_make(yed_direct_draw_t*);
}

static yed_direct_draw_t * _yed_make_direct_draw(int row, int col, char *string) {
    yed_direct_draw_t *dd;
    int                n_glyphs;
    int                width;

    dd                        = malloc(sizeof(*dd));
    dd->row                   = MIN(row, ys->term_rows);
    dd->col                   = MIN(col, ys->term_cols);
    dd->len                   = strlen(string);
    dd->additional_attr_flags = 0;
    dd->string                = strdup(string);
    dd->live                  = 1;
    dd->dirty                 = 1;

    yed_get_string_info(string, dd->len, &n_glyphs, &width);

    if (dd->col + width - 1 > ys->term_cols) {
        dd->string[(ys->term_cols - dd->col) + 1] = 0;
    }

    array_push(ys->direct_draws, dd);

    return dd;
}

yed_direct_draw_t * yed_direct_draw(int row, int col, yed_attrs attrs, char *string) {
    yed_direct_draw_t *dd;

    dd        = _yed_make_direct_draw(row, col, string);
    dd->scomp = -1;
    dd->attrs = attrs;

    return dd;
}

yed_direct_draw_t * yed_direct_draw_style(int row, int col, int scomp, char *string) {
    yed_direct_draw_t *dd;

    dd        = _yed_make_direct_draw(row, col, string);
    dd->scomp = scomp;

    return dd;
}

void yed_do_direct_draws(void) {
    int                  save_cur_x;
    int                  save_cur_y;
    array_t              new;
    yed_direct_draw_t  **dit;
    yed_direct_draw_t   *dd;
    yed_attrs            attrs;

    save_cur_x = ys->cur_x;
    save_cur_y = ys->cur_y;

    new = array_make(yed_direct_draw_t*);

    array_traverse(ys->direct_draws, dit) {
        dd = *dit;

        if (dd->live) {
            if (dd->dirty) {
                yed_set_cursor(dd->row, dd->col);
                if (dd->scomp != -1) {
                    attrs = yed_get_active_style_scomp(dd->scomp);
                } else {
                    attrs = dd->attrs;
                }
                attrs.flags |= dd->additional_attr_flags;
                yed_set_attr(attrs);
                append_to_output_buff(dd->string);
            }
            dd->dirty = 0;

            array_push(new, dd);
        } else {
            free(dd->string);
            free(dd);
        }
    }

    array_free(ys->direct_draws);
    ys->direct_draws = new;

    yed_set_cursor(save_cur_y, save_cur_x);
}

void yed_kill_direct_draw(yed_direct_draw_t *dd) {
    dd->live   = 0;
    ys->redraw = 1;
}

void yed_mark_dirty_direct_draws(int top, int bottom, int left, int right) {
    yed_direct_draw_t  **dit;
    yed_direct_draw_t   *dd;

    array_traverse(ys->direct_draws, dit) {
        dd = *dit;

        if (rect_intersect(top,     bottom,  left,    right,
                           dd->row, dd->row, dd->col, dd->col + dd->len - 1)) {
            dd->dirty = 1;
        }
    }
}

void yed_mark_direct_draws_as_dirty(void) {
    yed_direct_draw_t **dit;

    array_traverse(ys->direct_draws, dit) {
        (*dit)->dirty = 1;
    }
}
