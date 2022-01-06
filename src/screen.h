#ifndef __SCREEN_H__
#define __SCREEN_H__

typedef struct {
    yed_attrs attrs;
    yed_glyph glyph;
    int       dirty;
} yed_screen_cell;

typedef struct {
    yed_attrs        cur_attrs;
    int              cur_y;
    int              cur_x;
    yed_screen_cell *cells;
} yed_screen;

void yed_init_screen(void);
void yed_resize_screen(void);
void yed_set_attr(yed_attrs attr);
void yed_reset_attr(void);
void yed_draw_background(void);
void yed_diff_and_swap_screens(void);
void yed_render_screen(void);
void yed_screen_print(const char *s);
void yed_screen_print_n(const char *s, int n);
void yed_screen_print_over(const char *s);
void yed_screen_print_n_over(const char *s, int n);

#endif
