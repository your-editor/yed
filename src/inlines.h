#ifndef __INLINES_H__
#define __INLINES_H__

__attribute__((always_inline))
static inline void yed_set_attr(yed_attrs attr) {
    ys->screen_update->cur_attrs = attr;
}

__attribute__((always_inline))
static inline void yed_reset_attr(void) {
    yed_set_attr(ZERO_ATTR);
}

__attribute__((always_inline))
static inline void yed_set_cursor(int row, int col) {
    ys->screen_update->cur_y = row < 1 ? 1 : row;
    if (ys->screen_update->cur_y > ys->term_rows) {
        ys->screen_update->cur_y = ys->term_rows;
    }

    ys->screen_update->cur_x = col < 1 ? 1 : col;
    if (ys->screen_update->cur_x > ys->term_cols) {
        ys->screen_update->cur_x = ys->term_cols;
    }
}

__attribute__((always_inline))
static inline void yed_combine_attrs(yed_attrs *dst, yed_attrs *src) {
    int src_fg;
    int src_bg;

    src_fg = ATTR_FG_KIND(src->flags);
    src_bg = ATTR_BG_KIND(src->flags);

    if (src_fg != ATTR_KIND_NONE) {
        ATTR_SET_FG_KIND(dst->flags, src_fg);
        dst->fg = src->fg;
    }
    if (src_bg != ATTR_KIND_NONE) {
        ATTR_SET_BG_KIND(dst->flags, src_bg);
        dst->bg = src->bg;
    }

    dst->flags |= src->flags & ~0xF;
}

#endif
