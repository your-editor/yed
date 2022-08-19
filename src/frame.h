#ifndef __FRAME_H__
#define __FRAME_H__

#include "frame_tree.h"

struct yed_event_t;

typedef struct yed_frame_t {
    yed_frame_tree     *tree;
    yed_buffer         *buffer;
    float               top_f,
                        left_f,
                        height_f,
                        width_f;
    int                 top,
                        left,
                        height,
                        width;
    int                 btop, bleft, bheight, bwidth;
    int                 gutter_width;
    int                 cursor_line,
                        cursor_col,
                        buffer_y_offset,
                        buffer_x_offset;
    int                 cur_x,
                        cur_y;
    int                 desired_col;
    int                 scroll_off;
    yed_attrs           row_base_attrs;
    array_t             line_attrs;
    array_t             gutter_glyphs;
    array_t             gutter_attrs;
    char               *name;
} yed_frame;

void yed_init_frames(void);
yed_frame * yed_add_new_frame(float top_f, float left_f, float height_f, float width_f);
yed_frame * yed_add_new_frame_full(void);
yed_frame * yed_new_frame(float top_f, float left_f, float height_f, float width_f);
void yed_delete_frame(yed_frame *frame);
yed_frame * yed_vsplit_frame_tree(yed_frame_tree *tree);
yed_frame * yed_hsplit_frame_tree(yed_frame_tree *tree);
yed_frame * yed_vsplit_frame(yed_frame *frame);
yed_frame * yed_hsplit_frame(yed_frame *frame);
void yed_activate_frame(yed_frame *frame);
void yed_clear_frame(yed_frame *frame);
void yed_frame_set_size(yed_frame *frame, float height, float width);
void yed_frame_tree_set_size(yed_frame_tree *tree, float height, float width);
void yed_frame_set_pos(yed_frame *frame, float top, float left);
void yed_frame_tree_set_pos(yed_frame_tree *tree, float top, float left);
void yed_resize_frame(yed_frame *frame, int rows, int cols);
void yed_resize_frame_tree(yed_frame_tree *tree, int rows, int cols);
void yed_move_frame(yed_frame *frame, int rows, int cols);
void yed_move_frame_tree(yed_frame_tree *tree, int rows, int cols);
void yed_frame_draw_buff(yed_frame *frame, yed_buffer *buff, int y_offset, int x_offset);
void yed_frame_set_buff(yed_frame *frame, yed_buffer *buff);
void yed_frame_set_gutter_width(yed_frame *frame, int width);
void yed_frame_update(yed_frame *frame);
void yed_move_cursor_within_frame(yed_frame *f, int row, int n_glyphs);
void yed_move_cursor_within_active_frame(int row, int n_glyphs);
void yed_set_cursor_within_frame(yed_frame *frame, int new_row, int new_col);
void yed_set_cursor_far_within_frame(yed_frame *frame, int new_row, int new_col);
void yed_frame_reset_cursor(yed_frame *frame);
void yed_frame_hard_reset_cursor_x(yed_frame *frame);
void yed_frame_hard_reset_cursor_y(yed_frame *frame);
void yed_frame_scroll_buffer(yed_frame *frame, int rows);
void yed_update_frames(void);
void yed_frames_remove_buffer(yed_buffer *buff);
yed_frame * yed_find_frame_by_name(const char *name);
int  yed_frame_set_name(yed_frame *f, const char *name);

void frame_get_rect(yed_frame *frame, int *top,  int *left,  int *height,  int *width,
                                      int *btop, int *bleft, int *bheight, int *bwidth);

#define FRAME_RESET_RECT(f) frame_get_rect((f), &(f)->top,  &(f)->left,  &(f)->height,  &(f)->width,\
                                                &(f)->btop, &(f)->bleft, &(f)->bheight, &(f)->bwidth)

int yed_cell_is_in_frame(int row, int col, yed_frame *frame);

int yed_frame_line_to_y(yed_frame *frame, int row);

int yed_frame_is_tree_root(yed_frame *frame);

yed_attrs * yed_eline_get_col_attrs(struct yed_event_t *event, int col);
int yed_eline_set_col_attrs(struct yed_event_t *event, int col, yed_attrs *attrs);
int yed_eline_combine_col_attrs(struct yed_event_t *event, int col, yed_attrs *attrs);

#endif
