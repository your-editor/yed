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
    array_t              new;
    yed_direct_draw_t  **dit;
    yed_direct_draw_t   *dd;
    yed_attrs            attrs;

    new = array_make(yed_direct_draw_t*);

    array_traverse(ys->direct_draws, dit) {
        dd = *dit;

        if (dd->live) {
            yed_set_cursor(dd->row, dd->col);
            if (dd->scomp != -1) {
                attrs = yed_get_active_style_scomp(dd->scomp);
            } else {
                attrs = dd->attrs;
            }
            attrs.flags |= dd->additional_attr_flags;
            yed_set_attr(attrs);
            yed_screen_print_over(dd->string);
            array_push(new, dd);
        } else {
            free(dd->string);
            free(dd);
        }
    }

    array_free(ys->direct_draws);
    ys->direct_draws = new;
}

void yed_kill_direct_draw(yed_direct_draw_t *dd) { dd->live = 0; }
